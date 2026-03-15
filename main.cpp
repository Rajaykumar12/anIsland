// ============================================================
// OpenGL Terrain + Instanced Grass
// Fixes applied:
//   1) GLAD initialization immediately after glfwMakeContextCurrent
//   2) Perlin noise heightmap generated in C++ and passed to GL
//   3) FPS camera with WASD + mouse look (no static view matrix)
// ============================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cstdlib>
#include <iostream>

#include "Terrain.h"
#include "Shader.h"
#include "Camera.h"
#include "NoiseMap.h"

// ---- Window dimensions ----
const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;

// ---- Global camera (used in callbacks) ----
Camera camera(glm::vec3(50.0f, 25.0f, 80.0f));   // Start above terrain center
float lastX      = SCR_WIDTH  / 2.0f;
float lastY      = SCR_HEIGHT / 2.0f;
bool  firstMouse = true;

// ---- Timing ----
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// -------------------------------------------------------
// GLFW Callbacks
// -------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* /*win*/, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* /*win*/, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset =  (xpos - lastX);
    float yoffset =  (lastY - ypos); // reversed: y goes bottom-to-top in OpenGL
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* /*win*/, double /*xoffset*/, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD,  deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT,     deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT,    deltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) camera.ProcessKeyboard(UP,       deltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) camera.ProcessKeyboard(DOWN,     deltaTime);

    // Toggle wireframe with F
    static bool wireframe = false;
    static bool fWasDown  = false;
    bool fIsDown = glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS;
    if (fIsDown && !fWasDown) {
        wireframe = !wireframe;
        glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL);
    }
    fWasDown = fIsDown;
}

// -------------------------------------------------------
// Upload a float heightmap (one channel) as a GL texture
// Returns the OpenGL texture ID.
// -------------------------------------------------------
unsigned int createHeightmapTexture(const std::vector<float>& data, int width, int height) {
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Single-channel float texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0,
                 GL_RED, GL_FLOAT, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

// -------------------------------------------------------
int main() {
    // ---- 1. GLFW Init ----
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
                                          "Terrain + Instanced Grass", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ---- 2. GLAD Init (must be IMMEDIATELY after MakeContextCurrent) ----
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window,       mouse_callback);
    glfwSetScrollCallback(window,          scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR,  GLFW_CURSOR_DISABLED); // capture mouse

    // ---- OpenGL state ----
    glEnable(GL_DEPTH_TEST);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // ---- 3. Noise Heightmap ----
    const int HM_W = 256, HM_H = 256;
    std::vector<float> noiseData = NoiseMap::generate(HM_W, HM_H,
        /*octaves*/     6,
        /*persistence*/ 0.5f,
        /*lacunarity*/  2.0f,
        /*scale*/       60.0f,
        /*seed*/        1337u);

    unsigned int heightmapTex = createHeightmapTexture(noiseData, HM_W, HM_H);

    // ---- 4. Shaders ----
    Shader terrainShader("assets/shaders/terrain.vert", "assets/shaders/terrain.frag");
    Shader grassShader  ("assets/shaders/grass.vert",   "assets/shaders/grass.frag");

    // Tell terrain shader which texture unit holds the heightmap
    terrainShader.use();
    terrainShader.setInt("heightMap", 0); // GL_TEXTURE0

    // ---- 5. Terrain geometry ----
    const int TERRAIN_W = 100, TERRAIN_D = 100;
    Terrain terrain(TERRAIN_W, TERRAIN_D);

    // ---- 6. Instanced Grass Setup ----
    const int instanceCount = 10000;
    std::vector<glm::vec3> translations;
    translations.reserve(instanceCount);
    srand(42);
    for (int i = 0; i < instanceCount; ++i) {
        float x = (float)(rand() % TERRAIN_W);
        float z = (float)(rand() % TERRAIN_D);
        translations.push_back(glm::vec3(x, 0.0f, z));
    }

    // Single grass blade: a simple vertical triangle/quad (6 vertices)
    float grassVertices[] = {
        // pos (x,y,z)      uv
        -0.05f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.05f, 0.0f, 0.0f,  1.0f, 0.0f,
         0.00f, 0.8f, 0.0f,  0.5f, 1.0f,

        -0.05f, 0.0f, 0.0f,  0.0f, 0.0f,
         0.00f, 0.8f, 0.0f,  0.5f, 1.0f,
         0.05f, 0.8f, 0.0f,  1.0f, 1.0f,
    };

    unsigned int grassVAO, grassVBO, instanceVBO;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(grassVAO);

    // Blade geometry
    glBindBuffer(GL_ARRAY_BUFFER, grassVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grassVertices), grassVertices, GL_STATIC_DRAW);
    // aPos (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // aTexCoords (location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Instance offsets (location 2)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(glm::vec3),
                 translations.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1); // one offset per grass blade instance

    glBindVertexArray(0);

    // ---- 7. Render Loop ----
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // sky blue
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrices
        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
        float aspect = (currentHeight == 0) ? 1.0f : (float)currentWidth / (float)currentHeight;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                 aspect,
                                                 0.1f, 1000.0f);
        glm::mat4 view  = camera.GetViewMatrix(); // <-- FPS camera, not static!
        glm::mat4 model = glm::mat4(1.0f);

        // --- Draw Terrain ---
        terrainShader.use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view",       view);
        terrainShader.setMat4("model",      model);

        // Bind the Perlin noise heightmap to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, heightmapTex);

        terrain.Draw();

        // --- Draw Instanced Grass ---
        grassShader.use();
        grassShader.setMat4("projection", projection);
        grassShader.setMat4("view",       view);
        grassShader.setFloat("u_Time",   currentFrame);

        glBindVertexArray(grassVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Cleanup ----
    glDeleteVertexArrays(1, &grassVAO);
    glDeleteBuffers(1, &grassVBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteTextures(1, &heightmapTex);

    glfwTerminate();
    return 0;
}
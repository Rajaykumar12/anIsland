// ============================================================
// OpenGL Terrain Visualization - Modular Architecture
// Systems: Terrain, Trees, Buildings, NPCs, Particles, Water, Lighting, Grass
// ============================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <cstdlib>
#include <cmath>
#include <iostream>

#include "Terrain.h"
#include "Shader.h"
#include "Camera.h"
#include "NoiseMap.h"
#include "TreeSystem.h"
#include "BuildingSystem.h"
#include "NPCSystem.h"
#include "ParticleSystem.h"
#include "WaterSystem.h"
#include "GrassSystem.h"
#include "LightingSystem.h"

// ---- Window dimensions ----
const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;

// ---- Global camera (used in callbacks) ----
Camera camera(glm::vec3(125.0f, 200.0f, 175.0f));
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

    float xoffset = (xpos - lastX);
    float yoffset = (lastY - ypos);
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

unsigned int createHeightmapTexture(const std::vector<float>& data, int width, int height) {
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0,
                 GL_RED, GL_FLOAT, data.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

// Create a 1x1x1 cube for the skybox (36 vertices)
unsigned int createSkyboxCube() {
    float vertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    return skyboxVAO;
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
                                          "Terrain Visualization", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // ---- 2. GLAD Init ----
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window,       mouse_callback);
    glfwSetScrollCallback(window,          scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR,  GLFW_CURSOR_DISABLED);

    // ---- OpenGL state ----
    glEnable(GL_DEPTH_TEST);
    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // ---- 3. Generate Noise Heightmap ----
    const int HM_W = 512, HM_H = 512;
    std::vector<float> noiseData = NoiseMap::generate(HM_W, HM_H, 6, 0.5f, 2.0f, 150.0f, 1337u);
    unsigned int heightmapTex = createHeightmapTexture(noiseData, HM_W, HM_H);

    // ---- 4. Initialize Systems ----
    Terrain terrain(800, 800);
    TreeSystem treeSystem(noiseData, HM_W, HM_H, 800, 800);
    GrassSystem grassSystem(noiseData, HM_W, HM_H, 800, 800);
    BuildingSystem buildingSystem;
    NPCSystem npcSystem;
    ParticleSystem particleSystem(500);
    WaterSystem waterSystem(2000, 2000);  // Larger mesh for 5x ocean area
    LightingSystem lightingSystem;

    // ---- 5. Load Shaders ----
    Shader terrainShader("assets/shaders/terrain.vert",   "assets/shaders/terrain.frag");
    Shader treeShader("assets/shaders/tree.vert",         "assets/shaders/tree.frag");
    Shader grassShader("assets/shaders/grass.vert",       "assets/shaders/grass.frag");
    Shader buildingShader("assets/shaders/building.vert", "assets/shaders/building.frag");
    Shader personShader("assets/shaders/person.vert",     "assets/shaders/person.frag");
    Shader particleShader("assets/shaders/particle.vert", "assets/shaders/particle.frag");
    Shader waterShader("assets/shaders/water.vert",       "assets/shaders/water.frag");
    Shader skyboxShader("assets/shaders/skybox.vert",     "assets/shaders/skybox.frag");

    // Tell terrain shader which texture unit holds the heightmap
    terrainShader.use();
    terrainShader.setInt("heightMap", 0);
    
    // Create skybox VAO
    unsigned int skyboxVAO = createSkyboxCube();

    // ---- 6. Render Loop ----
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Update lighting/day-night cycle
        lightingSystem.Update(currentFrame);

        // Get window dimensions
        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);

        // ============================================================
        // PASS 1: DEPTH PASS - Render from light's perspective
        // ============================================================
        Shader depthShader("assets/shaders/depth.vert", "assets/shaders/depth.frag");
        
        glViewport(0, 0, 2048, 2048);
        glBindFramebuffer(GL_FRAMEBUFFER, lightingSystem.GetDepthMapFBO());
        glClear(GL_DEPTH_BUFFER_BIT);
        
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightingSystem.GetLightSpaceMatrix());
        depthShader.setMat4("model", glm::mat4(1.0f));
        
        // Draw terrain to depth map
        terrain.Draw();
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // ============================================================
        // PASS 2: COLOR PASS - Render from camera's perspective
        // ============================================================
        glViewport(0, 0, currentWidth, currentHeight);
        
        glClearColor(lightingSystem.GetSkyColor().r, lightingSystem.GetSkyColor().g, 
                     lightingSystem.GetSkyColor().b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate matrices
        float aspect = (currentHeight == 0) ? 1.0f : (float)currentWidth / (float)currentHeight;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 2000.0f);
        glm::mat4 view  = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // Calculate fog color based on day/night cycle
        // Sun intensity ranges from 0.0 (night) to ~1.5 (noon)
        float dayIntensity = glm::max(0.0f, glm::min(1.0f, lightingSystem.GetSunIntensity() - 0.2f) / 1.3f);
        glm::vec3 dayFogColor = glm::vec3(0.5f, 0.6f, 0.7f);
        glm::vec3 nightFogColor = glm::vec3(0.01f, 0.01f, 0.02f);
        glm::vec3 currentFogColor = glm::mix(nightFogColor, dayFogColor, dayIntensity);

        // Pass camera position and fog color to all shaders
        terrainShader.use();
        terrainShader.setVec3("cameraPos", camera.Position);
        terrainShader.setVec3("fogColor", currentFogColor);

        treeShader.use();
        treeShader.setVec3("cameraPos", camera.Position);
        treeShader.setVec3("fogColor", currentFogColor);

        grassShader.use();
        grassShader.setVec3("cameraPos", camera.Position);
        grassShader.setVec3("fogColor", currentFogColor);

        buildingShader.use();
        buildingShader.setVec3("cameraPos", camera.Position);
        buildingShader.setVec3("fogColor", currentFogColor);

        personShader.use();
        personShader.setVec3("cameraPos", camera.Position);
        personShader.setVec3("fogColor", currentFogColor);

        waterShader.use();
        waterShader.setVec3("cameraPos", camera.Position);
        waterShader.setVec3("fogColor", currentFogColor);

        // --- Draw Terrain (now with shadows) ---
        terrainShader.use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view",       view);
        terrainShader.setMat4("model",      model);
        terrainShader.setMat4("lightSpaceMatrix", lightingSystem.GetLightSpaceMatrix());
        terrainShader.setVec3("skyColor",   lightingSystem.GetSkyColor());
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, heightmapTex);
        
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, lightingSystem.GetDepthMapTexture());
        terrainShader.setInt("shadowMap", 1);
        
        terrain.Draw();

        // --- Draw Grass ---
        grassShader.use();
        grassShader.setMat4("projection", projection);
        grassShader.setMat4("view", view);
        grassShader.setVec3("skyColor", lightingSystem.GetSkyColor());
        grassSystem.Draw();

        // --- Draw Trees ---
        treeSystem.Render(projection, view, lightingSystem.GetSkyColor(), treeShader);

        // --- Draw Buildings ---
        buildingSystem.Render(projection, view, lightingSystem.GetSkyColor(), buildingShader);

        // --- Draw NPCs ---
        npcSystem.Render(projection, view, lightingSystem.GetSkyColor(), personShader, currentFrame);

        // --- Draw Water ---
        waterSystem.Render(projection, view, waterShader, currentFrame, 
                          lightingSystem.GetSunColor() * lightingSystem.GetSunIntensity());

        // --- Update and Draw Particles ---
        particleSystem.Update(deltaTime);
        particleSystem.Render(projection, view, particleShader);

        // === Draw Skybox (last, with special depth handling) ===
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        // Remove translation from view matrix so skybox follows camera
        glm::mat4 skyView = glm::mat4(glm::mat3(view));
        skyboxShader.setMat4("view", skyView);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setVec3("sunPos", lightingSystem.GetSunPosition());

        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Cleanup ----
    glDeleteTextures(1, &heightmapTex);
    glfwTerminate();
    return 0;
}

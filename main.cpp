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
#include <cmath>
#include <iostream>

#include "Terrain.h"
#include "Shader.h"
#include "Camera.h"
#include "NoiseMap.h"

// ---- Particle System ----
struct Particle {
    glm::vec3 Position;
    glm::vec3 Velocity;
    float Life;
};

// ---- Window dimensions ----
const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;

// ---- Global camera (used in callbacks) ----
Camera camera(glm::vec3(125.0f, 200.0f, 175.0f));   // Start above town center
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
    const int HM_W = 512, HM_H = 512;
    // Scale 150.0 makes the mountains vast and rolling
    std::vector<float> noiseData = NoiseMap::generate(HM_W, HM_H, 6, 0.5f, 2.0f, 150.0f, 1337u);
    unsigned int heightmapTex = createHeightmapTexture(noiseData, HM_W, HM_H);

    // ---- 4. Shaders ----
    Shader terrainShader("assets/shaders/terrain.vert", "assets/shaders/terrain.frag");
    Shader treeShader   ("assets/shaders/tree.vert",     "assets/shaders/tree.frag");

    // Tell terrain shader which texture unit holds the heightmap
    terrainShader.use();
    terrainShader.setInt("heightMap", 0); // GL_TEXTURE0

    // ---- 5. Terrain geometry ----
    const int TERRAIN_W = 800, TERRAIN_D = 800;  // Increased for more detail
    Terrain terrain(TERRAIN_W, TERRAIN_D);

    // ---- 6. Instanced Trees Setup ----
    const int instanceCount = 15000;
    std::vector<glm::vec3> translations;
    translations.reserve(instanceCount);
    srand(42);

    // Filter trees based on height and slope
    // The vertex shader displaces Y by (noise * 150.0f). We need to replicate that
    // here so the tree perfectly roots into the terrain.
    const float TERRAIN_MAX_HEIGHT = 150.0f;
    for (int i = 0; i < 50000; ++i) { // Try multiple times until we reach instanceCount
        if (translations.size() >= instanceCount) break;

        float x = (float)(rand() % (TERRAIN_W - 2)) + 1.0f;
        float z = (float)(rand() % (TERRAIN_D - 2)) + 1.0f;

        // Map (x,z) to (u,v) in the heightmap
        int px = (int)((x / TERRAIN_W) * HM_W);
        int pz = (int)((z / TERRAIN_D) * HM_H);
        px = glm::clamp(px, 0, HM_W - 1);
        pz = glm::clamp(pz, 0, HM_H - 1);

        float h = noiseData[pz * HM_W + px];
        float worldY = h * TERRAIN_MAX_HEIGHT;

        // Don't spawn trees underwater (if we had water) or on snow peaks
        if (worldY < 5.0f || worldY > 80.0f) continue;

        translations.push_back(glm::vec3(x, worldY, z));
    }

    // A simple 3D Tree: Multiple quads for trunk and canopy
    // X, Y, Z, U, V, NormalX, NormalY, NormalZ, R, G, B
    float treeVertices[] = {
        // --- Trunk (Brown) - 4 quads for more detail ---
        -0.2f, 0.0f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.5f, 0.25f, 0.12f,
         0.2f, 0.0f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.5f, 0.25f, 0.12f,
         0.0f, 2.0f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.0f, 0.0f,   0.55f, 0.27f, 0.13f,
         
        0.0f, 0.0f, -0.2f,    0.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.5f, 0.25f, 0.12f,
        0.0f, 0.0f,  0.2f,    1.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.5f, 0.25f, 0.12f,
        0.0f, 2.0f,  0.0f,    0.5f, 1.0f,   0.0f,  0.0f, -1.0f,  0.55f, 0.27f, 0.13f,

        -0.15f, 0.5f, -0.15f, 0.0f, 0.3f,   -0.7f, 0.0f, -0.7f,  0.48f, 0.24f, 0.11f,
         0.15f, 0.5f,  0.15f, 0.5f, 0.5f,   -0.7f, 0.0f, -0.7f,  0.52f, 0.26f, 0.12f,
         0.0f, 1.8f,  0.0f,   0.25f, 0.8f,  -0.7f, 0.0f, -0.7f,  0.58f, 0.28f, 0.14f,

        // --- Canopy Quad 1 (Dark Green) - Forward facing ---
        -1.5f, 1.5f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.2f, 0.65f, 0.3f,
         1.5f, 1.5f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.2f, 0.65f, 0.3f,
         0.0f, 5.0f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.22f, 0.68f, 0.32f,

        // --- Canopy Quad 2 (Dark Green) - Side facing ---
         0.0f, 1.5f, -1.5f,   0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.2f, 0.6f, 0.2f,
         0.0f, 1.5f,  1.5f,   1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.2f, 0.6f, 0.2f,
         0.0f, 5.0f,  0.0f,   0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.22f, 0.63f, 0.22f,
        
        // --- Canopy Quad 3 (Middle layer, higher) ---
        -1.2f, 2.8f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.19f, 0.62f, 0.28f,
         1.2f, 2.8f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.19f, 0.62f, 0.28f,
         0.0f, 4.5f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.21f, 0.65f, 0.3f,

        // --- Canopy Quad 4 (Other side, middle layer) ---
         0.0f, 2.8f, -1.2f,   0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.19f, 0.57f, 0.19f,
         0.0f, 2.8f,  1.2f,   1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.19f, 0.57f, 0.19f,
         0.0f, 4.5f,  0.0f,   0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.21f, 0.6f, 0.21f,
    };
    int treeVertexCount = sizeof(treeVertices) / (11 * sizeof(float));

    unsigned int treeVAO, treeVBO, instanceVBO;
    glGenVertexArrays(1, &treeVAO);
    glGenBuffers(1, &treeVBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(treeVAO);

    // Tree geometry
    glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(treeVertices), treeVertices, GL_STATIC_DRAW);
    
    int stride = 11 * sizeof(float);
    // aPos (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    // aTexCoords (location 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // aNormal (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    // aColor (location 3)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // Instance offsets (location 4)
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, translations.size() * sizeof(glm::vec3),
                 translations.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1); // one offset per tree instance

    glBindVertexArray(0);

    // ---- 7. Building Shader and Geometry ----
    Shader buildingShader("assets/shaders/building.vert", "assets/shaders/building.frag");

    // Create house geometry: walls (box) + roof (pyramid) - stored as TRIANGLES
    // X, Y, Z, NormalX, NormalY, NormalZ, R, G, B
    float buildingVertices[] = {
        // Front face - 2 triangles
        -1.5f, 0.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.6f, 0.4f, 0.2f,
         1.5f, 0.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.6f, 0.4f, 0.2f,
         1.5f, 3.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.65f, 0.42f, 0.22f,
        -1.5f, 0.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.6f, 0.4f, 0.2f,
         1.5f, 3.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.65f, 0.42f, 0.22f,
        -1.5f, 3.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.65f, 0.42f, 0.22f,
        // Back face - 2 triangles
         1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.6f, 0.4f, 0.2f,
        -1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.6f, 0.4f, 0.2f,
        -1.5f, 3.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.65f, 0.42f, 0.22f,
         1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.6f, 0.4f, 0.2f,
        -1.5f, 3.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.65f, 0.42f, 0.22f,
         1.5f, 3.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.65f, 0.42f, 0.22f,
        // Right face - 2 triangles
         1.5f, 0.0f, -1.5f,   1.0f, 0.0f, 0.0f,    0.58f, 0.38f, 0.18f,
         1.5f, 0.0f, 1.5f,    1.0f, 0.0f, 0.0f,    0.58f, 0.38f, 0.18f,
         1.5f, 3.0f, 1.5f,    1.0f, 0.0f, 0.0f,    0.63f, 0.41f, 0.21f,
         1.5f, 0.0f, -1.5f,   1.0f, 0.0f, 0.0f,    0.58f, 0.38f, 0.18f,
         1.5f, 3.0f, 1.5f,    1.0f, 0.0f, 0.0f,    0.63f, 0.41f, 0.21f,
         1.5f, 3.0f, -1.5f,   1.0f, 0.0f, 0.0f,    0.63f, 0.41f, 0.21f,
        // Left face - 2 triangles
        -1.5f, 0.0f, 1.5f,    -1.0f, 0.0f, 0.0f,   0.58f, 0.38f, 0.18f,
        -1.5f, 0.0f, -1.5f,   -1.0f, 0.0f, 0.0f,   0.58f, 0.38f, 0.18f,
        -1.5f, 3.0f, -1.5f,   -1.0f, 0.0f, 0.0f,   0.63f, 0.41f, 0.21f,
        -1.5f, 0.0f, 1.5f,    -1.0f, 0.0f, 0.0f,   0.58f, 0.38f, 0.18f,
        -1.5f, 3.0f, -1.5f,   -1.0f, 0.0f, 0.0f,   0.63f, 0.41f, 0.21f,
        -1.5f, 3.0f, 1.5f,    -1.0f, 0.0f, 0.0f,   0.63f, 0.41f, 0.21f,
        // Roof front - single triangle
        -1.5f, 3.0f, -1.5f,   0.0f, 0.7f, -0.7f,   0.8f, 0.2f, 0.2f,
         1.5f, 3.0f, -1.5f,   0.0f, 0.7f, -0.7f,   0.8f, 0.2f, 0.2f,
         0.0f, 4.5f, 0.0f,    0.0f, 0.7f, -0.7f,   0.85f, 0.22f, 0.22f,
        // Roof back - single triangle
         1.5f, 3.0f, 1.5f,    0.0f, 0.7f, 0.7f,    0.75f, 0.18f, 0.18f,
        -1.5f, 3.0f, 1.5f,    0.0f, 0.7f, 0.7f,    0.75f, 0.18f, 0.18f,
         0.0f, 4.5f, 0.0f,    0.0f, 0.7f, 0.7f,    0.8f, 0.2f, 0.2f,
        // Windows (light blue) - 2 triangles
        -0.5f, 1.5f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
         0.5f, 1.5f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
         0.5f, 2.0f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
        -0.5f, 1.5f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
         0.5f, 2.0f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
        -0.5f, 2.0f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
    };
    int buildingVertexCount = sizeof(buildingVertices) / (9 * sizeof(float));

    unsigned int buildingVAO, buildingVBO, buildingInstanceVBO;
    glGenVertexArrays(1, &buildingVAO);
    glGenBuffers(1, &buildingVBO);
    glGenBuffers(1, &buildingInstanceVBO);

    glBindVertexArray(buildingVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buildingVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buildingVertices), buildingVertices, GL_STATIC_DRAW);

    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Color (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Building instances
    std::vector<glm::vec3> buildingTranslations;
    // Town layout - flat area at (100-150, z=150-200)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float x = 100.0f + i * 7.0f;
            float z = 150.0f + j * 7.0f;
            float townTerrainHeight = 10.0f; // Town is on relatively flat terrain
            buildingTranslations.push_back(glm::vec3(x, townTerrainHeight, z));
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, buildingInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, buildingTranslations.size() * sizeof(glm::vec3),
                 buildingTranslations.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    // ---- 8. Person Shader and Geometry ----
    Shader personShader("assets/shaders/person.vert", "assets/shaders/person.frag");

    // Simple person geometry: head (cube), body (taller cube), arms/legs (small cubes) - as TRIANGLES
    float personVertices[] = {
        // Head (skin color) - 2 triangles
        -0.2f, 1.8f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
         0.2f, 1.8f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
         0.2f, 2.2f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
        -0.2f, 1.8f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
         0.2f, 2.2f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
        -0.2f, 2.2f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
        // Body (red shirt) - 2 triangles
        -0.35f, 1.0f, -0.25f, 0.0f, 0.0f, -1.0f,   0.8f, 0.2f, 0.2f,
         0.35f, 1.0f, -0.25f, 0.0f, 0.0f, -1.0f,   0.8f, 0.2f, 0.2f,
         0.35f, 1.8f, -0.25f, 0.0f, 0.0f, -1.0f,   0.9f, 0.25f, 0.25f,
        -0.35f, 1.0f, -0.25f, 0.0f, 0.0f, -1.0f,   0.8f, 0.2f, 0.2f,
         0.35f, 1.8f, -0.25f, 0.0f, 0.0f, -1.0f,   0.9f, 0.25f, 0.25f,
        -0.35f, 1.8f, -0.25f, 0.0f, 0.0f, -1.0f,   0.9f, 0.25f, 0.25f,
        // Left arm (blue) - 2 triangles
        -0.5f, 1.4f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.2f, 0.3f, 0.8f,
        -0.4f, 1.4f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.2f, 0.3f, 0.8f,
        -0.4f, 1.0f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.25f, 0.35f, 0.85f,
        -0.5f, 1.4f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.2f, 0.3f, 0.8f,
        -0.4f, 1.0f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.25f, 0.35f, 0.85f,
        -0.5f, 1.0f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.25f, 0.35f, 0.85f,
        // Right arm (blue) - 2 triangles
         0.4f, 1.4f, -0.15f,  1.0f, 0.0f, 0.0f,    0.2f, 0.3f, 0.8f,
         0.5f, 1.4f, -0.15f,  1.0f, 0.0f, 0.0f,    0.2f, 0.3f, 0.8f,
         0.5f, 1.0f, -0.15f,  1.0f, 0.0f, 0.0f,    0.25f, 0.35f, 0.85f,
         0.4f, 1.4f, -0.15f,  1.0f, 0.0f, 0.0f,    0.2f, 0.3f, 0.8f,
         0.5f, 1.0f, -0.15f,  1.0f, 0.0f, 0.0f,    0.25f, 0.35f, 0.85f,
         0.4f, 1.0f, -0.15f,  1.0f, 0.0f, 0.0f,    0.25f, 0.35f, 0.85f,
        // Legs (brown pants) - 2 triangles
        -0.2f, 0.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.4f, 0.3f, 0.2f,
         0.2f, 0.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.4f, 0.3f, 0.2f,
         0.2f, 1.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.5f, 0.4f, 0.25f,
        -0.2f, 0.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.4f, 0.3f, 0.2f,
         0.2f, 1.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.5f, 0.4f, 0.25f,
        -0.2f, 1.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.5f, 0.4f, 0.25f,
    };
    int personVertexCount = sizeof(personVertices) / (9 * sizeof(float));

    unsigned int personVAO, personVBO, personInstanceVBO;
    glGenVertexArrays(1, &personVAO);
    glGenBuffers(1, &personVBO);
    glGenBuffers(1, &personInstanceVBO);

    glBindVertexArray(personVAO);
    glBindBuffer(GL_ARRAY_BUFFER, personVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(personVertices), personVertices, GL_STATIC_DRAW);

    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Color (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // Person instances (NPCs in town)
    std::vector<glm::vec3> personTranslations;
    srand(99); // Different seed for people
    for (int i = 0; i < 30; i++) {
        float x = 100.0f + (rand() % 50) * 0.5f;
        float z = 150.0f + (rand() % 50) * 0.5f;
        personTranslations.push_back(glm::vec3(x, 10.0f, z));
    }

    glBindBuffer(GL_ARRAY_BUFFER, personInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, personTranslations.size() * sizeof(glm::vec3),
                 personTranslations.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    glBindVertexArray(0);

    // ---- Particle System (Fireflies) ----
    Shader particleShader("assets/shaders/particle.vert", "assets/shaders/particle.frag");
    
    const int MAX_PARTICLES = 500;
    std::vector<glm::vec3> particlePositions;
    std::vector<glm::vec3> particleVelocities;
    srand(42); // Different seed for particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        glm::vec3 pos(
            (rand() % 400) - 200.0f,  // x: -200 to 200
            50.0f + (rand() % 100),    // y: 50 to 150
            (rand() % 400) - 200.0f    // z: -200 to 200
        );
        glm::vec3 vel(
            (rand() % 11 - 5) * 0.1f,  // vx: -0.5 to 0.5
            0.5f + (rand() % 5) * 0.1f, // vy: 0.5 to 1.0 (upward)
            (rand() % 11 - 5) * 0.1f    // vz: -0.5 to 0.5
        );
        particlePositions.push_back(pos);
        particleVelocities.push_back(vel);
    }

    unsigned int particleVAO, particleVBO;
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);

    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, particlePositions.size() * sizeof(glm::vec3),
                 particlePositions.data(), GL_DYNAMIC_DRAW);

    // Position attribute for particles
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // ---- Water System (Procedural Sine Waves) ----
    Shader waterShader("assets/shaders/water.vert", "assets/shaders/water.frag");

    const int WATER_W = 500, WATER_D = 500;
    std::vector<glm::vec3> waterVertices;
    std::vector<unsigned int> waterIndices;
    
    float waterSpacing = 2.0f;  // Covers ±500 units around center (island surrounded by ocean)
    for (int z = 0; z < WATER_D; z++) {
        for (int x = 0; x < WATER_W; x++) {
            float xPos = -500.0f + x * waterSpacing;
            float zPos = -500.0f + z * waterSpacing;
            waterVertices.push_back(glm::vec3(xPos, -10.0f, zPos));
        }
    }

    // Create triangle indices for water grid
    for (int z = 0; z < WATER_D - 1; z++) {
        for (int x = 0; x < WATER_W - 1; x++) {
            // Triangle 1
            waterIndices.push_back(z * WATER_W + x);
            waterIndices.push_back((z + 1) * WATER_W + x);
            waterIndices.push_back(z * WATER_W + (x + 1));
            // Triangle 2
            waterIndices.push_back((z + 1) * WATER_W + x);
            waterIndices.push_back((z + 1) * WATER_W + (x + 1));
            waterIndices.push_back(z * WATER_W + (x + 1));
        }
    }

    unsigned int waterVAO, waterVBO, waterEBO;
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
    glGenBuffers(1, &waterEBO);

    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(glm::vec3),
                 waterVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterIndices.size() * sizeof(unsigned int),
                 waterIndices.data(), GL_STATIC_DRAW);

    // Position attribute for water
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Fog & Sky color
    glm::vec3 skyColor(0.7f, 0.8f, 1.0f); // Bright sky blue

    // ---- 7. Render Loop ----
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrices
        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
        float aspect = (currentHeight == 0) ? 1.0f : (float)currentWidth / (float)currentHeight;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                 aspect,
                                                 0.1f, 2000.0f); // Far clip plane increased for mountains
        glm::mat4 view  = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // --- Draw Terrain ---
        terrainShader.use();
        terrainShader.setMat4("projection", projection);
        terrainShader.setMat4("view",       view);
        terrainShader.setMat4("model",      model);
        terrainShader.setVec3("skyColor", skyColor);

        // Bind the Perlin noise heightmap to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, heightmapTex);

        terrain.Draw();

        // --- Draw Instanced Trees ---
        // Disable backface culling for trees so we can see the cross-quads from all sides
        glDisable(GL_CULL_FACE);
        
        treeShader.use();
        treeShader.setMat4("projection", projection);
        treeShader.setMat4("view",       view);
        treeShader.setVec3("skyColor",   skyColor);

        glBindVertexArray(treeVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, treeVertexCount, translations.size());
        glBindVertexArray(0);

        // --- Draw Instanced Buildings ---
        buildingShader.use();
        buildingShader.setMat4("projection", projection);
        buildingShader.setMat4("view",       view);
        buildingShader.setVec3("skyColor",   skyColor);

        glBindVertexArray(buildingVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, buildingVertexCount, buildingTranslations.size());
        glBindVertexArray(0);

        // --- Draw Instanced NPCs (People) ---
        personShader.use();
        personShader.setMat4("projection", projection);
        personShader.setMat4("view",       view);
        personShader.setVec3("skyColor",   skyColor);
        personShader.setFloat("u_Time",    currentFrame);

        glBindVertexArray(personVAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, personVertexCount, personTranslations.size());
        glBindVertexArray(0);

        glEnable(GL_CULL_FACE);

        // ---- Day/Night Cycle Calculation ----
        float daySpeed = 0.1f;  // Slower day cycle - one full day takes much longer
        float sunRadius = 100.0f;
        float sunX = sunRadius * cos(currentFrame * daySpeed);
        float sunY = sunRadius * sin(currentFrame * daySpeed);
        glm::vec3 sunPos(sunX, sunY, 0.0f);
        
        // Calculate light color based on sun position (Y ranges from -100 to +100)
        float timeOfDay = (sunY + sunRadius) / (2.0f * sunRadius); // 0 to 1
        glm::vec3 lightColor = glm::mix(
            glm::vec3(0.3f, 0.2f, 0.5f),  // Night: dark blue
            glm::vec3(1.0f, 1.0f, 0.9f),   // Day: bright yellow-white
            glm::clamp(timeOfDay, 0.0f, 1.0f)
        );
        float lightIntensity = glm::mix(0.2f, 1.0f, glm::clamp(timeOfDay, 0.0f, 1.0f));
        
        // Update sky color based on sun position
        skyColor = glm::mix(
            glm::vec3(0.1f, 0.1f, 0.2f),   // Night sky
            glm::vec3(0.7f, 0.8f, 1.0f),   // Day sky
            glm::clamp(timeOfDay, 0.0f, 1.0f)
        );

        // ---- Draw Water ----
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_CULL_FACE);

        waterShader.use();
        waterShader.setMat4("projection", projection);
        waterShader.setMat4("view",       view);
        waterShader.setMat4("model",      glm::mat4(1.0f));
        waterShader.setFloat("u_Time",    currentFrame);
        waterShader.setVec3("u_LightColor", lightColor * lightIntensity);

        glBindVertexArray(waterVAO);
        glDrawElements(GL_TRIANGLES, waterIndices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);

        // ---- Draw Particles (Fireflies) ----
        // Update particle positions and reupload to GPU
        for (int i = 0; i < particlePositions.size(); i++) {
            particlePositions[i] += particleVelocities[i] * deltaTime;
            
            // Wrap particles around vertically (loop back to bottom when reaching top)
            if (particlePositions[i].y > 150.0f) {
                particlePositions[i].y = 50.0f;
            }
            
            // Add slight jitter to velocity for fluttering effect
            particleVelocities[i].x += (rand() % 11 - 5) * 0.001f;
            particleVelocities[i].z += (rand() % 11 - 5) * 0.001f;
            particleVelocities[i].x = glm::clamp(particleVelocities[i].x, -0.5f, 0.5f);
            particleVelocities[i].z = glm::clamp(particleVelocities[i].z, -0.5f, 0.5f);
        }

        glBindBuffer(GL_COPY_READ_BUFFER, 0); // Clear binding
        glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, particlePositions.size() * sizeof(glm::vec3),
                        particlePositions.data());

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow effect
        glEnable(GL_PROGRAM_POINT_SIZE);

        particleShader.use();
        particleShader.setMat4("projection", projection);
        particleShader.setMat4("view",       view);

        glBindVertexArray(particleVAO);
        glDrawArrays(GL_POINTS, 0, particlePositions.size());
        glBindVertexArray(0);

        glDisable(GL_PROGRAM_POINT_SIZE);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Cleanup ----
    glDeleteVertexArrays(1, &treeVAO);
    glDeleteBuffers(1, &treeVBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteVertexArrays(1, &buildingVAO);
    glDeleteBuffers(1, &buildingVBO);
    glDeleteBuffers(1, &buildingInstanceVBO);
    glDeleteVertexArrays(1, &personVAO);
    glDeleteBuffers(1, &personVBO);
    glDeleteBuffers(1, &personInstanceVBO);
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);
    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
    glDeleteBuffers(1, &waterEBO);
    glDeleteTextures(1, &heightmapTex);

    glfwTerminate();
    return 0;
}
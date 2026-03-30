// ============================================================
// OpenGL Terrain Visualization - Modular Architecture
// Systems: Terrain, Trees, Buildings, NPCs, Particles, Water, Lighting, Grass, Rain
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
#include "RainSystem.h"
#include "SplashSystem.h"
#include "CinematicSystem.h"

// ---- Window dimensions ----
const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;

// ---- Global camera (used in callbacks) ----
Camera camera(glm::vec3(125.0f, 200.0f, 175.0f));
float lastX      = SCR_WIDTH  / 2.0f;
float lastY      = SCR_HEIGHT / 2.0f;
bool  firstMouse = true;
bool  g_cinematicMode = true;

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
    if (g_cinematicMode) {
        return;
    }

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
    if (g_cinematicMode) {
        return;
    }
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Rain system forward reference for key callback
RainSystem* g_rainSystem = nullptr;
bool g_rainEnabled = false;

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

    // Toggle rain with R
    static bool rWasDown = false;
    bool rIsDown = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;
    if (rIsDown && !rWasDown && g_rainSystem) {
        g_rainSystem->Toggle();
        g_rainEnabled = !g_rainEnabled;
    }
    rWasDown = rIsDown;
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

unsigned int createFullscreenQuad() {
    float vertices[] = {
        -1.0f, -1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f,  1.0f,
        -1.0f,  1.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);

    return quadVAO;
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
    Terrain       terrain(800, 800);
    TreeSystem    treeSystem(noiseData, HM_W, HM_H, 800, 800);
    GrassSystem   grassSystem(noiseData, HM_W, HM_H, 800, 800);
    BuildingSystem buildingSystem;
    NPCSystem     npcSystem;
    ParticleSystem particleSystem(500, noiseData, HM_W, HM_H, 800.0f, 800.0f);
    WaterSystem   waterSystem(2000, 2000);
    LightingSystem lightingSystem;
    RainSystem    rainSystem(5000);
    SplashSystem  splashSystem(200, noiseData, HM_W, HM_H, 800.0f, 800.0f);
    CinematicSystem cinematic;
    cinematic.init();
    g_rainSystem = &rainSystem;

    // ---- 5. Load Shaders ----
    Shader terrainShader ("assets/shaders/terrain.vert",  "assets/shaders/terrain.frag");
    Shader treeShader    ("assets/shaders/tree.vert",     "assets/shaders/tree.frag");
    Shader grassShader   ("assets/shaders/grass.vert",    "assets/shaders/grass.frag");
    Shader buildingShader("assets/shaders/building.vert", "assets/shaders/building.frag");
    Shader particleShader("assets/shaders/particle.vert", "assets/shaders/particle.frag");
    Shader waterShader   ("assets/shaders/water.vert",    "assets/shaders/water.frag");
    Shader skyboxShader  ("assets/shaders/skybox.vert",   "assets/shaders/skybox.frag");
    Shader rainShader    ("assets/shaders/rain.vert",     "assets/shaders/rain.frag");
    Shader splashShader  ("assets/shaders/splash.vert",   "assets/shaders/splash.frag");
    Shader depthShader   ("assets/shaders/depth.vert",    "assets/shaders/depth.frag");
    Shader personShader  ("assets/shaders/person.vert",   "assets/shaders/person.frag");
    Shader fadeShader    ("assets/shaders/fade.vert",     "assets/shaders/fade.frag");

    // Tell terrain shader which texture unit holds the heightmap
    terrainShader.use();
    terrainShader.setInt("heightMap", 0);

    // Create skybox VAO
    unsigned int skyboxVAO = createSkyboxCube();
    unsigned int fadeQuadVAO = createFullscreenQuad();

    float fadeAlpha = 0.0f;

    // ---- 6. Render Loop ----
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (g_cinematicMode) {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                g_cinematicMode = false;
                lightingSystem.setTimeScale(1.0f);
                fadeAlpha = 0.0f;
                firstMouse = true;
            } else {
                bool stillPlaying = cinematic.update(deltaTime, camera, &lightingSystem);
                if (cinematic.currentTime() >= 369.0f) {
                    float t = (cinematic.currentTime() - 369.0f) / 6.0f;
                    fadeAlpha = glm::clamp(t, 0.0f, 1.0f);
                }
                if (!stillPlaying) {
                    g_cinematicMode = false;
                    firstMouse = true;
                }
            }
        } else {
            processInput(window);
            lightingSystem.setTimeScale(1.0f);
            fadeAlpha = 0.0f;
        }

        // Update lighting/day-night cycle using delta time.
        lightingSystem.Update(deltaTime);

        // Compute dynamic wind strength (slow sine, 0->1)
        float windStrength = 0.5f + 0.5f * std::sin(currentFrame * 0.15f);

        // Update moving systems
        particleSystem.Update(deltaTime);
        rainSystem.Update(deltaTime, camera.Position);
        splashSystem.Update(deltaTime, g_rainEnabled, camera.Position);

        // Retrieve lighting values once per frame
        glm::vec3 lightDir    = lightingSystem.GetLightDir();
        glm::vec3 lightColor  = lightingSystem.GetSunColor() * lightingSystem.GetSunIntensity();
        glm::vec3 sunsetTint  = lightingSystem.GetSunsetTint();
        glm::vec3 skyColor    = lightingSystem.GetSkyColor();
        float     dayIntensity = lightingSystem.GetDayIntensity();

        // Get window dimensions
        int currentWidth, currentHeight;
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);

        // ============================================================
        // PASS 1: DEPTH PASS - Render from light's perspective
        // (now includes trees, buildings for proper shadows)
        // ============================================================
        glm::mat4 lightProj = lightingSystem.GetLightProjection();
        glm::mat4 lightView = lightingSystem.GetLightView();

        glViewport(0, 0, 2048, 2048);
        glBindFramebuffer(GL_FRAMEBUFFER, lightingSystem.GetDepthMapFBO());
        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightingSystem.GetLightSpaceMatrix());
        depthShader.setMat4("model", glm::mat4(1.0f));

        // Draw terrain to depth map
        terrain.Draw();

        // Draw trees to depth map – they now cast shadows on the terrain!
        treeSystem.Render(lightProj, lightView, glm::vec3(0.0f), depthShader);

        // Draw buildings to depth map
        buildingSystem.Render(lightProj, lightView, glm::vec3(0.0f), depthShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // ============================================================
        // PASS 2: COLOR PASS - Render from camera's perspective
        // ============================================================
        glViewport(0, 0, currentWidth, currentHeight);

        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate matrices
        float aspect = (currentHeight == 0) ? 1.0f : (float)currentWidth / (float)currentHeight;
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), aspect, 0.1f, 2000.0f);
        glm::mat4 view  = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // --- Helper lambda: set common scene uniforms on a shader ---
        auto setSceneUniforms = [&](Shader& sh) {
            sh.use();
            sh.setVec3("skyColor",    skyColor);
            sh.setVec3("lightDir",    lightDir);
            sh.setVec3("lightColor",  lightColor);
            sh.setVec3("sunsetTint",  sunsetTint);
            sh.setVec3("cameraPos",   camera.Position);
        };

        setSceneUniforms(terrainShader);
        setSceneUniforms(treeShader);
        setSceneUniforms(grassShader);
        setSceneUniforms(buildingShader);
        setSceneUniforms(waterShader);

        // --- Draw Terrain (with shadows) ---
        terrainShader.use();
        terrainShader.setMat4("projection",      projection);
        terrainShader.setMat4("view",            view);
        terrainShader.setMat4("model",           model);
        terrainShader.setMat4("lightSpaceMatrix", lightingSystem.GetLightSpaceMatrix());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, heightmapTex);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, lightingSystem.GetDepthMapTexture());
        terrainShader.setInt("shadowMap", 1);

        terrain.Draw();

        // --- Draw Grass ---
        grassShader.use();
        grassShader.setMat4 ("projection",   projection);
        grassShader.setMat4 ("view",         view);
        grassShader.setFloat("u_Time",       currentFrame);
        grassShader.setFloat("windStrength", windStrength);
        grassSystem.Draw();

        // --- Draw Trees (with canopy sway) ---
        treeShader.use();
        treeShader.setFloat("u_Time",       currentFrame);
        treeShader.setFloat("windStrength", windStrength);
        treeSystem.Render(projection, view, skyColor, treeShader);

        // --- Draw Buildings ---
        buildingSystem.Render(projection, view, skyColor, buildingShader);


        // --- Draw Water ---
        waterShader.use();
        waterShader.setVec3("lightDir",   lightDir);
        waterShader.setVec3("lightColor", lightColor);
        waterShader.setVec3("sunsetTint", sunsetTint);
        waterShader.setVec3("cameraPos",  camera.Position);
        waterShader.setVec3("skyColor",   skyColor);
        waterSystem.Render(projection, view, waterShader, currentFrame,
                           lightingSystem.GetSunColor() * lightingSystem.GetSunIntensity());

        // --- Draw Firefly Particles (only at night) ---
        particleShader.use();
        particleShader.setFloat("dayIntensity", dayIntensity);
        particleSystem.Render(projection, view, particleShader);

        // --- Draw Rain (if enabled, toggle with R) ---
        rainSystem.Render(projection, view, rainShader);

        // --- Draw Rain Splashes ---
        splashSystem.Render(projection, view, splashShader);

        // --- Draw static cinematic NPCs ---
        if (g_cinematicMode) {
            auto renderStaticNPC = [&](const StaticNPC& npc) {
                if (!npc.visible) {
                    return;
                }

                personShader.use();
                personShader.setMat4("projection", projection);
                personShader.setMat4("view", view);
                personShader.setMat4("model", npc.modelMatrix);
                personShader.setInt("uUseModel", 1);
                personShader.setFloat("u_Time", 0.0f);
                personShader.setVec3("skyColor", skyColor);
                personShader.setVec3("lightDir", lightDir);
                personShader.setVec3("lightColor", lightColor);
                personShader.setVec3("sunsetTint", sunsetTint);
                personShader.setFloat("ambientStrength", 0.25f);

                glBindVertexArray(npcSystem.GetVAO());
                glDrawArrays(GL_TRIANGLES, 0, npcSystem.GetVertexCount());
                glBindVertexArray(0);
            };

            renderStaticNPC(cinematic.npcA());
            renderStaticNPC(cinematic.npcB());
            renderStaticNPC(cinematic.npcC());
        }

        // === Draw Skybox (last, with special depth handling) ===
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        glm::mat4 skyView = glm::mat4(glm::mat3(view)); // remove translation
        skyboxShader.setMat4("view",       skyView);
        skyboxShader.setMat4("projection", projection);
        skyboxShader.setVec3("sunPos",     lightingSystem.GetSunPosition());

        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);

        if (fadeAlpha > 0.0f) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            fadeShader.use();
            fadeShader.setFloat("alpha", fadeAlpha);
            glBindVertexArray(fadeQuadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);

            glDisable(GL_BLEND);
            glEnable(GL_DEPTH_TEST);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ---- Cleanup ----
    glDeleteTextures(1, &heightmapTex);
    glfwTerminate();
    return 0;
}

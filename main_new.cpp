// ============================================================
// OpenGL Terrain Visualization - Modular Architecture
// Systems: Terrain, Trees, Particles, Water, Lighting, Grass, Rain
// Cinematic Scene: "The Island - A Day's Journey" - ~9 minute unbroken sequence
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
#include "CinematicCamera.h"
#include "NoiseMap.h"
#include "TreeSystem.h"
#include "ParticleSystem.h"
#include "WaterSystem.h"
#include "GrassSystem.h"
#include "LightingSystem.h"
#include "RainSystem.h"
#include "SplashSystem.h"

// ---- Window dimensions ----
const unsigned int SCR_WIDTH  = 1280;
const unsigned int SCR_HEIGHT = 720;

// ---- Global camera (used in callbacks) ----
// Using CinematicCamera for terrain-following and scripted shots
CinematicCamera* g_camera = nullptr;
float lastX      = SCR_WIDTH  / 2.0f;
float lastY      = SCR_HEIGHT / 2.0f;
bool  firstMouse = true;

// ---- Timing ----
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ---- Cinematic mode ----
bool g_cinematicMode = false;
float g_cinematicTime = 0.0f;
const float CINEMATIC_DURATION = 525.0f; // ~8.75 minutes for full day cycle
bool g_cinematicPaused = false;

// ---- Time control for dawn sequence ----
// The scene starts at pre-dawn (time ~0.05) and progresses through the day
float g_sceneTimeOfDay = 0.05f;  // Start pre-dawn
const float SCENE_DAY_START = 0.05f;   // Pre-dawn
const float SCENE_DAY_END = 0.85f;     // Late evening

// Rain system forward reference for key callback
RainSystem* g_rainSystem = nullptr;
bool g_rainEnabled = false;

// -------------------------------------------------------
// GLFW Callbacks
// -------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* /*win*/, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* /*win*/, double xposIn, double yposIn) {
    if (!g_camera || !g_camera->IsUserControlEnabled()) return;

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

    g_camera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* /*win*/, double /*xoffset*/, double yoffset) {
    if (g_camera) {
        g_camera->ProcessMouseScroll(static_cast<float>(yoffset));
    }
}

void processInput(GLFWwindow* window) {
    if (!g_camera) return;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement only in non-cinematic or user control mode
    if (g_camera->IsUserControlEnabled()) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) g_camera->ProcessKeyboard(FORWARD,  deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) g_camera->ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) g_camera->ProcessKeyboard(LEFT,     deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) g_camera->ProcessKeyboard(RIGHT,    deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) g_camera->ProcessKeyboard(UP,       deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) g_camera->ProcessKeyboard(DOWN,     deltaTime);

        // Keyboard-look fallback for laptops where touchpad input is limited while typing.
        const float lookSpeedDegPerSec = 95.0f;
        const float lookDelta = lookSpeedDegPerSec * deltaTime;
        const float mouseEquivalent = lookDelta / g_camera->MouseSensitivity;
        if (glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS) g_camera->ProcessMouseMovement(-mouseEquivalent, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) g_camera->ProcessMouseMovement( mouseEquivalent, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS) g_camera->ProcessMouseMovement(0.0f,  mouseEquivalent);
        if (glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS) g_camera->ProcessMouseMovement(0.0f, -mouseEquivalent);
    }

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

    // Toggle cinematic mode with C
    static bool cWasDown = false;
    bool cIsDown = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
    if (cIsDown && !cWasDown) {
        g_cinematicMode = !g_cinematicMode;
        if (g_cinematicMode) {
            g_cinematicTime = 0.0f;
            g_sceneTimeOfDay = SCENE_DAY_START;
            g_camera->StartCinematic();
            std::cout << "[Scene] The Island at Dawn - Started" << std::endl;
        } else {
            g_camera->StopCinematic();
            std::cout << "[Scene] Cinematic mode ended" << std::endl;
        }
    }
    cWasDown = cIsDown;

    // Pause cinematic with Space
    static bool spaceWasDown = false;
    bool spaceIsDown = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;
    if (spaceIsDown && !spaceWasDown && g_cinematicMode) {
        g_cinematicPaused = !g_cinematicPaused;
        std::cout << "[Scene] " << (g_cinematicPaused ? "Paused" : "Resumed") << std::endl;
    }
    spaceWasDown = spaceIsDown;

    // Reset cinematic with Backspace
    static bool backspaceWasDown = false;
    bool backspaceIsDown = glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS;
    if (backspaceIsDown && !backspaceWasDown && g_cinematicMode) {
        g_cinematicTime = 0.0f;
        g_sceneTimeOfDay = SCENE_DAY_START;
        std::cout << "[Scene] Reset to beginning" << std::endl;
    }
    backspaceWasDown = backspaceIsDown;
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
                                          "The Island at Dawn - Press C to start cinematic",
                                          nullptr, nullptr);
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
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

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
    std::cout << "[DEBUG] TreeSystem initialized. Total trees: " << treeSystem.GetTotalTreeCount() << std::endl;
    GrassSystem   grassSystem(noiseData, HM_W, HM_H, 800, 800);
    ParticleSystem particleSystem(500, noiseData, HM_W, HM_H, 800.0f, 800.0f);
    WaterSystem   waterSystem(2000, 2000);
    LightingSystem lightingSystem;
    RainSystem    rainSystem(1000000);
    std::cout << "[DEBUG] RainSystem initialized with max drops: 500000" << std::endl;
    SplashSystem  splashSystem(200000, noiseData, HM_W, HM_H, 800.0f, 800.0f);
    std::cout << "[DEBUG] SplashSystem initialized with max splashes: 200000" << std::endl;
    g_rainSystem = &rainSystem;

    // Initialize Cinematic Camera with terrain data for height-following
    CinematicCamera cinematicCamera(noiseData, HM_W, HM_H, 800.0f, 800.0f);
    g_camera = &cinematicCamera;

    // ---- 5. Load Shaders ----
    Shader terrainShader ("assets/shaders/terrain.vert",  "assets/shaders/terrain.frag");
    Shader treeShader    ("assets/shaders/tree.vert",     "assets/shaders/tree.frag");
    Shader grassShader   ("assets/shaders/grass.vert",    "assets/shaders/grass.frag");
    Shader particleShader("assets/shaders/particle.vert", "assets/shaders/particle.frag");
    Shader waterShader   ("assets/shaders/water.vert",    "assets/shaders/water.frag");
    Shader skyboxShader  ("assets/shaders/skybox.vert",   "assets/shaders/skybox.frag");
    Shader rainShader    ("assets/shaders/rain.vert",     "assets/shaders/rain.frag");
    Shader splashShader  ("assets/shaders/splash.vert",   "assets/shaders/splash.frag");
    Shader depthShader   ("assets/shaders/depth.vert",    "assets/shaders/depth.frag");

    std::cout << "[DEBUG] All shaders loaded successfully" << std::endl;
    std::cout << "[DEBUG] Rain enabled: " << rainSystem.IsEnabled() << std::endl;

    // Tell terrain shader which texture unit holds the heightmap
    terrainShader.use();
    terrainShader.setInt("heightMap", 0);

    // Create skybox VAO
    unsigned int skyboxVAO = createSkyboxCube();

    // ---- 6. Render Loop ----
    while (!glfwWindowShouldClose(window)) {
        // Process pending input events first so mouse-look and keyboard movement
        // are applied in the same frame.
        glfwPollEvents();

        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Cap delta time to avoid jumps
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        processInput(window);

        // -----------------------------------------------------------
        // Cinematic Mode: Update camera and time-of-day
        // -----------------------------------------------------------
        if (g_cinematicMode && !g_cinematicPaused) {
            // Update cinematic time
            g_cinematicTime += deltaTime;

            // Full day/night cycle progression (525 seconds)
            // 0-60s: Night (stars visible)
            // 60-120s: Dawn/Sunrise transition
            // 120-300s: Full day
            // 300-390s: Dusk/Sunset transition
            // 390-525s: Night (moon and fireflies)
            float timeProgress = g_cinematicTime / CINEMATIC_DURATION;
            if (timeProgress > 1.0f) timeProgress = 1.0f;

            // Map to full day cycle: 0.0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk, 1.0=midnight
            if (timeProgress < 0.114f) {
                // Night start (0.0 - 0.114) -> timeOfDay 0.0 - 0.1
                g_sceneTimeOfDay = timeProgress / 0.114f * 0.1f;
            } else if (timeProgress < 0.229f) {
                // Dawn transition (0.114 - 0.229) -> timeOfDay 0.1 - 0.35
                float t = (timeProgress - 0.114f) / 0.115f;
                g_sceneTimeOfDay = 0.1f + t * 0.25f;
            } else if (timeProgress < 0.571f) {
                // Day (0.229 - 0.571) -> timeOfDay 0.35 - 0.65
                float t = (timeProgress - 0.229f) / 0.342f;
                g_sceneTimeOfDay = 0.35f + t * 0.3f;
            } else if (timeProgress < 0.743f) {
                // Dusk transition (0.571 - 0.743) -> timeOfDay 0.65 - 0.85
                float t = (timeProgress - 0.571f) / 0.172f;
                g_sceneTimeOfDay = 0.65f + t * 0.2f;
            } else {
                // Night end (0.743 - 1.0) -> timeOfDay 0.85 - 1.0 (back to midnight)
                float t = (timeProgress - 0.743f) / 0.257f;
                g_sceneTimeOfDay = 0.85f + t * 0.15f;
            }

            // Update camera
            cinematicCamera.Update(g_cinematicTime, deltaTime);

            // Show shot name periodically
            static float lastShotDisplay = 0.0f;
            if (g_cinematicTime - lastShotDisplay > 8.0f) {
                std::cout << "[Scene] " << cinematicCamera.GetCurrentShotName()
                          << " (Time: " << (int)g_cinematicTime << "s, Day: "
                          << (int)(g_sceneTimeOfDay * 100) << "%)" << std::endl;
                lastShotDisplay = g_cinematicTime;
            }

            // End cinematic after duration
            if (g_cinematicTime >= CINEMATIC_DURATION) {
                std::cout << "[Scene] The Island - A Day's Journey - Complete" << std::endl;
                g_cinematicMode = false;
                cinematicCamera.StopCinematic();
            }
        } else if (!g_cinematicMode) {
            // Free camera mode - apply terrain collision
            float terrainH = cinematicCamera.GetTerrainHeightClamped(
                cinematicCamera.Position.x, cinematicCamera.Position.z);
            float minHeight = terrainH + 2.0f;
            if (cinematicCamera.Position.y < minHeight) {
                cinematicCamera.Position.y = minHeight;
            }
        }

        // Update lighting with controlled time of day
        if (g_cinematicMode) {
            // Override the lighting system's time with our scene time
            // This creates the dawn-to-dusk progression
            lightingSystem.Update(g_sceneTimeOfDay * 100.0f); // Scale to match expected range
        } else {
            lightingSystem.Update(currentFrame);
        }

        // Compute dynamic wind strength (slow sine, 0->1)
        float windStrength = 0.5f + 0.5f * std::sin(currentFrame * 0.15f);

        // Update moving systems
        particleSystem.Update(deltaTime);
        rainSystem.Update(deltaTime, g_camera->Position);
        splashSystem.Update(deltaTime, g_rainEnabled, g_camera->Position);

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
        // (includes terrain and trees)
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

        // Draw trees to depth map
        treeSystem.Render(lightProj, lightView, glm::vec3(0.0f), depthShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);


        // ============================================================
        // PASS 2: COLOR PASS - Render from camera's perspective
        // ============================================================
        glViewport(0, 0, currentWidth, currentHeight);

        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Calculate matrices
        float aspect = (currentHeight == 0) ? 1.0f : (float)currentWidth / (float)currentHeight;
        glm::mat4 projection = glm::perspective(glm::radians(g_camera->Zoom), aspect, 0.1f, 2000.0f);
        glm::mat4 view  = g_camera->GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // --- Helper lambda: set common scene uniforms on a shader ---
        auto setSceneUniforms = [&](Shader& sh) {
            sh.use();
            sh.setVec3("skyColor",    skyColor);
            sh.setVec3("lightDir",    lightDir);
            sh.setVec3("lightColor",  lightColor);
            sh.setVec3("sunsetTint",  sunsetTint);
            sh.setVec3("cameraPos",   g_camera->Position);
        };

        setSceneUniforms(terrainShader);
        setSceneUniforms(treeShader);
        setSceneUniforms(grassShader);
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
        int treeCount = treeSystem.GetTotalTreeCount();
        if (treeCount > 0) {
            treeSystem.Render(projection, view, skyColor, treeShader);
        }

        // --- Draw Water ---
        waterShader.use();
        waterShader.setVec3("lightDir",   lightDir);
        waterShader.setVec3("lightColor", lightColor);
        waterShader.setVec3("sunsetTint", sunsetTint);
        waterShader.setVec3("cameraPos",  g_camera->Position);
        waterShader.setVec3("skyColor",   skyColor);
        waterSystem.Render(projection, view, waterShader, currentFrame,
                           lightingSystem.GetSunColor() * lightingSystem.GetSunIntensity());

        // --- Draw Firefly Particles (only at night) ---
        // Fireflies appear as day transitions to night (cinematic shot 6-7)
        particleShader.use();
        particleShader.setFloat("dayIntensity", dayIntensity);
        particleSystem.Render(projection, view, particleShader);

        // --- Draw Rain (if enabled, toggle with R) ---
        rainSystem.Render(projection, view, rainShader);

        // --- Draw Rain Splashes ---
        splashSystem.Render(projection, view, splashShader);

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

        glfwSwapBuffers(window);
    }

    // ---- Cleanup ----
    glDeleteTextures(1, &heightmapTex);
    glfwTerminate();
    return 0;
}

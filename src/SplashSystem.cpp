#include "SplashSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdlib>
#include <cmath>

SplashSystem::SplashSystem(int maxSplashes,
                                                     const std::vector<float>& heightmap,
                                                     int hmWidth,
                                                     int hmHeight,
                                                     float terrainWidth,
                                                     float terrainDepth)
        : maxSplashes(maxSplashes),
            terrainHeightmap(heightmap),
            heightmapWidth(hmWidth),
            heightmapHeight(hmHeight),
            terrainWidth(terrainWidth),
            terrainDepth(terrainDepth) {
    glGenVertexArrays(1, &splashVAO);
    glGenBuffers(1, &splashVBO);
    glGenBuffers(1, &sizeVBO);

    // Pre-allocate capacity
    splashes.reserve(maxSplashes);
    positions.reserve(maxSplashes);
    sizes.reserve(maxSplashes);

    glBindVertexArray(splashVAO);

    // Position buffer
    glBindBuffer(GL_ARRAY_BUFFER, splashVBO);
    glBufferData(GL_ARRAY_BUFFER, maxSplashes * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Size buffer
    glBindBuffer(GL_ARRAY_BUFFER, sizeVBO);
    glBufferData(GL_ARRAY_BUFFER, maxSplashes * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

SplashSystem::~SplashSystem() {
    glDeleteVertexArrays(1, &splashVAO);
    glDeleteBuffers(1, &splashVBO);
    glDeleteBuffers(1, &sizeVBO);
}

float SplashSystem::SampleTerrainHeight(float x, float z) const {
    if (terrainHeightmap.empty() || heightmapWidth <= 1 || heightmapHeight <= 1) {
        return 0.0f;
    }

    float nx = glm::clamp(x / terrainWidth, 0.0f, 1.0f);
    float nz = glm::clamp(z / terrainDepth, 0.0f, 1.0f);

    int ix = static_cast<int>(nx * (heightmapWidth - 1));
    int iz = static_cast<int>(nz * (heightmapHeight - 1));

    float h = terrainHeightmap[iz * heightmapWidth + ix];
    return h * 150.0f;
}

void SplashSystem::SpawnSplash(const glm::vec3& cameraPos) {
    // Spawn splashes on ground near camera
    float rx = (static_cast<float>(rand() % 200) - 100.0f);
    float rz = (static_cast<float>(rand() % 200) - 100.0f);

    float x = glm::clamp(cameraPos.x + rx, 0.0f, terrainWidth);
    float z = glm::clamp(cameraPos.z + rz, 0.0f, terrainDepth);
    float y = SampleTerrainHeight(x, z) + 0.08f;

    Splash splash;
    splash.position = glm::vec3(x, y, z);
    splash.lifetime = 0.0f;
    splash.maxLifetime = 0.2f + (static_cast<float>(rand() % 10) / 50.0f); // 0.2-0.4s
    splash.size = 0.5f + (static_cast<float>(rand() % 5) / 10.0f);

    if (static_cast<int>(splashes.size()) < maxSplashes) {
        splashes.push_back(splash);
    }
}

void SplashSystem::Update(float dt, bool rainEnabled, const glm::vec3& cameraPos) {
    if (!rainEnabled) {
        splashes.clear();
        positions.clear();
        sizes.clear();
        return;
    }

    // Spawn new splashes
    int spawnCount = 3 + (rand() % 5); // 3-7 splashes per frame when raining
    for (int i = 0; i < spawnCount; i++) {
        SpawnSplash(cameraPos);
    }

    // Update existing splashes
    for (auto it = splashes.begin(); it != splashes.end();) {
        it->lifetime += dt;
        if (it->lifetime >= it->maxLifetime) {
            it = splashes.erase(it);
        } else {
            ++it;
        }
    }

    UpdateGPU();
}

void SplashSystem::UpdateGPU() {
    positions.clear();
    sizes.clear();

    for (const auto& splash : splashes) {
        positions.push_back(splash.position);
        // Scale size based on lifetime (grow then shrink)
        float t = splash.lifetime / splash.maxLifetime;
        float scale = 1.0f - abs(t * 2.0f - 1.0f); // triangle wave: 0 -> 1 -> 0
        sizes.push_back(splash.size * scale);
    }

    glBindVertexArray(splashVAO);

    // Update position buffer
    glBindBuffer(GL_ARRAY_BUFFER, splashVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

    // Update size buffer
    glBindBuffer(GL_ARRAY_BUFFER, sizeVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizes.size() * sizeof(float), sizes.data());

    glBindVertexArray(0);
}

void SplashSystem::Render(const glm::mat4& projection, const glm::mat4& view, Shader& shader) {
    if (splashes.empty()) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDepthMask(GL_FALSE);

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);

    glBindVertexArray(splashVAO);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(splashes.size()));
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_BLEND);
}

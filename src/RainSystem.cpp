#include "RainSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdlib>
#include <cmath>

RainSystem::RainSystem(int maxDrops) : maxDrops(maxDrops) {
    glGenVertexArrays(1, &rainVAO);
    glGenBuffers(1, &rainVBO);

    glBindVertexArray(rainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferData(GL_ARRAY_BUFFER, maxDrops * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // Spawn drops around origin by default; will be re-anchored to camera on first Update
    InitDrops(glm::vec3(0.0f));
}

RainSystem::~RainSystem() {
    glDeleteVertexArrays(1, &rainVAO);
    glDeleteBuffers(1, &rainVBO);
}

void RainSystem::InitDrops(const glm::vec3& cameraPos) {
    positions.clear();
    velocities.clear();
    positions.reserve(maxDrops);
    velocities.reserve(maxDrops);

    srand(7331);
    for (int i = 0; i < maxDrops; i++) {
        float rx = (rand() % 300) - 150.0f;
        float ry = (rand() % 80);          // height above camera
        float rz = (rand() % 300) - 150.0f;
        positions.push_back(glm::vec3(cameraPos.x + rx,
                                      cameraPos.y + ry + 20.0f,
                                      cameraPos.z + rz));
        // Rain falls fast downward, slight horizontal drift
        velocities.push_back(glm::vec3(
            (rand() % 11 - 5) * 0.05f,   // slight wind drift X
            -(20.0f + (rand() % 15)),      // fall speed: -20 to -35 units/s
            (rand() % 11 - 5) * 0.05f
        ));
    }
}

void RainSystem::Update(float deltaTime, const glm::vec3& cameraPos) {
    if (!enabled) {
        wasEnabled = false;  // reset so next enable triggers re-init
        return;
    }

    // First frame after enabling: move all drops to the camera vicinity immediately
    if (!wasEnabled) {
        InitDrops(cameraPos);
        wasEnabled = true;
    }

    for (int i = 0; i < maxDrops; i++) {
        positions[i] += velocities[i] * deltaTime;

        // Respawn drop near the camera when it goes below ground level
        if (positions[i].y < -5.0f) {
            float rx = (rand() % 300) - 150.0f;
            float rz = (rand() % 300) - 150.0f;
            positions[i] = glm::vec3(cameraPos.x + rx,
                                     cameraPos.y + 70.0f + (rand() % 20),
                                     cameraPos.z + rz);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, rainVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, maxDrops * sizeof(glm::vec3), positions.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void RainSystem::Render(const glm::mat4& projection, const glm::mat4& view, Shader& shader) {
    if (!enabled) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glDepthMask(GL_FALSE); // Don't write to depth to avoid z-fighting

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view",       view);

    glBindVertexArray(rainVAO);
    glDrawArrays(GL_POINTS, 0, maxDrops);
    glBindVertexArray(0);

    glDepthMask(GL_TRUE);
    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_BLEND);
}

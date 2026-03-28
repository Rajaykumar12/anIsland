#include "ParticleSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace {
const float FOREST_MIN_HEIGHT = 8.0f;
const float FOREST_MAX_HEIGHT = 85.0f;
const int MAX_SPAWN_RETRIES = 32;
const float BASE_HOVER = 8.0f;
}

ParticleSystem::ParticleSystem(int maxParticles,
                                                             const std::vector<float>& heightmap,
                                                             int hmWidth,
                                                             int hmHeight,
                                                             float terrainWidth,
                                                             float terrainDepth)
        : maxParticles(maxParticles),
            terrainHeightmap(heightmap),
            heightmapWidth(hmWidth),
            heightmapHeight(hmHeight),
            terrainWidth(terrainWidth),
            terrainDepth(terrainDepth) {
    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);

    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(glm::vec3), nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    InitializeParticles();
}

ParticleSystem::~ParticleSystem() {
    glDeleteVertexArrays(1, &particleVAO);
    glDeleteBuffers(1, &particleVBO);
}

float ParticleSystem::SampleTerrainHeight(float x, float z) const {
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

void ParticleSystem::InitializeParticles() {
    particlePositions.clear();
    particleVelocities.clear();
    particlePositions.reserve(maxParticles);
    particleVelocities.reserve(maxParticles);

    srand(42);
    for (int i = 0; i < maxParticles; i++) {
        float x = 0.0f;
        float z = 0.0f;
        float terrainY = 0.0f;

        bool foundForestSpot = false;
        for (int attempt = 0; attempt < MAX_SPAWN_RETRIES; ++attempt) {
            x = static_cast<float>(rand()) / RAND_MAX * terrainWidth;
            z = static_cast<float>(rand()) / RAND_MAX * terrainDepth;
            terrainY = SampleTerrainHeight(x, z);

            if (terrainY >= FOREST_MIN_HEIGHT && terrainY <= FOREST_MAX_HEIGHT) {
                foundForestSpot = true;
                break;
            }
        }

        if (!foundForestSpot) {
            x = static_cast<float>(rand()) / RAND_MAX * terrainWidth;
            z = static_cast<float>(rand()) / RAND_MAX * terrainDepth;
            terrainY = glm::clamp(SampleTerrainHeight(x, z), FOREST_MIN_HEIGHT, FOREST_MAX_HEIGHT);
        }

        float hover = 5.0f + static_cast<float>(rand() % 6); // 5..10 units above terrain

        glm::vec3 pos(x, terrainY + hover, z);
        glm::vec3 vel(
            (rand() % 11 - 5) * 0.05f,  // vx: gentle drift
            0.0f,                        // vy: mostly horizontal
            (rand() % 11 - 5) * 0.05f   // vz: gentle drift
        );
        particlePositions.push_back(pos);
        particleVelocities.push_back(vel);
    }

    UpdateGPU();
}

void ParticleSystem::UpdateGPU() {
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, particlePositions.size() * sizeof(glm::vec3),
                    particlePositions.data());
}

void ParticleSystem::Update(float deltaTime) {
    for (size_t i = 0; i < particlePositions.size(); i++) {
        particlePositions[i] += particleVelocities[i] * deltaTime;

        // Keep particles inside terrain footprint.
        if (particlePositions[i].x < 0.0f) particlePositions[i].x += terrainWidth;
        if (particlePositions[i].x > terrainWidth) particlePositions[i].x -= terrainWidth;
        if (particlePositions[i].z < 0.0f) particlePositions[i].z += terrainDepth;
        if (particlePositions[i].z > terrainDepth) particlePositions[i].z -= terrainDepth;

        // Hover above local terrain with subtle flutter.
        float terrainY = SampleTerrainHeight(particlePositions[i].x, particlePositions[i].z);

        if (terrainY < FOREST_MIN_HEIGHT || terrainY > FOREST_MAX_HEIGHT) {
            bool relocated = false;
            for (int attempt = 0; attempt < MAX_SPAWN_RETRIES; ++attempt) {
                float x = static_cast<float>(rand()) / RAND_MAX * terrainWidth;
                float z = static_cast<float>(rand()) / RAND_MAX * terrainDepth;
                float y = SampleTerrainHeight(x, z);

                if (y >= FOREST_MIN_HEIGHT && y <= FOREST_MAX_HEIGHT) {
                    particlePositions[i].x = x;
                    particlePositions[i].z = z;
                    particlePositions[i].y = y + BASE_HOVER;
                    relocated = true;
                    break;
                }
            }

            if (!relocated) {
                particlePositions[i].y = glm::clamp(terrainY, FOREST_MIN_HEIGHT, FOREST_MAX_HEIGHT) + BASE_HOVER;
            }
        } else {
            float flutter = 0.8f * std::sin(0.35f * particlePositions[i].x + 0.27f * particlePositions[i].z);
            particlePositions[i].y = terrainY + BASE_HOVER + flutter;
        }

        // Add slight jitter to velocity for fluttering effect
        particleVelocities[i].x += (rand() % 11 - 5) * 0.001f;
        particleVelocities[i].z += (rand() % 11 - 5) * 0.001f;
        particleVelocities[i].x = glm::clamp(particleVelocities[i].x, -0.5f, 0.5f);
        particleVelocities[i].z = glm::clamp(particleVelocities[i].z, -0.5f, 0.5f);
    }

    UpdateGPU();
}

void ParticleSystem::Render(const glm::mat4& projection, const glm::mat4& view, Shader& shader) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // Additive blending for glow effect
    glEnable(GL_PROGRAM_POINT_SIZE);

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view",       view);

    glBindVertexArray(particleVAO);
    glDrawArrays(GL_POINTS, 0, particlePositions.size());
    glBindVertexArray(0);

    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_BLEND);
}

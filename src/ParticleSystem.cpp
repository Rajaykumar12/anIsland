#include "ParticleSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdlib>

ParticleSystem::ParticleSystem(int maxParticles) : maxParticles(maxParticles) {
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

void ParticleSystem::InitializeParticles() {
    particlePositions.clear();
    particleVelocities.clear();
    particlePositions.reserve(maxParticles);
    particleVelocities.reserve(maxParticles);

    srand(42);
    for (int i = 0; i < maxParticles; i++) {
        glm::vec3 pos(
            50.0f  + (rand() % 200) - 100.0f,  // x: spread around forest (0-200)
            2.0f   + (rand() % 13),             // y: 2-15 units (near tree-top height)
            100.0f + (rand() % 200) - 100.0f   // z: spread around forest (0-200)
        );
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

        // Gentle up/down bobbing using sin
        particlePositions[i].y += 0.3f * std::sin(particlePositions[i].x * 0.7f + particlePositions[i].z * 0.5f) * 0.02f;
        // Clamp altitude to realistic firefly range
        if (particlePositions[i].y > 16.0f) particlePositions[i].y = 16.0f;
        if (particlePositions[i].y < 1.5f)  particlePositions[i].y = 1.5f;

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

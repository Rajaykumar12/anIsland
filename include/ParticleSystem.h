#pragma once

#include <glm/glm.hpp>
#include <vector>

class Shader;

struct Particle {
    glm::vec3 Position;
    glm::vec3 Velocity;
    float Life;
};

class ParticleSystem {
public:
    ParticleSystem(int maxParticles = 500);
    ~ParticleSystem();

    void Update(float deltaTime);
    void Render(const glm::mat4& projection, const glm::mat4& view, Shader& shader);

    int GetParticleCount() const { return particlePositions.size(); }

private:
    unsigned int particleVAO, particleVBO;
    std::vector<glm::vec3> particlePositions;
    std::vector<glm::vec3> particleVelocities;
    int maxParticles;

    void InitializeParticles();
    void UpdateGPU();
};

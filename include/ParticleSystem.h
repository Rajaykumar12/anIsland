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
    ParticleSystem(int maxParticles = 500,
                   const std::vector<float>& heightmap = {},
                   int hmWidth = 0,
                   int hmHeight = 0,
                   float terrainWidth = 800.0f,
                   float terrainDepth = 800.0f);
    ~ParticleSystem();

    void Update(float deltaTime);
    void Render(const glm::mat4& projection, const glm::mat4& view, Shader& shader);

    int GetParticleCount() const { return particlePositions.size(); }

private:
    unsigned int particleVAO, particleVBO;
    std::vector<glm::vec3> particlePositions;
    std::vector<glm::vec3> particleVelocities;
    int maxParticles;
    std::vector<float> terrainHeightmap;
    int heightmapWidth;
    int heightmapHeight;
    float terrainWidth;
    float terrainDepth;

    void InitializeParticles();
    float SampleTerrainHeight(float x, float z) const;
    void UpdateGPU();
};

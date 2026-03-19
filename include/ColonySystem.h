#pragma once

#include <glm/glm.hpp>
#include <vector>

class Shader;

class ColonySystem {
public:
    ColonySystem(const std::vector<float>& noiseData, int hmWidth, int hmHeight,
                 int terrainWidth, int terrainHeight);
    ~ColonySystem();

    // Sample terrain height at (x, z) using bilinear interpolation
    float SampleHeight(float x, float z) const;

    // Update villager positions (patrol paths)
    void Update(float currentTime);

    // Render huts and villagers
    void Render(const glm::mat4& projection, const glm::mat4& view,
                const glm::vec3& skyColor, Shader& hutShader, Shader& villagerShader,
                float currentTime);

    int GetHutVertexCount() const { return hutVertexCount; }
    int GetHutInstanceCount() const { return (int)hutTranslations.size(); }
    int GetVillagerVertexCount() const { return villagerVertexCount; }
    int GetVillagerInstanceCount() const { return (int)villagerTranslations.size(); }

private:
    // Reference to noise data for height sampling
    const std::vector<float>& noiseData;
    int hmWidth, hmHeight;
    int terrainWidth, terrainHeight;

    // Hut geometry
    unsigned int hutVAO, hutVBO, hutInstanceVBO;
    int hutVertexCount;
    std::vector<glm::vec3> hutTranslations;

    // Villager geometry
    unsigned int villagerVAO, villagerVBO, villagerInstanceVBO;
    int villagerVertexCount;
    std::vector<glm::vec3> villagerTranslations;
    std::vector<glm::vec3> villagerBases;
    std::vector<float> villagerPatrolRadius;
    std::vector<float> villagerPatrolPhase;

    void SetupHutGeometry();
    void SetupHutInstances();
    void SetupVillagerGeometry();
    void SetupVillagerInstances();
};

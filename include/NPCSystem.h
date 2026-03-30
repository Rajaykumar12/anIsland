#pragma once

#include <glm/glm.hpp>
#include <vector>

class Shader;

class NPCSystem {
public:
    NPCSystem();
    ~NPCSystem();

    void Update(float currentTime);  // Moves NPCs along patrol paths
    void Render(const glm::mat4& projection, const glm::mat4& view,
                const glm::vec3& skyColor, Shader& shader, float currentTime);

    unsigned int GetVAO() const { return personVAO; }
    int GetVertexCount()   const { return personVertexCount; }
    int GetInstanceCount() const { return (int)personTranslations.size(); }

private:
    unsigned int personVAO, personVBO, personInstanceVBO;
    int personVertexCount;
    std::vector<glm::vec3> personTranslations;
    // Base positions for patrol orbit (town center)
    std::vector<glm::vec3> personBases;
    // Patrol radii and speed offsets per NPC
    std::vector<float> patrolRadius;
    std::vector<float> patrolPhase;

    void SetupGeometry();
    void SetupInstances();
};

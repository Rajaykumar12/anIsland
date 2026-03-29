#pragma once

#include <glm/glm.hpp>
#include <vector>

class Shader;

enum class NPCPoseType {
    PRONE,
    SITTING,
    STANDING,
    WALKING
};

struct NPCInstance {
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 basePos;
    NPCPoseType pose;
    bool isCinematic;
    float patrolRadius;
    float patrolPhase;
    float visibleStart;
    float visibleEnd;
    bool isVisible;
};

class NPCSystem {
public:
    NPCSystem();
    ~NPCSystem();

    void Update(float currentTime);  // Moves NPCs along patrol paths
    void Render(const glm::mat4& projection, const glm::mat4& view,
                const glm::vec3& skyColor, Shader& shader, float currentTime);
    void AddCinematicNPC(const glm::vec3& pos, const glm::vec3& rot, NPCPoseType pose,
                         float visibleStart, float visibleEnd);
    void ClearCinematicNPCs();

    int GetVertexCount()   const { return personVertexCount; }
    int GetInstanceCount() const { return (int)npcs.size(); }

private:
    unsigned int personVAO, personVBO, personInstanceVBO;
    int personVertexCount;
    std::vector<NPCInstance> npcs;

    void SetupGeometry();
    void SetupInstances();
    void UpdateInstanceBuffer();
};

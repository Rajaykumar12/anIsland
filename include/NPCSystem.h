#pragma once

#include <glm/glm.hpp>
#include <vector>

class Shader;

class NPCSystem {
public:
    NPCSystem();
    ~NPCSystem();

    void Render(const glm::mat4& projection, const glm::mat4& view,
                const glm::vec3& skyColor, Shader& shader, float currentTime);

    int GetVertexCount() const { return personVertexCount; }
    int GetInstanceCount() const { return personTranslations.size(); }

private:
    unsigned int personVAO, personVBO, personInstanceVBO;
    int personVertexCount;
    std::vector<glm::vec3> personTranslations;

    void SetupGeometry();
    void SetupInstances();
};

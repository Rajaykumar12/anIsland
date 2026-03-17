#pragma once

#include <glm/glm.hpp>
#include <vector>

class Shader;

class BuildingSystem {
public:
    BuildingSystem();
    ~BuildingSystem();

    void Render(const glm::mat4& projection, const glm::mat4& view,
                const glm::vec3& skyColor, Shader& shader);

    int GetVertexCount() const { return buildingVertexCount; }
    int GetInstanceCount() const { return buildingTranslations.size(); }

private:
    unsigned int buildingVAO, buildingVBO, buildingInstanceVBO;
    int buildingVertexCount;
    std::vector<glm::vec3> buildingTranslations;

    void SetupGeometry();
    void SetupInstances();
};

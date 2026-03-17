#pragma once

#include <glm/glm.hpp>
#include <vector>

class Shader;

class WaterSystem {
public:
    WaterSystem(int width = 500, int depth = 500);
    ~WaterSystem();

    void Render(const glm::mat4& projection, const glm::mat4& view,
                Shader& shader, float currentTime, const glm::vec3& lightColor);

    int GetVertexCount() const { return waterIndices.size(); }

private:
    unsigned int waterVAO, waterVBO, waterEBO;
    std::vector<glm::vec3> waterVertices;
    std::vector<unsigned int> waterIndices;
    int waterWidth, waterDepth;

    void GenerateMesh(int width, int depth);
};

#include "WaterSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

WaterSystem::WaterSystem(int width, int depth) : waterWidth(width), waterDepth(depth) {
    GenerateMesh(width, depth);
    
    glGenVertexArrays(1, &waterVAO);
    glGenBuffers(1, &waterVBO);
    glGenBuffers(1, &waterEBO);

    glBindVertexArray(waterVAO);
    glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
    glBufferData(GL_ARRAY_BUFFER, waterVertices.size() * sizeof(glm::vec3),
                 waterVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, waterIndices.size() * sizeof(unsigned int),
                 waterIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

WaterSystem::~WaterSystem() {
    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
    glDeleteBuffers(1, &waterEBO);
}

void WaterSystem::GenerateMesh(int width, int depth) {
    waterVertices.clear();
    waterIndices.clear();

    float waterSpacing = 2.0f;  // Keep coarser for performance
    const float WATER_EXTENT = 2000.0f;  // 5x larger ocean area
    const float WATER_LEVEL = -1.5f;  // Better shoreline contact after edge submerge
    
    for (int z = 0; z < depth; z++) {
        for (int x = 0; x < width; x++) {
            float xPos = -WATER_EXTENT + x * waterSpacing;
            float zPos = -WATER_EXTENT + z * waterSpacing;
            waterVertices.push_back(glm::vec3(xPos, WATER_LEVEL, zPos));
        }
    }

    for (int z = 0; z < depth - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            // Triangle 1
            waterIndices.push_back(z * width + x);
            waterIndices.push_back((z + 1) * width + x);
            waterIndices.push_back(z * width + (x + 1));
            // Triangle 2
            waterIndices.push_back((z + 1) * width + x);
            waterIndices.push_back((z + 1) * width + (x + 1));
            waterIndices.push_back(z * width + (x + 1));
        }
    }
}

void WaterSystem::Render(const glm::mat4& projection, const glm::mat4& view,
                         Shader& shader, float currentTime, const glm::vec3& lightColor) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);

    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view",       view);
    shader.setMat4("model",      glm::mat4(1.0f));
    shader.setFloat("u_Time",    currentTime);
    shader.setVec3("u_LightColor", lightColor);

    glBindVertexArray(waterVAO);
    glDrawElements(GL_TRIANGLES, waterIndices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_CULL_FACE);
}

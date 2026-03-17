#include "BuildingSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>

BuildingSystem::BuildingSystem() {
    SetupGeometry();
    SetupInstances();
}

BuildingSystem::~BuildingSystem() {
    glDeleteVertexArrays(1, &buildingVAO);
    glDeleteBuffers(1, &buildingVBO);
    glDeleteBuffers(1, &buildingInstanceVBO);
}

void BuildingSystem::SetupGeometry() {
    float buildingVertices[] = {
        // Front face - 2 triangles
        -1.5f, 0.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.6f, 0.4f, 0.2f,
         1.5f, 0.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.6f, 0.4f, 0.2f,
         1.5f, 3.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.65f, 0.42f, 0.22f,
        -1.5f, 0.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.6f, 0.4f, 0.2f,
         1.5f, 3.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.65f, 0.42f, 0.22f,
        -1.5f, 3.0f, -1.5f,   0.0f, 0.0f, -1.0f,   0.65f, 0.42f, 0.22f,
        // Back face - 2 triangles
         1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.6f, 0.4f, 0.2f,
        -1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.6f, 0.4f, 0.2f,
        -1.5f, 3.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.65f, 0.42f, 0.22f,
         1.5f, 0.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.6f, 0.4f, 0.2f,
        -1.5f, 3.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.65f, 0.42f, 0.22f,
         1.5f, 3.0f, 1.5f,    0.0f, 0.0f, 1.0f,    0.65f, 0.42f, 0.22f,
        // Right face - 2 triangles
         1.5f, 0.0f, -1.5f,   1.0f, 0.0f, 0.0f,    0.58f, 0.38f, 0.18f,
         1.5f, 0.0f, 1.5f,    1.0f, 0.0f, 0.0f,    0.58f, 0.38f, 0.18f,
         1.5f, 3.0f, 1.5f,    1.0f, 0.0f, 0.0f,    0.63f, 0.41f, 0.21f,
         1.5f, 0.0f, -1.5f,   1.0f, 0.0f, 0.0f,    0.58f, 0.38f, 0.18f,
         1.5f, 3.0f, 1.5f,    1.0f, 0.0f, 0.0f,    0.63f, 0.41f, 0.21f,
         1.5f, 3.0f, -1.5f,   1.0f, 0.0f, 0.0f,    0.63f, 0.41f, 0.21f,
        // Left face - 2 triangles
        -1.5f, 0.0f, 1.5f,    -1.0f, 0.0f, 0.0f,   0.58f, 0.38f, 0.18f,
        -1.5f, 0.0f, -1.5f,   -1.0f, 0.0f, 0.0f,   0.58f, 0.38f, 0.18f,
        -1.5f, 3.0f, -1.5f,   -1.0f, 0.0f, 0.0f,   0.63f, 0.41f, 0.21f,
        -1.5f, 0.0f, 1.5f,    -1.0f, 0.0f, 0.0f,   0.58f, 0.38f, 0.18f,
        -1.5f, 3.0f, -1.5f,   -1.0f, 0.0f, 0.0f,   0.63f, 0.41f, 0.21f,
        -1.5f, 3.0f, 1.5f,    -1.0f, 0.0f, 0.0f,   0.63f, 0.41f, 0.21f,
        // Roof front - single triangle
        -1.5f, 3.0f, -1.5f,   0.0f, 0.7f, -0.7f,   0.8f, 0.2f, 0.2f,
         1.5f, 3.0f, -1.5f,   0.0f, 0.7f, -0.7f,   0.8f, 0.2f, 0.2f,
         0.0f, 4.5f, 0.0f,    0.0f, 0.7f, -0.7f,   0.85f, 0.22f, 0.22f,
        // Roof back - single triangle
         1.5f, 3.0f, 1.5f,    0.0f, 0.7f, 0.7f,    0.75f, 0.18f, 0.18f,
        -1.5f, 3.0f, 1.5f,    0.0f, 0.7f, 0.7f,    0.75f, 0.18f, 0.18f,
         0.0f, 4.5f, 0.0f,    0.0f, 0.7f, 0.7f,    0.8f, 0.2f, 0.2f,
        // Windows (light blue) - 2 triangles
        -0.5f, 1.5f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
         0.5f, 1.5f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
         0.5f, 2.0f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
        -0.5f, 1.5f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
         0.5f, 2.0f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
        -0.5f, 2.0f, -1.51f,  0.0f, 0.0f, -1.0f,   0.3f, 0.7f, 1.0f,
    };
    buildingVertexCount = sizeof(buildingVertices) / (9 * sizeof(float));

    glGenVertexArrays(1, &buildingVAO);
    glGenBuffers(1, &buildingVBO);
    glGenBuffers(1, &buildingInstanceVBO);

    glBindVertexArray(buildingVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buildingVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buildingVertices), buildingVertices, GL_STATIC_DRAW);

    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Color (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void BuildingSystem::SetupInstances() {
    buildingTranslations.clear();
    
    // Town layout - flat area at (100-150, z=150-200)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            float x = 100.0f + i * 7.0f;
            float z = 150.0f + j * 7.0f;
            float townTerrainHeight = 10.0f; // Town is on relatively flat terrain
            buildingTranslations.push_back(glm::vec3(x, townTerrainHeight, z));
        }
    }

    glBindVertexArray(buildingVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buildingInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, buildingTranslations.size() * sizeof(glm::vec3),
                 buildingTranslations.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

void BuildingSystem::Render(const glm::mat4& projection, const glm::mat4& view,
                            const glm::vec3& skyColor, Shader& shader) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view",       view);
    shader.setVec3("skyColor",   skyColor);

    glBindVertexArray(buildingVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, buildingVertexCount, buildingTranslations.size());
    glBindVertexArray(0);
}

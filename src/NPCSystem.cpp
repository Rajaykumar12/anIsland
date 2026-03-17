#include "NPCSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdlib>

NPCSystem::NPCSystem() {
    SetupGeometry();
    SetupInstances();
}

NPCSystem::~NPCSystem() {
    glDeleteVertexArrays(1, &personVAO);
    glDeleteBuffers(1, &personVBO);
    glDeleteBuffers(1, &personInstanceVBO);
}

void NPCSystem::SetupGeometry() {
    float personVertices[] = {
        // Head (skin color) - 2 triangles
        -0.2f, 1.8f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
         0.2f, 1.8f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
         0.2f, 2.2f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
        -0.2f, 1.8f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
         0.2f, 2.2f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
        -0.2f, 2.2f, -0.2f,   0.0f, 0.0f, -1.0f,   1.0f, 0.8f, 0.6f,
        // Body (red shirt) - 2 triangles
        -0.35f, 1.0f, -0.25f, 0.0f, 0.0f, -1.0f,   0.8f, 0.2f, 0.2f,
         0.35f, 1.0f, -0.25f, 0.0f, 0.0f, -1.0f,   0.8f, 0.2f, 0.2f,
         0.35f, 1.8f, -0.25f, 0.0f, 0.0f, -1.0f,   0.9f, 0.25f, 0.25f,
        -0.35f, 1.0f, -0.25f, 0.0f, 0.0f, -1.0f,   0.8f, 0.2f, 0.2f,
         0.35f, 1.8f, -0.25f, 0.0f, 0.0f, -1.0f,   0.9f, 0.25f, 0.25f,
        -0.35f, 1.8f, -0.25f, 0.0f, 0.0f, -1.0f,   0.9f, 0.25f, 0.25f,
        // Left arm (blue) - 2 triangles
        -0.5f, 1.4f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.2f, 0.3f, 0.8f,
        -0.4f, 1.4f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.2f, 0.3f, 0.8f,
        -0.4f, 1.0f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.25f, 0.35f, 0.85f,
        -0.5f, 1.4f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.2f, 0.3f, 0.8f,
        -0.4f, 1.0f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.25f, 0.35f, 0.85f,
        -0.5f, 1.0f, -0.15f,  -1.0f, 0.0f, 0.0f,   0.25f, 0.35f, 0.85f,
        // Right arm (blue) - 2 triangles
         0.4f, 1.4f, -0.15f,  1.0f, 0.0f, 0.0f,    0.2f, 0.3f, 0.8f,
         0.5f, 1.4f, -0.15f,  1.0f, 0.0f, 0.0f,    0.2f, 0.3f, 0.8f,
         0.5f, 1.0f, -0.15f,  1.0f, 0.0f, 0.0f,    0.25f, 0.35f, 0.85f,
         0.4f, 1.4f, -0.15f,  1.0f, 0.0f, 0.0f,    0.2f, 0.3f, 0.8f,
         0.5f, 1.0f, -0.15f,  1.0f, 0.0f, 0.0f,    0.25f, 0.35f, 0.85f,
         0.4f, 1.0f, -0.15f,  1.0f, 0.0f, 0.0f,    0.25f, 0.35f, 0.85f,
        // Legs (brown pants) - 2 triangles
        -0.2f, 0.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.4f, 0.3f, 0.2f,
         0.2f, 0.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.4f, 0.3f, 0.2f,
         0.2f, 1.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.5f, 0.4f, 0.25f,
        -0.2f, 0.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.4f, 0.3f, 0.2f,
         0.2f, 1.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.5f, 0.4f, 0.25f,
        -0.2f, 1.0f, -0.15f,  0.0f, 0.0f, -1.0f,   0.5f, 0.4f, 0.25f,
    };
    personVertexCount = sizeof(personVertices) / (9 * sizeof(float));

    glGenVertexArrays(1, &personVAO);
    glGenBuffers(1, &personVBO);
    glGenBuffers(1, &personInstanceVBO);

    glBindVertexArray(personVAO);
    glBindBuffer(GL_ARRAY_BUFFER, personVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(personVertices), personVertices, GL_STATIC_DRAW);

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

void NPCSystem::SetupInstances() {
    personTranslations.clear();
    
    srand(99); // Different seed for people
    for (int i = 0; i < 30; i++) {
        float x = 100.0f + (rand() % 50) * 0.5f;
        float z = 150.0f + (rand() % 50) * 0.5f;
        personTranslations.push_back(glm::vec3(x, 10.0f, z));
    }

    glBindVertexArray(personVAO);
    glBindBuffer(GL_ARRAY_BUFFER, personInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, personTranslations.size() * sizeof(glm::vec3),
                 personTranslations.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

void NPCSystem::Render(const glm::mat4& projection, const glm::mat4& view,
                       const glm::vec3& skyColor, Shader& shader, float currentTime) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view",       view);
    shader.setVec3("skyColor",   skyColor);
    shader.setFloat("u_Time",    currentTime);

    glBindVertexArray(personVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, personVertexCount, personTranslations.size());
    glBindVertexArray(0);
}

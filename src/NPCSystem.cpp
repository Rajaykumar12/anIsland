#include "NPCSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>

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
    personBases.clear();
    patrolRadius.clear();
    patrolPhase.clear();

    srand(99);
    for (int i = 0; i < 30; i++) {
        // Base position inside town
        float bx = 110.0f + (rand() % 30);
        float bz = 155.0f + (rand() % 40);
        personBases.push_back(glm::vec3(bx, 10.0f, bz));
        personTranslations.push_back(glm::vec3(bx, 10.0f, bz));
        patrolRadius.push_back(5.0f + (rand() % 10));   // orbit radius 5-15
        patrolPhase.push_back((float)(rand() % 628) / 100.0f); // 0-2pi
    }

    glBindVertexArray(personVAO);
    glBindBuffer(GL_ARRAY_BUFFER, personInstanceVBO);
    // Use GL_DYNAMIC_DRAW because we update positions every frame
    glBufferData(GL_ARRAY_BUFFER, personTranslations.size() * sizeof(glm::vec3),
                 personTranslations.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

void NPCSystem::Update(float currentTime) {
    for (int i = 0; i < (int)personTranslations.size(); i++) {
        float angle = currentTime * 0.25f + patrolPhase[i];
        float r = patrolRadius[i];
        personTranslations[i].x = personBases[i].x + r * std::cos(angle);
        personTranslations[i].z = personBases[i].z + r * std::sin(angle);
        // Y stays at ground level
        personTranslations[i].y = personBases[i].y;
    }

    // Upload updated positions to GPU
    glBindBuffer(GL_ARRAY_BUFFER, personInstanceVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    personTranslations.size() * sizeof(glm::vec3),
                    personTranslations.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void NPCSystem::Render(const glm::mat4& projection, const glm::mat4& view,
                       const glm::vec3& skyColor, Shader& shader, float currentTime) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view",       view);
    shader.setVec3("skyColor",   skyColor);
    shader.setFloat("u_Time",    currentTime);

    glBindVertexArray(personVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, personVertexCount, (int)personTranslations.size());
    glBindVertexArray(0);
}

#include "NPCSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>
#include <vector>

// Helper: Generate 3D cylinder geometry with proper normals
void GenerateCylinderGeometry(std::vector<float>& verts, 
                             float x, float y, float z,
                             float radius, float height,
                             int sides, glm::vec3 color) {
    const float PI = 3.14159265f;
    
    // Generate cylinder sides
    for (int i = 0; i < sides; i++) {
        float angle1 = (2.0f * PI * i) / sides;
        float angle2 = (2.0f * PI * (i + 1)) / sides;
        
        float x1 = radius * cos(angle1);
        float z1 = radius * sin(angle1);
        float x2 = radius * cos(angle2);
        float z2 = radius * sin(angle2);
        
        // Normal vectors pointing outward
        float nx1 = cos(angle1);
        float nz1 = sin(angle1);
        float nx2 = cos(angle2);
        float nz2 = sin(angle2);
        
        // Bottom-left vertex
        verts.push_back(x + x1); verts.push_back(y); verts.push_back(z + z1);
        verts.push_back(nx1); verts.push_back(0.0f); verts.push_back(nz1);
        verts.push_back(color.x); verts.push_back(color.y); verts.push_back(color.z);
        
        // Bottom-right vertex
        verts.push_back(x + x2); verts.push_back(y); verts.push_back(z + z2);
        verts.push_back(nx2); verts.push_back(0.0f); verts.push_back(nz2);
        verts.push_back(color.x); verts.push_back(color.y); verts.push_back(color.z);
        
        // Top-right vertex
        verts.push_back(x + x2); verts.push_back(y + height); verts.push_back(z + z2);
        verts.push_back(nx2); verts.push_back(0.0f); verts.push_back(nz2);
        verts.push_back(color.x); verts.push_back(color.y); verts.push_back(color.z);
        
        // Top-left vertex
        verts.push_back(x + x1); verts.push_back(y + height); verts.push_back(z + z1);
        verts.push_back(nx1); verts.push_back(0.0f); verts.push_back(nz1);
        verts.push_back(color.x); verts.push_back(color.y); verts.push_back(color.z);
    }
    
    // Add bottom cap
    for (int i = 0; i < sides; i++) {
        float angle1 = (2.0f * PI * i) / sides;
        float angle2 = (2.0f * PI * (i + 1)) / sides;
        
        float x1 = radius * cos(angle1);
        float z1 = radius * sin(angle1);
        float x2 = radius * cos(angle2);
        float z2 = radius * sin(angle2);
        
        // Center of bottom
        verts.push_back(x); verts.push_back(y); verts.push_back(z);
        verts.push_back(0.0f); verts.push_back(-1.0f); verts.push_back(0.0f);
        verts.push_back(color.x * 0.8f); verts.push_back(color.y * 0.8f); verts.push_back(color.z * 0.8f);
        
        // Edge 2
        verts.push_back(x + x2); verts.push_back(y); verts.push_back(z + z2);
        verts.push_back(0.0f); verts.push_back(-1.0f); verts.push_back(0.0f);
        verts.push_back(color.x * 0.8f); verts.push_back(color.y * 0.8f); verts.push_back(color.z * 0.8f);
        
        // Edge 1
        verts.push_back(x + x1); verts.push_back(y); verts.push_back(z + z1);
        verts.push_back(0.0f); verts.push_back(-1.0f); verts.push_back(0.0f);
        verts.push_back(color.x * 0.8f); verts.push_back(color.y * 0.8f); verts.push_back(color.z * 0.8f);
    }
    
    // Add top cap
    for (int i = 0; i < sides; i++) {
        float angle1 = (2.0f * PI * i) / sides;
        float angle2 = (2.0f * PI * (i + 1)) / sides;
        
        float x1 = radius * cos(angle1);
        float z1 = radius * sin(angle1);
        float x2 = radius * cos(angle2);
        float z2 = radius * sin(angle2);
        
        // Center of top
        verts.push_back(x); verts.push_back(y + height); verts.push_back(z);
        verts.push_back(0.0f); verts.push_back(1.0f); verts.push_back(0.0f);
        verts.push_back(color.x * 0.9f); verts.push_back(color.y * 0.9f); verts.push_back(color.z * 0.9f);
        
        // Edge 1
        verts.push_back(x + x1); verts.push_back(y + height); verts.push_back(z + z1);
        verts.push_back(0.0f); verts.push_back(1.0f); verts.push_back(0.0f);
        verts.push_back(color.x * 0.9f); verts.push_back(color.y * 0.9f); verts.push_back(color.z * 0.9f);
        
        // Edge 2
        verts.push_back(x + x2); verts.push_back(y + height); verts.push_back(z + z2);
        verts.push_back(0.0f); verts.push_back(1.0f); verts.push_back(0.0f);
        verts.push_back(color.x * 0.9f); verts.push_back(color.y * 0.9f); verts.push_back(color.z * 0.9f);
    }
}

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
    std::vector<float> personVertices;
    
    // Create 3D cylindrical NPCs for much better visual quality
    glm::vec3 skinColor(1.0f, 0.8f, 0.6f);      // Skin
    glm::vec3 shirtColor(0.85f, 0.2f, 0.2f);    // Red shirt
    glm::vec3 armColor(0.2f, 0.3f, 0.8f);       // Blue arms
    glm::vec3 pantColor(0.4f, 0.3f, 0.2f);      // Brown pants
    
    // Head - smooth cylinder with 12 sides (sphere-like)
    GenerateCylinderGeometry(personVertices, 0.0f, 1.8f, 0.0f, 0.25f, 0.4f, 12, skinColor);
    
    // Body - main cylinder with 10 sides
    GenerateCylinderGeometry(personVertices, 0.0f, 1.0f, 0.0f, 0.35f, 0.8f, 10, shirtColor);
    
    // Left arm - thin cylinder with 8 sides
    GenerateCylinderGeometry(personVertices, -0.45f, 1.3f, 0.0f, 0.1f, 0.4f, 8, armColor);
    
    // Right arm - thin cylinder with 8 sides
    GenerateCylinderGeometry(personVertices, 0.45f, 1.3f, 0.0f, 0.1f, 0.4f, 8, armColor);
    
    // Legs - unified lower body cylinder
    GenerateCylinderGeometry(personVertices, 0.0f, 0.0f, 0.0f, 0.2f, 1.0f, 8, pantColor);
    
    personVertexCount = personVertices.size() / 9;

    glGenVertexArrays(1, &personVAO);
    glGenBuffers(1, &personVBO);
    glGenBuffers(1, &personInstanceVBO);

    glBindVertexArray(personVAO);
    glBindBuffer(GL_ARRAY_BUFFER, personVBO);
    glBufferData(GL_ARRAY_BUFFER, personVertices.size() * sizeof(float), personVertices.data(), GL_STATIC_DRAW);

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
        // Move NPCs far away - island remembers itself without inhabitants
        float bx = -1000.0f + (rand() % 40);
        float bz = -1000.0f + (rand() % 40);
        personBases.push_back(glm::vec3(bx, 110.0f, bz));
        personTranslations.push_back(glm::vec3(bx, 110.0f, bz));
        patrolRadius.push_back(5.0f + (rand() % 10));
        patrolPhase.push_back((float)(rand() % 628) / 100.0f);
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

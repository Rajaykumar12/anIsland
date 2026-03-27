#include "ColonySystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <cstdlib>

ColonySystem::ColonySystem(const std::vector<float>& noiseData, int hmWidth, int hmHeight,
                           int terrainWidth, int terrainHeight)
    : noiseData(noiseData), hmWidth(hmWidth), hmHeight(hmHeight),
      terrainWidth(terrainWidth), terrainHeight(terrainHeight) {
    // Disabled for "The Island Remembers" - no inhabitants
    // SetupHutGeometry();
    // SetupHutInstances();
    // SetupVillagerGeometry();
    // SetupVillagerInstances();
}

ColonySystem::~ColonySystem() {
    glDeleteVertexArrays(1, &hutVAO);
    glDeleteBuffers(1, &hutVBO);
    glDeleteBuffers(1, &hutInstanceVBO);
    glDeleteVertexArrays(1, &villagerVAO);
    glDeleteBuffers(1, &villagerVBO);
    glDeleteBuffers(1, &villagerInstanceVBO);
}

float ColonySystem::SampleHeight(float x, float z) const {
    // Map world coordinates to heightmap indices
    // Heightmap is 512x512, world terrain is 800x800
    float normX = x / terrainWidth;  // 0 to 1
    float normZ = z / terrainHeight; // 0 to 1
    
    float hmX = normX * hmWidth;
    float hmZ = normZ * hmHeight;
    
    // Clamp to valid range
    if (hmX < 0.0f) hmX = 0.0f; else if (hmX >= hmWidth - 1) hmX = hmWidth - 1.5f;
    if (hmZ < 0.0f) hmZ = 0.0f; else if (hmZ >= hmHeight - 1) hmZ = hmHeight - 1.5f;
    
    // Bilinear interpolation
    int x0 = (int)hmX;
    int z0 = (int)hmZ;
    int x1 = x0 + 1;
    int z1 = z0 + 1;
    
    float fx = hmX - x0;
    float fz = hmZ - z0;
    
    float h00 = noiseData[z0 * hmWidth + x0];
    float h10 = noiseData[z0 * hmWidth + x1];
    float h01 = noiseData[z1 * hmWidth + x0];
    float h11 = noiseData[z1 * hmWidth + x1];
    
    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;
    float height = h0 * (1.0f - fz) + h1 * fz;
    
    return height * 150.0f;
}

void ColonySystem::SetupHutGeometry() {
    // Create an 8-sided cylindrical hut with conical roof + doors, windows, details
    // Cylinder wall: 8 sides, height from 0 to 2.5, radius 1.8
    // Roof: conical top from 2.5 to 3.5
    // Features: Door, 4 windows, smoke hole
    
    std::vector<float> vertices;
    const int sides = 16;  // Increased from 8 for smoother cylinders
    const float wallHeight = 2.5f;
    const float roofHeight = 1.2f;
    const float radius = 1.8f;
    
    // Earthy tan/ochre colors for huts (mud walls)
    glm::vec3 wallColor(0.75f, 0.6f, 0.4f);       // tan
    glm::vec3 wallColorDark(0.65f, 0.5f, 0.3f);   // darker tan for shading
    glm::vec3 roofColor(0.45f, 0.3f, 0.1f);       // dark brown thatch
    glm::vec3 roofColorLight(0.6f, 0.45f, 0.2f);  // lighter straw
    glm::vec3 doorColor(0.3f, 0.15f, 0.05f);      // very dark brown door
    glm::vec3 windowColor(0.2f, 0.3f, 0.4f);      // dark blue window
    
    // Generate cylinder walls (8 sides) with door on side 0 and windows on sides 1,3,5,7
    for (int i = 0; i < sides; i++) {
        float angle1 = 2.0f * 3.14159f * i / sides;
        float angle2 = 2.0f * 3.14159f * ((i + 1) % sides) / sides;
        
        float x1_0 = radius * std::cos(angle1);
        float z1_0 = radius * std::sin(angle1);
        float x2_0 = radius * std::cos(angle2);
        float z2_0 = radius * std::sin(angle2);
        
        // Normal pointing outward
        float nx = (x1_0 + x2_0) / 2.0f;
        float nz = (z1_0 + z2_0) / 2.0f;
        float nlen = std::sqrt(nx*nx + nz*nz);
        nx /= nlen;
        nz /= nlen;
        
        // Base wall color with alternating shades
        glm::vec3 color = (i % 2 == 0) ? wallColor : wallColorDark;
        
        // Main wall section (quad = 2 triangles)
        // First triangle
        vertices.push_back(x1_0);
        vertices.push_back(0.0f);
        vertices.push_back(z1_0);
        vertices.push_back(nx);
        vertices.push_back(0.0f);
        vertices.push_back(nz);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
        
        vertices.push_back(x2_0);
        vertices.push_back(0.0f);
        vertices.push_back(z2_0);
        vertices.push_back(nx);
        vertices.push_back(0.0f);
        vertices.push_back(nz);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
        
        vertices.push_back(x2_0);
        vertices.push_back(wallHeight);
        vertices.push_back(z2_0);
        vertices.push_back(nx);
        vertices.push_back(0.0f);
        vertices.push_back(nz);
        vertices.push_back(color.r + 0.08f);
        vertices.push_back(color.g + 0.08f);
        vertices.push_back(color.b + 0.08f);
        
        // Second triangle
        vertices.push_back(x1_0);
        vertices.push_back(0.0f);
        vertices.push_back(z1_0);
        vertices.push_back(nx);
        vertices.push_back(0.0f);
        vertices.push_back(nz);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
        
        vertices.push_back(x2_0);
        vertices.push_back(wallHeight);
        vertices.push_back(z2_0);
        vertices.push_back(nx);
        vertices.push_back(0.0f);
        vertices.push_back(nz);
        vertices.push_back(color.r + 0.08f);
        vertices.push_back(color.g + 0.08f);
        vertices.push_back(color.b + 0.08f);
        
        vertices.push_back(x1_0);
        vertices.push_back(wallHeight);
        vertices.push_back(z1_0);
        vertices.push_back(nx);
        vertices.push_back(0.0f);
        vertices.push_back(nz);
        vertices.push_back(color.r + 0.08f);
        vertices.push_back(color.g + 0.08f);
        vertices.push_back(color.b + 0.08f);
        
        // Add door on side 0 (large dark rectangle)
        if (i == 0) {
            float doorX1 = x1_0 * 0.6f;
            float doorZ1 = z1_0 * 0.6f;
            float doorX2 = x2_0 * 0.6f;
            float doorZ2 = z2_0 * 0.6f;
            float doorY1 = 0.2f;
            float doorY2 = 1.8f;
            
            // Door quad (2 triangles, pushed out slightly)
            float doorOffset = 0.12f;
            float doorNx = nx * doorOffset;
            float doorNz = nz * doorOffset;
            
            // Tri 1
            vertices.push_back(doorX1 + doorNx);
            vertices.push_back(doorY1);
            vertices.push_back(doorZ1 + doorNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(doorColor.r);
            vertices.push_back(doorColor.g);
            vertices.push_back(doorColor.b);
            
            vertices.push_back(doorX2 + doorNx);
            vertices.push_back(doorY1);
            vertices.push_back(doorZ2 + doorNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(doorColor.r);
            vertices.push_back(doorColor.g);
            vertices.push_back(doorColor.b);
            
            vertices.push_back(doorX2 + doorNx);
            vertices.push_back(doorY2);
            vertices.push_back(doorZ2 + doorNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(doorColor.r + 0.1f);
            vertices.push_back(doorColor.g + 0.1f);
            vertices.push_back(doorColor.b + 0.1f);
            
            // Tri 2
            vertices.push_back(doorX1 + doorNx);
            vertices.push_back(doorY1);
            vertices.push_back(doorZ1 + doorNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(doorColor.r);
            vertices.push_back(doorColor.g);
            vertices.push_back(doorColor.b);
            
            vertices.push_back(doorX2 + doorNx);
            vertices.push_back(doorY2);
            vertices.push_back(doorZ2 + doorNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(doorColor.r + 0.1f);
            vertices.push_back(doorColor.g + 0.1f);
            vertices.push_back(doorColor.b + 0.1f);
            
            vertices.push_back(doorX1 + doorNx);
            vertices.push_back(doorY2);
            vertices.push_back(doorZ1 + doorNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(doorColor.r + 0.1f);
            vertices.push_back(doorColor.g + 0.1f);
            vertices.push_back(doorColor.b + 0.1f);
        }
        
        // Add windows on sides 1, 3, 5, 7 (odd sides)
        if (i % 2 == 1) {
            float winX1 = x1_0 * 0.75f;
            float winZ1 = z1_0 * 0.75f;
            float winX2 = x2_0 * 0.75f;
            float winZ2 = z2_0 * 0.75f;
            float winY1 = 1.2f;
            float winY2 = 1.7f;
            float winOffset = 0.1f;
            float winNx = nx * winOffset;
            float winNz = nz * winOffset;
            
            // Window quad (2 triangles)
            // Tri 1
            vertices.push_back(winX1 + winNx);
            vertices.push_back(winY1);
            vertices.push_back(winZ1 + winNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(windowColor.r);
            vertices.push_back(windowColor.g);
            vertices.push_back(windowColor.b);
            
            vertices.push_back(winX2 + winNx);
            vertices.push_back(winY1);
            vertices.push_back(winZ2 + winNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(windowColor.r);
            vertices.push_back(windowColor.g);
            vertices.push_back(windowColor.b);
            
            vertices.push_back(winX2 + winNx);
            vertices.push_back(winY2);
            vertices.push_back(winZ2 + winNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(windowColor.r + 0.1f);
            vertices.push_back(windowColor.g + 0.1f);
            vertices.push_back(windowColor.b + 0.1f);
            
            // Tri 2
            vertices.push_back(winX1 + winNx);
            vertices.push_back(winY1);
            vertices.push_back(winZ1 + winNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(windowColor.r);
            vertices.push_back(windowColor.g);
            vertices.push_back(windowColor.b);
            
            vertices.push_back(winX2 + winNx);
            vertices.push_back(winY2);
            vertices.push_back(winZ2 + winNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(windowColor.r + 0.1f);
            vertices.push_back(windowColor.g + 0.1f);
            vertices.push_back(windowColor.b + 0.1f);
            
            vertices.push_back(winX1 + winNx);
            vertices.push_back(winY2);
            vertices.push_back(winZ1 + winNz);
            vertices.push_back(nx);
            vertices.push_back(0.0f);
            vertices.push_back(nz);
            vertices.push_back(windowColor.r + 0.1f);
            vertices.push_back(windowColor.g + 0.1f);
            vertices.push_back(windowColor.b + 0.1f);
        }
    }
    
    // Conical roof: triangle fan from top apex down to cylinder edge with alternating colors
    float roofApexY = wallHeight + roofHeight;
    for (int i = 0; i < sides; i++) {
        float angle1 = 2.0f * 3.14159f * i / sides;
        float angle2 = 2.0f * 3.14159f * ((i + 1) % sides) / sides;
        
        float x1 = radius * std::cos(angle1);
        float z1 = radius * std::sin(angle1);
        float x2 = radius * std::cos(angle2);
        float z2 = radius * std::sin(angle2);
        
        glm::vec3 color = (i % 2 == 0) ? roofColor : roofColorLight;
        
        // Triangle: apex, v1, v2
        // Apex
        vertices.push_back(0.0f);
        vertices.push_back(roofApexY);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.7f);
        vertices.push_back(-0.7f);
        vertices.push_back(color.r);
        vertices.push_back(color.g);
        vertices.push_back(color.b);
        
        // v1
        vertices.push_back(x1);
        vertices.push_back(wallHeight);
        vertices.push_back(z1);
        float nx1 = x1 / radius;
        float nz1 = z1 / radius;
        vertices.push_back(nx1 * 0.6f);
        vertices.push_back(0.7f);
        vertices.push_back(nz1 * 0.6f);
        vertices.push_back(color.r - 0.08f);
        vertices.push_back(color.g - 0.08f);
        vertices.push_back(color.b - 0.08f);
        
        // v2
        vertices.push_back(x2);
        vertices.push_back(wallHeight);
        vertices.push_back(z2);
        float nx2 = x2 / radius;
        float nz2 = z2 / radius;
        vertices.push_back(nx2 * 0.6f);
        vertices.push_back(0.7f);
        vertices.push_back(nz2 * 0.6f);
        vertices.push_back(color.r - 0.08f);
        vertices.push_back(color.g - 0.08f);
        vertices.push_back(color.b - 0.08f);
    }
    
    // Add smoke hole at roof peak (small dark circle)
    float smokeRadius = 0.3f;
    for (int i = 0; i < 6; i++) {
        float a1 = 2.0f * 3.14159f * i / 6;
        float a2 = 2.0f * 3.14159f * ((i + 1) % 6) / 6;
        
        float x1 = smokeRadius * std::cos(a1);
        float z1 = smokeRadius * std::sin(a1);
        float x2 = smokeRadius * std::cos(a2);
        float z2 = smokeRadius * std::sin(a2);
        
        // Triangle to apex
        vertices.push_back(0.0f);
        vertices.push_back(roofApexY);
        vertices.push_back(0.0f);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.15f);
        vertices.push_back(0.1f);
        vertices.push_back(0.05f);
        
        vertices.push_back(x1);
        vertices.push_back(roofApexY - 0.1f);
        vertices.push_back(z1);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.15f);
        vertices.push_back(0.1f);
        vertices.push_back(0.05f);
        
        vertices.push_back(x2);
        vertices.push_back(roofApexY - 0.1f);
        vertices.push_back(z2);
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);
        vertices.push_back(0.15f);
        vertices.push_back(0.1f);
        vertices.push_back(0.05f);
    }
    
    hutVertexCount = vertices.size() / 9;
    
    glGenVertexArrays(1, &hutVAO);
    glGenBuffers(1, &hutVBO);
    glGenBuffers(1, &hutInstanceVBO);
    
    glBindVertexArray(hutVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hutVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
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

void ColonySystem::SetupHutInstances() {
    hutTranslations.clear();
    
    // Move colony far away - island remembers itself without inhabitants
    // Use impossible coordinates far off the visible world
    const float colonyX = -1000.0f;
    const float colonyZ = -1000.0f;
    const float colonyRadius = 80.0f;
    const int numHuts = 50;
    
    // Spiral/grid pattern far away
    for (int i = 0; i < numHuts; i++) {
        float t = (float)i / numHuts;
        float angle = t * 8.0f * 3.14159f;
        float r = t * colonyRadius;
        
        float x = colonyX + r * std::cos(angle);
        float z = colonyZ + r * std::sin(angle);
        
        float y = 10.0f;
        
        hutTranslations.push_back(glm::vec3(x, y, z));
    }
    
    glBindVertexArray(hutVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hutInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, hutTranslations.size() * sizeof(glm::vec3),
                 hutTranslations.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

void ColonySystem::SetupVillagerGeometry() {
    // Simple, well-defined static people with clear geometric shapes
    float personVertices[] = {
        // ===== HEAD (0.6x0.6 square, peach skin) =====
        -0.3f, 3.0f, -0.3f,     0.0f, 0.0f, 1.0f,   0.95f, 0.75f, 0.55f,
         0.3f, 3.0f, -0.3f,     0.0f, 0.0f, 1.0f,   0.95f, 0.75f, 0.55f,
         0.3f, 3.6f, -0.3f,     0.0f, 0.0f, 1.0f,   0.95f, 0.75f, 0.55f,
        -0.3f, 3.0f, -0.3f,     0.0f, 0.0f, 1.0f,   0.95f, 0.75f, 0.55f,
         0.3f, 3.6f, -0.3f,     0.0f, 0.0f, 1.0f,   0.95f, 0.75f, 0.55f,
        -0.3f, 3.6f, -0.3f,     0.0f, 0.0f, 1.0f,   0.95f, 0.75f, 0.55f,
        
        // ===== LEFT EYE (white dot) =====
        -0.15f, 3.35f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
        -0.08f, 3.35f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
        -0.08f, 3.42f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
        -0.15f, 3.35f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
        -0.08f, 3.42f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
        -0.15f, 3.42f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
        
        // ===== LEFT PUPIL (black dot) =====
        -0.12f, 3.37f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
        -0.08f, 3.37f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
        -0.08f, 3.41f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
        -0.12f, 3.37f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
        -0.08f, 3.41f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
        -0.12f, 3.41f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
        
        // ===== RIGHT EYE (white dot) =====
         0.08f, 3.35f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
         0.15f, 3.35f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
         0.15f, 3.42f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
         0.08f, 3.35f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
         0.15f, 3.42f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
         0.08f, 3.42f, -0.31f,  0.0f, 0.0f, -1.0f,  0.95f, 0.95f, 0.95f,
        
        // ===== RIGHT PUPIL (black dot) =====
         0.08f, 3.37f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
         0.12f, 3.37f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
         0.12f, 3.41f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
         0.08f, 3.37f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
         0.12f, 3.41f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
         0.08f, 3.41f, -0.32f,  0.0f, 0.0f, -1.0f,  0.1f, 0.1f, 0.1f,
        
        // ===== MOUTH (red smile) =====
        -0.15f, 3.15f, -0.31f,  0.0f, 0.0f, -1.0f,  0.9f, 0.3f, 0.3f,
         0.15f, 3.15f, -0.31f,  0.0f, 0.0f, -1.0f,  0.9f, 0.3f, 0.3f,
         0.0f, 3.08f, -0.31f,   0.0f, 0.0f, -1.0f,  0.9f, 0.3f, 0.3f,
        
        // ===== TORSO (1.0 wide, 2.0 tall, yellow shirt) =====
        -0.5f, 1.0f, -0.5f,     0.0f, 0.0f, 1.0f,   0.95f, 0.85f, 0.4f,
         0.5f, 1.0f, -0.5f,     0.0f, 0.0f, 1.0f,   0.95f, 0.85f, 0.4f,
         0.5f, 3.0f, -0.5f,     0.0f, 0.0f, 1.0f,   0.95f, 0.85f, 0.4f,
        -0.5f, 1.0f, -0.5f,     0.0f, 0.0f, 1.0f,   0.95f, 0.85f, 0.4f,
         0.5f, 3.0f, -0.5f,     0.0f, 0.0f, 1.0f,   0.95f, 0.85f, 0.4f,
        -0.5f, 3.0f, -0.5f,     0.0f, 0.0f, 1.0f,   0.95f, 0.85f, 0.4f,
        
        // ===== LEFT ARM (0.3 wide, 1.5 tall, peach) =====
        -0.8f, 1.8f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
        -0.5f, 1.8f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
        -0.5f, 3.3f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
        -0.8f, 1.8f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
        -0.5f, 3.3f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
        -0.8f, 3.3f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
        
        // ===== RIGHT ARM (0.3 wide, 1.5 tall, peach, symmetric) =====
         0.5f, 1.8f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
         0.8f, 1.8f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
         0.8f, 3.3f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
         0.5f, 1.8f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
         0.8f, 3.3f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
         0.5f, 3.3f, -0.3f,     0.0f, 0.0f, 1.0f,   0.9f, 0.7f, 0.5f,
        
        // ===== LEFT LEG (0.35 wide, 1.0 tall, dark blue pants) =====
        -0.35f, 0.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
        -0.05f, 0.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
        -0.05f, 1.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
        -0.35f, 0.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
        -0.05f, 1.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
        -0.35f, 1.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
        
        // ===== RIGHT LEG (0.35 wide, 1.0 tall, dark blue pants, symmetric) =====
         0.05f, 0.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
         0.35f, 0.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
         0.35f, 1.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
         0.05f, 0.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
         0.35f, 1.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
         0.05f, 1.0f, -0.4f,    0.0f, 0.0f, 1.0f,   0.2f, 0.2f, 0.5f,
        
        // ===== LEFT FOOT (0.35 wide, 0.2 tall, black shoe) =====
        -0.35f, -0.2f, -0.35f,  0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
        -0.05f, -0.2f, -0.35f,  0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
        -0.05f, 0.0f, -0.35f,   0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
        -0.35f, -0.2f, -0.35f,  0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
        -0.05f, 0.0f, -0.35f,   0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
        -0.35f, 0.0f, -0.35f,   0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
        
        // ===== RIGHT FOOT (0.35 wide, 0.2 tall, black shoe, symmetric) =====
         0.05f, -0.2f, -0.35f,  0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
         0.35f, -0.2f, -0.35f,  0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
         0.35f, 0.0f, -0.35f,   0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
         0.05f, -0.2f, -0.35f,  0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
         0.35f, 0.0f, -0.35f,   0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
         0.05f, 0.0f, -0.35f,   0.0f, 0.0f, 1.0f,   0.1f, 0.1f, 0.1f,
    };
    villagerVertexCount = sizeof(personVertices) / (9 * sizeof(float));
    
    glGenVertexArrays(1, &villagerVAO);
    glGenBuffers(1, &villagerVBO);
    glGenBuffers(1, &villagerInstanceVBO);
    
    glBindVertexArray(villagerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, villagerVBO);
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

void ColonySystem::SetupVillagerInstances() {
    villagerTranslations.clear();
    villagerBases.clear();
    villagerPatrolRadius.clear();
    villagerPatrolPhase.clear();
    
    // Find the same hilltop location as the huts
    float bestX = 380.0f;
    float bestZ = 380.0f;
    float maxHeight = -1.0f;
    
    // Sample a 10x10 grid to find the peak (same as huts)
    for (int sx = 0; sx < 10; sx++) {
        for (int sz = 0; sz < 10; sz++) {
            float testX = 340.0f + sx * 8.0f;
            float testZ = 340.0f + sz * 8.0f;
            float h = SampleHeight(testX, testZ);
            if (h > maxHeight) {
                maxHeight = h;
                bestX = testX;
                bestZ = testZ;
            }
        }
    }
    
    const float colonyX = bestX;
    const float colonyZ = bestZ;
    const float colonyRadius = 80.0f;
    const int numVillagers = 120;
    
    srand(2024);
    
    for (int i = 0; i < numVillagers; i++) {
        // Random position around colony within radius
        float angle = (float)(rand() % 628) / 100.0f;
        float r = (float)(rand() % 70) / 100.0f * colonyRadius;
        
        float bx = colonyX + r * std::cos(angle);
        float bz = colonyZ + r * std::sin(angle);
        float by = SampleHeight(bx, bz);
        
        villagerBases.push_back(glm::vec3(bx, by, bz));
        villagerTranslations.push_back(glm::vec3(bx, by, bz));
        villagerPatrolRadius.push_back(3.0f + (float)(rand() % 5));  // orbit radius 3-8
        villagerPatrolPhase.push_back((float)(rand() % 628) / 100.0f);
    }
    
    glBindVertexArray(villagerVAO);
    glBindBuffer(GL_ARRAY_BUFFER, villagerInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, villagerTranslations.size() * sizeof(glm::vec3),
                 villagerTranslations.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

void ColonySystem::Update(float currentTime) {
    // Static villagers - no animation needed
    // All positions are fixed at initialization
}

void ColonySystem::Render(const glm::mat4& projection, const glm::mat4& view,
                          const glm::vec3& skyColor, Shader& hutShader, Shader& villagerShader,
                          float currentTime) {
    // Render huts
    hutShader.use();
    hutShader.setMat4("projection", projection);
    hutShader.setMat4("view", view);
    hutShader.setVec3("skyColor", skyColor);
    
    glBindVertexArray(hutVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, hutVertexCount, (int)hutTranslations.size());
    glBindVertexArray(0);
    
    // Render villagers
    villagerShader.use();
    villagerShader.setMat4("projection", projection);
    villagerShader.setMat4("view", view);
    villagerShader.setVec3("skyColor", skyColor);
    villagerShader.setFloat("u_Time", currentTime);
    
    glBindVertexArray(villagerVAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, villagerVertexCount, (int)villagerTranslations.size());
    glBindVertexArray(0);
}

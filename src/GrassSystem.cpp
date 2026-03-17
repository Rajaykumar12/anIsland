#include "GrassSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>

GrassSystem::GrassSystem(const std::vector<float>& heightmap, int hmWidth, int hmHeight,
                         float terrainSizeX, float terrainSizeZ)
    : VAO(0), VBO(0), grassInstanceVBO(0), instanceCount(0) {
    
    generateGrassGeometry();
    generateGrassInstances(heightmap, hmWidth, hmHeight, terrainSizeX, terrainSizeZ);
    
    // Create VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // Create and fill VBO for blade vertices
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, bladeVertices.size() * sizeof(float),
                 bladeVertices.data(), GL_STATIC_DRAW);
    
    // Position attribute (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    // Instance VBO for offsets
    glGenBuffers(1, &grassInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, grassInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(GrassInstance),
                 instances.data(), GL_STATIC_DRAW);
    
    // Offset attribute (location 2) - instanced
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GrassInstance), (void*)0);
    glVertexAttribDivisor(2, 1); // One offset per instance
    
    glBindVertexArray(0);
    
    instanceCount = instances.size();
}

GrassSystem::~GrassSystem() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (grassInstanceVBO) glDeleteBuffers(1, &grassInstanceVBO);
}

void GrassSystem::generateGrassGeometry() {
    // Create a simple grass blade (two triangles forming a quad)
    // Each blade is small and positioned locally
    
    // Blade dimensions
    float width = 0.15f;
    float height = 0.6f;
    
    // First triangle (left side of blade)
    bladeVertices = {
        // Left triangle
        -width/2, 0.0f, 0.0f,
        -width/2, height, 0.0f,
         width/2, height, 0.0f,
        
        // Right triangle
        -width/2, 0.0f, 0.0f,
         width/2, height, 0.0f,
         width/2, 0.0f, 0.0f,
    };
}

void GrassSystem::generateGrassInstances(const std::vector<float>& heightmap, 
                                         int hmWidth, int hmHeight,
                                         float terrainSizeX, float terrainSizeZ) {
    instances.clear();
    
    // Blade density: one blade per ~4 units in world space
    float bladeSpacing = 2.0f;
    
    // Calculate heightmap to world conversion
    float hmToWorldX = terrainSizeX / hmWidth;
    float hmToWorldZ = terrainSizeZ / hmHeight;
    
    // Terrain center
    float centerX = terrainSizeX / 2.0f;
    float centerZ = terrainSizeZ / 2.0f;
    
    for (float x = -centerX; x < centerX; x += bladeSpacing) {
        for (float z = -centerZ; z < centerZ; z += bladeSpacing) {
            // Sample heightmap at this location
            float normalizedX = (x + centerX) / terrainSizeX;
            float normalizedZ = (z + centerZ) / terrainSizeZ;
            
            int hmX = static_cast<int>(normalizedX * hmWidth);
            int hmZ = static_cast<int>(normalizedZ * hmHeight);
            
            // Clamp to valid range
            hmX = std::max(0, std::min(hmWidth - 1, hmX));
            hmZ = std::max(0, std::min(hmHeight - 1, hmZ));
            
            float height = heightmap[hmZ * hmWidth + hmX];
            
            // Grass only grows on grass zone (flat, lower elevations)
            // Skip steep slopes and high elevations
            if (height < 35.0f) { // Grass zone threshold
                // Check slope - skip very steep terrain
                bool isSteep = false;
                if (hmX > 0 && hmX < hmWidth - 1 && hmZ > 0 && hmZ < hmHeight - 1) {
                    float heightLeft = heightmap[hmZ * hmWidth + (hmX - 1)];
                    float heightRight = heightmap[hmZ * hmWidth + (hmX + 1)];
                    float heightUp = heightmap[(hmZ - 1) * hmWidth + hmX];
                    float heightDown = heightmap[(hmZ + 1) * hmWidth + hmX];
                    
                    float slopeX = std::abs(heightRight - heightLeft);
                    float slopeZ = std::abs(heightDown - heightUp);
                    
                    if (slopeX > 15.0f || slopeZ > 15.0f) {
                        isSteep = true;
                    }
                }
                
                if (!isSteep) {
                    // Add some randomness to avoid grid pattern
                    float randomOffsetX = ((float)rand() / RAND_MAX - 0.5f) * 0.8f;
                    float randomOffsetZ = ((float)rand() / RAND_MAX - 0.5f) * 0.8f;
                    
                    GrassInstance instance;
                    instance.offset = glm::vec3(x + randomOffsetX, height, z + randomOffsetZ);
                    instances.push_back(instance);
                }
            }
        }
    }
}

void GrassSystem::Draw() const {
    if (instanceCount == 0) return;
    
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, 6, instanceCount);
    glBindVertexArray(0);
}

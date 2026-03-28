#include "GrassSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/common.hpp>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <cstddef>

namespace {
constexpr float TERRAIN_MAX_HEIGHT = 150.0f;
constexpr float WATERLINE_MIN_HEIGHT = 2.0f;
constexpr float MAX_GRASS_HEIGHT = 120.0f;
constexpr float MAX_SLOPE_DEG = 55.0f;
}

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

    // Normal attribute (location 3) - instanced
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(GrassInstance),
                          (void*)offsetof(GrassInstance, normal));
    glVertexAttribDivisor(3, 1); // One normal per instance
    
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
    
    // Increase density for fuller mountain vegetation
    float bladeSpacing = 1.4f;
    
    // Calculate heightmap to world conversion
    float hmToWorldX = terrainSizeX / hmWidth;
    float hmToWorldZ = terrainSizeZ / hmHeight;
    
    for (float x = 0.0f; x < terrainSizeX; x += bladeSpacing) {
        for (float z = 0.0f; z < terrainSizeZ; z += bladeSpacing) {
            // Terrain and grass share the same world-space coordinates.
            float normalizedX = x / terrainSizeX;
            float normalizedZ = z / terrainSizeZ;

            int hmX = static_cast<int>(normalizedX * (hmWidth - 1));
            int hmZ = static_cast<int>(normalizedZ * (hmHeight - 1));
            
            // Clamp to valid range
            hmX = std::max(0, std::min(hmWidth - 1, hmX));
            hmZ = std::max(0, std::min(hmHeight - 1, hmZ));
            
            float sampledHeight = heightmap[hmZ * hmWidth + hmX];
            float worldHeight = sampledHeight * TERRAIN_MAX_HEIGHT;

            // Keep grass above water and below high peaks.
            if (worldHeight <= WATERLINE_MIN_HEIGHT || worldHeight >= MAX_GRASS_HEIGHT) {
                continue;
            }

            glm::vec3 slopeNormal(0.0f, 1.0f, 0.0f);
            bool isSteep = false;

            if (hmX > 0 && hmX < hmWidth - 1 && hmZ > 0 && hmZ < hmHeight - 1) {
                float heightLeft = heightmap[hmZ * hmWidth + (hmX - 1)] * TERRAIN_MAX_HEIGHT;
                float heightRight = heightmap[hmZ * hmWidth + (hmX + 1)] * TERRAIN_MAX_HEIGHT;
                float heightUp = heightmap[(hmZ - 1) * hmWidth + hmX] * TERRAIN_MAX_HEIGHT;
                float heightDown = heightmap[(hmZ + 1) * hmWidth + hmX] * TERRAIN_MAX_HEIGHT;

                glm::vec3 tangent(2.0f * hmToWorldX, heightRight - heightLeft, 0.0f);
                glm::vec3 bitangent(0.0f, heightDown - heightUp, 2.0f * hmToWorldZ);

                slopeNormal = glm::normalize(glm::cross(bitangent, tangent));

                float slopeAngle = glm::degrees(
                    glm::acos(glm::clamp(slopeNormal.y, -1.0f, 1.0f))
                );
                if (slopeAngle > MAX_SLOPE_DEG) {
                    isSteep = true;
                }
            }

            if (!isSteep) {
                // Add some randomness to avoid grid pattern
                float randomOffsetX = ((float)rand() / RAND_MAX - 0.5f) * 0.8f;
                float randomOffsetZ = ((float)rand() / RAND_MAX - 0.5f) * 0.8f;

                GrassInstance instance;
                instance.offset = glm::vec3(x + randomOffsetX, worldHeight, z + randomOffsetZ);
                instance.normal = slopeNormal;
                instances.push_back(instance);
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

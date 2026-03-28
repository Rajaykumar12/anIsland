#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class GrassSystem {
public:
    GrassSystem(const std::vector<float>& heightmap, int hmWidth, int hmHeight, 
                float terrainSizeX, float terrainSizeZ);
    ~GrassSystem();
    
    void Draw() const;

private:
    unsigned int VAO, VBO, grassInstanceVBO;
    unsigned int instanceCount;
    
    struct GrassInstance {
        glm::vec3 offset;
        glm::vec3 normal;
    };
    
    void generateGrassGeometry();
    void generateGrassInstances(const std::vector<float>& heightmap, int hmWidth, int hmHeight,
                               float terrainSizeX, float terrainSizeZ);
    
    std::vector<float> bladeVertices;
    std::vector<GrassInstance> instances;
};

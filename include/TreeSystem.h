#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <string>

class Shader;

// Tree type definitions
struct TreeType {
    unsigned int VAO, VBO, instanceVBO;
    int vertexCount;
    std::vector<glm::vec3> instances;
    std::string name;
};

class TreeSystem {
public:
    TreeSystem(const std::vector<float>& noiseData, int hmWidth, int hmHeight, 
               int terrainWidth, int terrainHeight);
    ~TreeSystem();

    void Render(const glm::mat4& projection, const glm::mat4& view, 
                const glm::vec3& skyColor, Shader& shader);

    int GetTotalTreeCount() const;

private:
    std::vector<TreeType> treeTypes;

    void SetupGeometry();
    void SetupInstances(const std::vector<float>& noiseData, int hmWidth, int hmHeight,
                        int terrainWidth, int terrainHeight);
    
    void CreatePineTree();      // Tall, narrow, pointed
    void CreateOakTree();       // Round, wide canopy
    void CreateBirchTree();     // Thin trunk, small foliage
    void CreateFirTree();       // Dense, conical shape
};


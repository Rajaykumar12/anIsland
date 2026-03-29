#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Shader;

class SplashSystem {
public:
    SplashSystem(int maxSplashes,
                 const std::vector<float>& heightmap,
                 int hmWidth,
                 int hmHeight,
                 float terrainWidth,
                 float terrainDepth);
    ~SplashSystem();

    void Update(float dt, bool rainEnabled, const glm::vec3& cameraPos);
    void Render(const glm::mat4& projection, const glm::mat4& view, Shader& shader);

private:
    struct Splash {
        glm::vec3 position;
        float lifetime;
        float maxLifetime;
        float size;
    };

    int maxSplashes;
    std::vector<Splash> splashes;
    std::vector<glm::vec3> positions;
    std::vector<float> sizes;
    std::vector<float> terrainHeightmap;
    int heightmapWidth;
    int heightmapHeight;
    float terrainWidth;
    float terrainDepth;

    GLuint splashVAO, splashVBO, sizeVBO;

    void SpawnSplash(const glm::vec3& cameraPos);
    float SampleTerrainHeight(float x, float z) const;
    void UpdateGPU();
};

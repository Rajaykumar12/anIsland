#pragma once
#include <glm/glm.hpp>
#include <vector>

class Shader;

// Toggleable rain effect: 5000 falling GL_POINTS rendered with additive blending
class RainSystem {
public:
    explicit RainSystem(int maxDrops = 5000);
    ~RainSystem();

    void Update(float deltaTime, const glm::vec3& cameraPos);
    void Render(const glm::mat4& projection, const glm::mat4& view, Shader& shader);

    bool IsEnabled() const { return enabled; }
    void Toggle()         { enabled = !enabled; }

private:
    unsigned int rainVAO, rainVBO;
    int          maxDrops;
    bool         enabled    = false;
    bool         wasEnabled = false;  // tracks previous frame state for instant respawn

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;

    void InitDrops(const glm::vec3& cameraPos);
};

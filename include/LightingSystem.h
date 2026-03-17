#pragma once

#include <glm/glm.hpp>

class LightingSystem {
public:
    LightingSystem();
    ~LightingSystem();

    void Update(float currentTime);

    glm::vec3 GetSunPosition() const { return sunPos; }
    glm::vec3 GetSunColor() const { return lightColor; }
    glm::vec3 GetSkyColor() const { return skyColor; }
    float GetSunIntensity() const { return lightIntensity; }
    float GetTimeOfDay() const { return timeOfDay; }
    
    // Shadow mapping getters
    unsigned int GetDepthMapFBO() const { return depthMapFBO; }
    unsigned int GetDepthMapTexture() const { return depthMap; }
    glm::mat4 GetLightSpaceMatrix() const { return lightSpaceMatrix; }
    void UpdateShadowMatrices();

private:
    glm::vec3 sunPos;
    glm::vec3 lightColor;
    glm::vec3 skyColor;
    float lightIntensity;
    float timeOfDay;

    float daySpeed;
    float sunRadius;

    // Shadow mapping
    unsigned int depthMapFBO;
    unsigned int depthMap;
    glm::mat4 lightProjection;
    glm::mat4 lightView;
    glm::mat4 lightSpaceMatrix;

    void CalculateLighting();
    void SetupShadowMapping();
};

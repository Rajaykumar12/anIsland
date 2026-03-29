#pragma once

#include <glm/glm.hpp>

class LightingSystem {
public:
    LightingSystem();
    ~LightingSystem();

    void Update(float currentTime);

    glm::vec3 GetSunPosition()   const { return sunPos; }
    glm::vec3 GetSunColor()      const { return lightColor; }
    glm::vec3 GetSkyColor()      const { return skyColor; }
    glm::vec3 GetSunsetTint()    const { return sunsetTint; }
    glm::vec3 GetLightDir()      const { return glm::normalize(sunPos); }
    float     GetSunIntensity()  const { return lightIntensity; }
    float     GetTimeOfDay()     const { return timeOfDay; }
    float     GetDayIntensity()  const { return dayIntensity; }

    void SetManualTimeOfDay(float normalizedTime);
    void ClearManualTimeOverride();
    bool IsManualTimeEnabled() const { return useManualTime; }
    
    // Shadow mapping getters
    unsigned int GetDepthMapFBO()     const { return depthMapFBO; }
    unsigned int GetDepthMapTexture() const { return depthMap; }
    glm::mat4    GetLightSpaceMatrix()  const { return lightSpaceMatrix; }
    glm::mat4    GetLightProjection()   const { return lightProjection; }
    glm::mat4    GetLightView()         const { return lightView; }
    void UpdateShadowMatrices();

private:
    glm::vec3 sunPos;
    glm::vec3 lightColor;
    glm::vec3 skyColor;
    glm::vec3 sunsetTint;
    float lightIntensity;
    float timeOfDay;
    float dayIntensity;  // 0=night, 1=full day
    bool useManualTime;
    float manualTimeOfDay;

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

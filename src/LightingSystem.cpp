#include "LightingSystem.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <glad/glad.h>

const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

LightingSystem::LightingSystem() : daySpeed(0.1f), sunRadius(100.0f), 
                                   timeOfDay(0.5f), lightIntensity(1.0f),
                                   useManualTime(false), manualTimeOfDay(0.5f) {
    sunPos = glm::vec3(0.0f, sunRadius, 0.0f);
    lightColor = glm::vec3(1.0f, 1.0f, 0.9f);
    skyColor = glm::vec3(0.7f, 0.8f, 1.0f);
    
    SetupShadowMapping();
}

LightingSystem::~LightingSystem() {
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
}

void LightingSystem::SetupShadowMapping() {
    // Create framebuffer object
    glGenFramebuffers(1, &depthMapFBO);
    
    // Create depth texture
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    
    // Texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void LightingSystem::UpdateShadowMatrices() {
    float near_plane = 1.0f, far_plane = 300.0f;
    lightProjection = glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, near_plane, far_plane);
    lightView = glm::lookAt(sunPos, glm::vec3(400.0f, 0.0f, 400.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
}

void LightingSystem::Update(float currentTime) {
    float angle = 0.0f;
    if (useManualTime) {
        angle = glm::clamp(manualTimeOfDay, 0.0f, 1.0f) * 6.2831853f;
    } else {
        angle = currentTime * daySpeed;
    }

    // Calculate sun position with orbit
    float sunX = sunRadius * cos(angle);
    float sunY = sunRadius * sin(angle);
    sunPos = glm::vec3(sunX, sunY, 0.0f);
    
    CalculateLighting();
    UpdateShadowMatrices();
}

void LightingSystem::SetManualTimeOfDay(float normalizedTime) {
    useManualTime = true;
    manualTimeOfDay = glm::clamp(normalizedTime, 0.0f, 1.0f);
}

void LightingSystem::ClearManualTimeOverride() {
    useManualTime = false;
}

void LightingSystem::CalculateLighting() {
    // Calculate light color based on sun position (Y ranges from -100 to +100)
    timeOfDay = (sunPos.y + sunRadius) / (2.0f * sunRadius); // 0 to 1

    // dayIntensity: 0=night, 1=full day (smoothed at horizon)
    dayIntensity = glm::smoothstep(0.0f, 0.35f, timeOfDay);

    lightColor = glm::mix(
        glm::vec3(0.3f, 0.2f, 0.5f),  // Night: dark blue
        glm::vec3(1.0f, 1.0f, 0.9f),   // Day: bright yellow-white
        glm::clamp(timeOfDay, 0.0f, 1.0f)
    );
    lightIntensity = glm::mix(0.2f, 1.0f, glm::clamp(timeOfDay, 0.0f, 1.0f));

    // Sunset / sunrise band: peaks near horizon (timeOfDay ~0.25 and ~0.75)
    // Use the absolute-value trick: strongest when |sin(angle)| is small
    float sunAngle = timeOfDay * 2.0f * 3.14159f;
    float sunsetStrength = glm::smoothstep(0.15f, 0.35f, timeOfDay)
                         * glm::smoothstep(0.60f, 0.40f, timeOfDay);
    sunsetTint = glm::vec3(1.0f, 0.55f, 0.2f) * sunsetStrength * 0.9f;

    // Update sky color with sunset overlay
    glm::vec3 daySky   = glm::vec3(0.7f, 0.8f, 1.0f);
    glm::vec3 nightSky = glm::vec3(0.02f, 0.02f, 0.05f);
    skyColor = glm::mix(nightSky, daySky, glm::clamp(timeOfDay, 0.0f, 1.0f))
             + glm::vec3(0.25f, 0.12f, 0.04f) * sunsetStrength;  // warm sky at sunset
}


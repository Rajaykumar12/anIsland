#pragma once
#include "Camera.h"
#include <glm/glm.hpp>
#include <vector>
#include <functional>

// Cinematic shot segment with timing and camera behavior
struct ShotSegment {
    float startTime;      // When this segment begins (seconds)
    float duration;       // How long this segment lasts
    std::function<void(float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch)> updateFunc;
    const char* name;
};

class CinematicCamera : public Camera {
public:
    CinematicCamera(const std::vector<float>& heightmap, int hmWidth, int hmHeight,
                    float terrainWidth, float terrainDepth);

    // Update camera position based on cinematic time
    void Update(float cinematicTime, float deltaTime);

    // Start/stop cinematic mode
    void StartCinematic();
    void StopCinematic();
    bool IsCinematicActive() const { return isActive; }

    // Get current shot info
    const char* GetCurrentShotName() const;
    float GetCinematicTime() const { return currentTime; }

    // Get terrain height at world position
    float GetTerrainHeight(float worldX, float worldZ) const;
    float GetTerrainHeightClamped(float worldX, float worldZ) const;

    // Calculate minimum camera height above terrain
    float GetMinimumCameraHeight(float worldX, float worldZ) const;

    // Enable/disable user input
    void SetUserControlEnabled(bool enabled) { userControlEnabled = enabled; }
    bool IsUserControlEnabled() const { return userControlEnabled; }

private:
    std::vector<float> heightmapData;
    int hmWidth, hmHeight;
    float terrainWidth, terrainDepth;
    float hmWorldScaleX, hmWorldScaleZ;

    float currentTime;
    bool isActive;
    bool userControlEnabled;
    int currentShotIndex;

    // Shot definitions
    std::vector<ShotSegment> shots;

    void InitializeShots();
    int FindCurrentShot(float time) const;

    // Helper interpolation functions
    static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t);
    static float Lerp(float a, float b, float t);
    static float SmoothStep(float t);
    static float EaseInOut(float t);
};

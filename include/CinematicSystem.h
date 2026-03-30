#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <string>

class Camera;
class LightingSystem;

enum class SegmentType {
    SPLINE,
    ORBIT,
    FREEZE,
};

struct CameraKeyframe {
    float time;
    glm::vec3 position;
    glm::vec3 target;
    float fov;

    SegmentType segmentType = SegmentType::SPLINE;

    glm::vec3 orbitPivot = glm::vec3(0.0f);
    float orbitRadius = 80.0f;
    float orbitHeight = 90.0f;
    float orbitDuration = 12.0f;
    float orbitDegrees = 360.0f;

    float forcedTimeOfDay = -1.0f;
    float lightingSpeed = 1.0f;
};

class CinematicSystem {
public:
    CinematicSystem();
    ~CinematicSystem() = default;

    void init();
    bool update(float deltaTime, Camera& camera, LightingSystem* lighting);
    void reset();

    bool isPlaying() const { return m_playing; }
    float currentTime() const { return m_time; }
    float getFadeAlpha() const;

    std::string currentActName() const;

    static constexpr float TOTAL_DURATION = 300.0f;
    static constexpr float FADE_START_TIME = 294.0f;

private:
    void buildKeyframes();

    void evaluateCamera(float t,
                        glm::vec3& outPos,
                        glm::vec3& outTarget,
                        float& outFov) const;

    glm::vec3 evalOrbit(const CameraKeyframe& kf, float elapsed) const;
    int findSegment(float t) const;

    void applyToCamera(const glm::vec3& pos,
                       const glm::vec3& target,
                       float fov,
                       Camera& camera) const;

    static glm::vec3 catmullRom(const glm::vec3& p0,
                                const glm::vec3& p1,
                                const glm::vec3& p2,
                                const glm::vec3& p3,
                                float t);

    static float smoothstep(float t);

    std::vector<CameraKeyframe> m_keyframes;
    float m_time = 0.0f;
    bool m_playing = false;
};

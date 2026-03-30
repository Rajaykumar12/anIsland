#pragma once

#include <glm/glm.hpp>
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
    float orbitDuration = 10.0f;
    float orbitDegrees = 360.0f;
    float orbitHeight = 90.0f;

    float forcedTimeOfDay = -1.0f;
    float lightingSpeed = 1.0f;
};

struct StaticNPC {
    glm::vec3 position;
    glm::vec3 rotationEulerDeg;
    glm::mat4 modelMatrix;
    bool visible;
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

    const StaticNPC& npcA() const { return m_npcA; }
    const StaticNPC& npcB() const { return m_npcB; }
    const StaticNPC& npcC() const { return m_npcC; }

    std::string currentActName() const;

private:
    void buildKeyframes();
    void buildNPCData();

    static glm::vec3 catmullRom(const glm::vec3& p0,
                                const glm::vec3& p1,
                                const glm::vec3& p2,
                                const glm::vec3& p3,
                                float t);
    static float smoothstep(float t);

    void evaluateCamera(float t, glm::vec3& outPos, glm::vec3& outTarget, float& outFov);
    glm::vec3 evalOrbit(const CameraKeyframe& kf, float elapsed);
    int findSegment(float t) const;
    void applyToCamera(const glm::vec3& pos, const glm::vec3& target, float fov, Camera& camera);
    void updateNPCVisibility(float t);

    std::vector<CameraKeyframe> m_keyframes;
    float m_time = 0.0f;
    bool m_playing = false;
    int m_orbitIndex = -1;
    float m_orbitTimer = 0.0f;
    float m_orbitStartAngle = 0.0f;

    StaticNPC m_npcA;
    StaticNPC m_npcB;
    StaticNPC m_npcC;

    static constexpr float NPC_A_START = 35.0f;
    static constexpr float NPC_A_END = 163.0f;
    static constexpr float NPC_B_START = 150.0f;
    static constexpr float NPC_B_END = 253.0f;
    static constexpr float NPC_C_START = 250.0f;
    static constexpr float NPC_C_END = 375.0f;

    static constexpr float TOTAL_DURATION = 375.0f;
};

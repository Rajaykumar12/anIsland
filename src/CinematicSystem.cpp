#include "CinematicSystem.h"
#include "Camera.h"
#include "LightingSystem.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <cmath>

CinematicSystem::CinematicSystem() {
    buildNPCData();
}

void CinematicSystem::init() {
    buildKeyframes();
    reset();
}

void CinematicSystem::reset() {
    m_time = 0.0f;
    m_playing = true;
    m_orbitIndex = -1;
    m_orbitTimer = 0.0f;
    m_orbitStartAngle = 0.0f;
}

void CinematicSystem::buildNPCData() {
    m_npcA.position = glm::vec3(115.0f, 0.5f, -5.0f);
    m_npcA.rotationEulerDeg = glm::vec3(-90.0f, 180.0f, 0.0f);
    m_npcA.visible = false;
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, m_npcA.position);
        m = glm::rotate(m, glm::radians(m_npcA.rotationEulerDeg.y), glm::vec3(0, 1, 0));
        m = glm::rotate(m, glm::radians(m_npcA.rotationEulerDeg.x), glm::vec3(1, 0, 0));
        m = glm::rotate(m, glm::radians(m_npcA.rotationEulerDeg.z), glm::vec3(0, 0, 1));
        m_npcA.modelMatrix = m;
    }

    m_npcB.position = glm::vec3(125.0f, 5.0f, 125.0f);
    m_npcB.rotationEulerDeg = glm::vec3(0.0f, 315.0f, 0.0f);
    m_npcB.visible = false;
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, m_npcB.position);
        m = glm::rotate(m, glm::radians(m_npcB.rotationEulerDeg.y), glm::vec3(0, 1, 0));
        m = glm::scale(m, glm::vec3(1.0f, 0.5f, 1.0f));
        m_npcB.modelMatrix = m;
    }

    m_npcC.position = glm::vec3(62.0f, 78.0f, 68.0f);
    m_npcC.rotationEulerDeg = glm::vec3(0.0f, 225.0f, 0.0f);
    m_npcC.visible = false;
    {
        glm::mat4 m = glm::mat4(1.0f);
        m = glm::translate(m, m_npcC.position);
        m = glm::rotate(m, glm::radians(m_npcC.rotationEulerDeg.y), glm::vec3(0, 1, 0));
        m_npcC.modelMatrix = m;
    }
}

void CinematicSystem::buildKeyframes() {
    m_keyframes.clear();

    auto kf = [](float t, glm::vec3 pos, glm::vec3 tgt, float fov,
                 SegmentType seg = SegmentType::SPLINE,
                 float forcedToD = -1.0f, float lightSpd = 1.0f) -> CameraKeyframe {
        CameraKeyframe k;
        k.time = t;
        k.position = pos;
        k.target = tgt;
        k.fov = fov;
        k.segmentType = seg;
        k.forcedTimeOfDay = forcedToD;
        k.lightingSpeed = lightSpd;
        return k;
    };

    m_keyframes.push_back(kf(0.0f, glm::vec3(125, 1, -200), glm::vec3(125, 1, -80), 65.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(6.0f, glm::vec3(125, 2, -40), glm::vec3(125, 1, -20), 60.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(14.0f, glm::vec3(115, 3, -30), glm::vec3(115, 1, -8), 55.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(21.0f, glm::vec3(125, 40, -120), glm::vec3(125, 20, 125), 65.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(29.0f, glm::vec3(60, 5, 60), glm::vec3(80, 12, 90), 55.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));

    m_keyframes.push_back(kf(35.0f, glm::vec3(125, 3, -45), glm::vec3(115, 1, -5), 60.0f, SegmentType::SPLINE, glm::pi<float>(), 0.3f));
    m_keyframes.push_back(kf(42.0f, glm::vec3(108, 1.5f, -12), glm::vec3(115, 0.5f, -5), 45.0f, SegmentType::SPLINE, -1.0f, 0.3f));
    {
        CameraKeyframe orbitKf;
        orbitKf.time = 48.0f;
        orbitKf.position = glm::vec3(127, 1, -5);
        orbitKf.target = glm::vec3(115, 0.5f, -5);
        orbitKf.fov = 55.0f;
        orbitKf.segmentType = SegmentType::ORBIT;
        orbitKf.orbitPivot = glm::vec3(115, 1, -5);
        orbitKf.orbitRadius = 12.0f;
        orbitKf.orbitHeight = 1.0f;
        orbitKf.orbitDuration = 10.0f;
        orbitKf.orbitDegrees = 270.0f;
        orbitKf.forcedTimeOfDay = -1.0f;
        orbitKf.lightingSpeed = 0.3f;
        m_keyframes.push_back(orbitKf);
    }
    m_keyframes.push_back(kf(58.0f, glm::vec3(120, 0.8f, 8), glm::vec3(115, 0.0f, -5), 55.0f, SegmentType::SPLINE, -1.0f, 0.3f));
    m_keyframes.push_back(kf(63.0f, glm::vec3(115, 50.0f, -5), glm::vec3(115, 0.0f, -5), 60.0f, SegmentType::SPLINE, -1.0f, 0.3f));
    m_keyframes.push_back(kf(71.0f, glm::vec3(30, 4, -30), glm::vec3(0, 8, -200), 65.0f, SegmentType::SPLINE, -1.0f, 1.0f));

    m_keyframes.push_back(kf(75.0f, glm::vec3(30, 4, -30), glm::vec3(0, 10, -200), 65.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(81.0f, glm::vec3(30, 4, -30), glm::vec3(125, 80, 125), 60.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(89.0f, glm::vec3(50, 120, 50), glm::vec3(125, 60, 125), 60.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(99.0f, glm::vec3(125, 5, 125), glm::vec3(125, 5, 200), 55.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(105.0f, glm::vec3(115, 15, 20), glm::vec3(115, 0.5f, -5), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(111.0f, glm::vec3(60, 35, -40), glm::vec3(115, 0, -5), 65.0f, SegmentType::SPLINE, -1.0f, 1.0f));

    m_keyframes.push_back(kf(115.0f, glm::vec3(30, 4, 80), glm::vec3(60, 4, 110), 60.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(121.0f, glm::vec3(85, 4, 125), glm::vec3(105, 4, 140), 55.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(131.0f, glm::vec3(70, 4, 110), glm::vec3(70, 60, 110), 50.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(138.0f, glm::vec3(80, 100, 110), glm::vec3(125, 60, 125), 60.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    {
        CameraKeyframe orbitKf;
        orbitKf.time = 145.0f;
        orbitKf.position = glm::vec3(205, 90, 125);
        orbitKf.target = glm::vec3(125, 75, 125);
        orbitKf.fov = 65.0f;
        orbitKf.segmentType = SegmentType::ORBIT;
        orbitKf.orbitPivot = glm::vec3(125, 90, 125);
        orbitKf.orbitRadius = 80.0f;
        orbitKf.orbitHeight = 90.0f;
        orbitKf.orbitDuration = 10.0f;
        orbitKf.orbitDegrees = 360.0f;
        orbitKf.forcedTimeOfDay = -1.0f;
        orbitKf.lightingSpeed = 1.0f;
        m_keyframes.push_back(orbitKf);
    }
    m_keyframes.push_back(kf(155.0f, glm::vec3(100, 90, -20), glm::vec3(115, 0, -5), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));

    m_keyframes.push_back(kf(163.0f, glm::vec3(115, 3, 10), glm::vec3(115, 0.5f, -5), 55.0f, SegmentType::FREEZE, -1.0f, 1.5f));
    m_keyframes.push_back(kf(170.0f, glm::vec3(115, 3, 10), glm::vec3(125, 5, 125), 60.0f, SegmentType::SPLINE, -1.0f, 1.5f));
    m_keyframes.push_back(kf(175.0f, glm::vec3(125, 5, 115), glm::vec3(125, 5, 125), 55.0f, SegmentType::SPLINE, -1.0f, 1.5f));
    m_keyframes.push_back(kf(180.0f, glm::vec3(115, 5, 115), glm::vec3(125, 5, 125), 45.0f, SegmentType::FREEZE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(187.0f, glm::vec3(125, 8, 125), glm::vec3(62, 78, 68), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(193.0f, glm::vec3(130, 1.5f, 118), glm::vec3(125, 3, 125), 50.0f, SegmentType::SPLINE, -1.0f, 1.0f));

    m_keyframes.push_back(kf(200.0f, glm::vec3(200, 0.5f, 200), glm::vec3(230, 0.4f, 200), 50.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(206.0f, glm::vec3(180, 1.5f, 250), glm::vec3(210, 1.0f, 250), 55.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(212.0f, glm::vec3(50, 165, 50), glm::vec3(125, 100, 125), 65.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    {
        CameraKeyframe orbitKf;
        orbitKf.time = 218.0f;
        orbitKf.position = glm::vec3(185, 165, 125);
        orbitKf.target = glm::vec3(125, 100, 125);
        orbitKf.fov = 65.0f;
        orbitKf.segmentType = SegmentType::ORBIT;
        orbitKf.orbitPivot = glm::vec3(125, 165, 125);
        orbitKf.orbitRadius = 60.0f;
        orbitKf.orbitHeight = 165.0f;
        orbitKf.orbitDuration = 12.0f;
        orbitKf.orbitDegrees = 360.0f;
        orbitKf.forcedTimeOfDay = -1.0f;
        orbitKf.lightingSpeed = 1.0f;
        m_keyframes.push_back(orbitKf);
    }
    m_keyframes.push_back(kf(230.0f, glm::vec3(80, 2, -50), glm::vec3(125, 0, -20), 55.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(236.0f, glm::vec3(115, 8, 115), glm::vec3(125, 5, 125), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(243.0f, glm::vec3(30, 4, -30), glm::vec3(0, 5, -200), 65.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(249.0f, glm::vec3(100, 10, 110), glm::vec3(125, 5, 125), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));

    m_keyframes.push_back(kf(255.0f, glm::vec3(30, 2, -40), glm::vec3(0, 5, -200), 65.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(262.0f, glm::vec3(80, 1.5f, -50), glm::vec3(125, 0.0f, -20), 55.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(269.0f, glm::vec3(118, 3, -25), glm::vec3(115, 1, -8), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(275.0f, glm::vec3(115, 3, 10), glm::vec3(115, 0, -5), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(281.0f, glm::vec3(115, 8, 115), glm::vec3(125, 5, 125), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(287.0f, glm::vec3(30, 15, 30), glm::vec3(200, 20, 200), 65.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(kf(293.0f, glm::vec3(80, 20, 80), glm::vec3(62, 78, 68), 55.0f, SegmentType::FREEZE, -1.0f, 1.0f));

    m_keyframes.push_back(kf(300.0f, glm::vec3(80, 10, 90), glm::vec3(62, 40, 68), 60.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(307.0f, glm::vec3(65, 65, 72), glm::vec3(62, 78, 68), 55.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(317.0f, glm::vec3(50, 75, 55), glm::vec3(62, 78, 68), 45.0f, SegmentType::FREEZE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(325.0f, glm::vec3(62, 80, 68), glm::vec3(125, 0, -200), 65.0f, SegmentType::FREEZE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(333.0f, glm::vec3(55, 72, 60), glm::vec3(62, 78, 68), 55.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(341.0f, glm::vec3(125, 80, 125), glm::vec3(125, 0, 125), 60.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(349.0f, glm::vec3(125, 350, 125), glm::vec3(125, 0, 125), 60.0f, SegmentType::FREEZE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(357.0f, glm::vec3(115, 30, 20), glm::vec3(62, 78, 68), 55.0f, SegmentType::SPLINE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(367.0f, glm::vec3(58, 85, 62), glm::vec3(62, 78, 68), 50.0f, SegmentType::FREEZE, glm::pi<float>(), 0.0f));
    m_keyframes.push_back(kf(375.0f, glm::vec3(58, 85, 62), glm::vec3(62, 78, 68), 50.0f, SegmentType::FREEZE, glm::pi<float>(), 0.0f));

    std::sort(m_keyframes.begin(), m_keyframes.end(),
              [](const CameraKeyframe& a, const CameraKeyframe& b) {
                  return a.time < b.time;
              });
}

bool CinematicSystem::update(float deltaTime, Camera& camera, LightingSystem* lighting) {
    if (!m_playing) {
        return false;
    }

    m_time += deltaTime;

    int idx = findSegment(m_time);
    if (idx < 0) {
        idx = 0;
    }

    const CameraKeyframe& cur = m_keyframes[idx];

    if (lighting) {
        if (cur.forcedTimeOfDay >= 0.0f) {
            lighting->setTimeOfDay(cur.forcedTimeOfDay);
            lighting->setTimeScale(0.0f);
        } else {
            lighting->setTimeScale(cur.lightingSpeed);
        }
    }

    glm::vec3 camPos;
    glm::vec3 camTarget;
    float camFov = 45.0f;
    evaluateCamera(m_time, camPos, camTarget, camFov);
    applyToCamera(camPos, camTarget, camFov, camera);

    updateNPCVisibility(m_time);

    if (m_time >= TOTAL_DURATION) {
        m_playing = false;
        return false;
    }

    return true;
}

void CinematicSystem::evaluateCamera(float t,
                                     glm::vec3& outPos,
                                     glm::vec3& outTarget,
                                     float& outFov) {
    int idx = findSegment(t);
    if (idx < 0) {
        outPos = m_keyframes.front().position;
        outTarget = m_keyframes.front().target;
        outFov = m_keyframes.front().fov;
        return;
    }

    if (idx >= static_cast<int>(m_keyframes.size()) - 1) {
        outPos = m_keyframes.back().position;
        outTarget = m_keyframes.back().target;
        outFov = m_keyframes.back().fov;
        return;
    }

    const CameraKeyframe& cur = m_keyframes[idx];
    const CameraKeyframe& next = m_keyframes[idx + 1];

    float segLen = next.time - cur.time;
    float elapsed = t - cur.time;
    float rawT = (segLen > 0.0f) ? (elapsed / segLen) : 1.0f;
    float easeT = smoothstep(rawT);

    if (cur.segmentType == SegmentType::FREEZE) {
        outPos = cur.position;
        outTarget = cur.target;
        outFov = cur.fov;
        return;
    }

    if (cur.segmentType == SegmentType::ORBIT) {
        outPos = evalOrbit(cur, elapsed);
        outTarget = cur.orbitPivot;
        outFov = cur.fov;
        return;
    }

    int i0 = std::max(idx - 1, 0);
    int i1 = idx;
    int i2 = std::min(idx + 1, static_cast<int>(m_keyframes.size()) - 1);
    int i3 = std::min(idx + 2, static_cast<int>(m_keyframes.size()) - 1);

    outPos = catmullRom(m_keyframes[i0].position,
                        m_keyframes[i1].position,
                        m_keyframes[i2].position,
                        m_keyframes[i3].position,
                        easeT);

    outTarget = catmullRom(m_keyframes[i0].target,
                           m_keyframes[i1].target,
                           m_keyframes[i2].target,
                           m_keyframes[i3].target,
                           easeT);

    outFov = glm::mix(cur.fov, next.fov, easeT);
}

glm::vec3 CinematicSystem::evalOrbit(const CameraKeyframe& kf, float elapsed) {
    float t = std::min(elapsed / kf.orbitDuration, 1.0f);

    glm::vec3 startOffset = kf.position - kf.orbitPivot;
    float startAngle = std::atan2(startOffset.z, startOffset.x);

    float totalRad = glm::radians(kf.orbitDegrees);
    float angle = startAngle + totalRad * t;

    glm::vec3 pos;
    pos.x = kf.orbitPivot.x + kf.orbitRadius * std::cos(angle);
    pos.y = kf.orbitHeight;
    pos.z = kf.orbitPivot.z + kf.orbitRadius * std::sin(angle);
    return pos;
}

int CinematicSystem::findSegment(float t) const {
    if (m_keyframes.empty()) {
        return -1;
    }
    if (t <= m_keyframes.front().time) {
        return 0;
    }
    if (t >= m_keyframes.back().time) {
        return static_cast<int>(m_keyframes.size()) - 1;
    }

    int lo = 0;
    int hi = static_cast<int>(m_keyframes.size()) - 1;
    while (lo < hi - 1) {
        int mid = (lo + hi) / 2;
        if (m_keyframes[mid].time <= t) {
            lo = mid;
        } else {
            hi = mid;
        }
    }
    return lo;
}

glm::vec3 CinematicSystem::catmullRom(const glm::vec3& p0,
                                      const glm::vec3& p1,
                                      const glm::vec3& p2,
                                      const glm::vec3& p3,
                                      float t) {
    float t2 = t * t;
    float t3 = t2 * t;

    return 0.5f * ((2.0f * p1) +
                   (-p0 + p2) * t +
                   (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                   (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

float CinematicSystem::smoothstep(float t) {
    t = glm::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

void CinematicSystem::applyToCamera(const glm::vec3& pos,
                                    const glm::vec3& target,
                                    float fov,
                                    Camera& camera) {
    camera.Position = pos;

    glm::vec3 dir = glm::normalize(target - pos);
    float pitch = glm::degrees(std::asin(dir.y));
    float yaw = glm::degrees(std::atan2(dir.z, dir.x));

    camera.Yaw = yaw;
    camera.Pitch = pitch;
    camera.Zoom = fov;
    camera.updateCameraVectors();
}

void CinematicSystem::updateNPCVisibility(float t) {
    m_npcA.visible = (t >= NPC_A_START && t < NPC_A_END);
    m_npcB.visible = (t >= NPC_B_START && t < NPC_B_END);
    m_npcC.visible = (t >= NPC_C_START && t < NPC_C_END);
}

std::string CinematicSystem::currentActName() const {
    if (m_time < 35.0f) {
        return "Act 1 - The Boat";
    }
    if (m_time < 75.0f) {
        return "Act 2 - He Is on the Beach";
    }
    if (m_time < 115.0f) {
        return "Act 3 - The Sun Does Not Care";
    }
    if (m_time < 163.0f) {
        return "Act 4 - The Forest He Cannot Reach";
    }
    if (m_time < 200.0f) {
        return "Act 5 - He Got Up";
    }
    if (m_time < 253.0f) {
        return "Act 6 - The Long Afternoon";
    }
    if (m_time < 300.0f) {
        return "Act 7 - The Slow Fire and Empty Valley";
    }
    return "Act 8 - He Is on the Mountain";
}

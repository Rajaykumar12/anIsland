#include "CinematicSystem.h"
#include "Camera.h"
#include "LightingSystem.h"

#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cmath>

CinematicSystem::CinematicSystem() {}

void CinematicSystem::init() {
    buildKeyframes();
    reset();
}

void CinematicSystem::reset() {
    // Start directly at sunrise sequence while leaving timeline content unchanged.
    m_time = 70.0f;
    m_playing = true;
}

float CinematicSystem::getFadeAlpha() const {
    if (m_time < FADE_START_TIME) {
        return 0.0f;
    }
    float t = (m_time - FADE_START_TIME) / (TOTAL_DURATION - FADE_START_TIME);
    return glm::clamp(t, 0.0f, 1.0f);
}

bool CinematicSystem::update(float deltaTime, Camera& camera, LightingSystem* lighting) {
    if (!m_playing) {
        return false;
    }

    m_time += deltaTime;

    int idx = findSegment(m_time);
    if (idx >= 0 && idx < static_cast<int>(m_keyframes.size())) {
        const CameraKeyframe& kf = m_keyframes[idx];
        if (lighting) {
            if (kf.forcedTimeOfDay >= 0.0f) {
                lighting->setTimeOfDay(kf.forcedTimeOfDay);
                lighting->setTimeScale(0.0f);
            } else {
                lighting->setTimeScale(kf.lightingSpeed);
            }
        }
    }

    glm::vec3 pos;
    glm::vec3 tgt;
    float fov = 60.0f;
    evaluateCamera(m_time, pos, tgt, fov);
    applyToCamera(pos, tgt, fov, camera);

    if (m_time >= TOTAL_DURATION) {
        m_playing = false;
        return false;
    }
    return true;
}

void CinematicSystem::buildKeyframes() {
    m_keyframes.clear();

    auto K = [](float t,
                glm::vec3 pos, glm::vec3 tgt, float fov,
                SegmentType seg = SegmentType::SPLINE,
                float forcedToD = -1.0f,
                float lightSpd = 1.0f) -> CameraKeyframe {
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

    const float PI = glm::pi<float>();
    const float NIGHT = PI;
    const float NOON = PI * 0.5f;

    // Mountain-centered, high-altitude path only.
    // Camera keeps moving around the island to avoid fixed-position pans.
    const glm::vec3 center(125.0f, 118.0f, 125.0f);
    const glm::vec3 moonTarget(125.0f, 118.0f, 125.0f);

    // Act 1 (0-35): night establishing while traveling around island.
    m_keyframes.push_back(K(0.0f,  glm::vec3(125, 320, -260), center, 62.0f, SegmentType::SPLINE, NIGHT, 0.0f));
    m_keyframes.push_back(K(8.0f,  glm::vec3( 30, 305, -210), center, 60.0f, SegmentType::SPLINE, NIGHT, 0.0f));
    m_keyframes.push_back(K(16.0f, glm::vec3(-40, 295,  -80), center, 58.0f, SegmentType::SPLINE, NIGHT, 0.0f));
    m_keyframes.push_back(K(24.0f, glm::vec3(-25, 285,   90), center, 56.0f, SegmentType::SPLINE, NIGHT, 0.0f));
    m_keyframes.push_back(K(35.0f, glm::vec3( 40, 270,  240), center, 55.0f, SegmentType::SPLINE, NIGHT, 0.0f));

    // Act 2 (35-70): dawn with continuous movement.
    m_keyframes.push_back(K(45.0f, glm::vec3(170, 255, 300), center, 55.0f, SegmentType::SPLINE, -1.0f, 0.5f));
    m_keyframes.push_back(K(56.0f, glm::vec3(285, 242, 210), center, 54.0f, SegmentType::SPLINE, -1.0f, 0.5f));
    // Opening shot: face sunrise horizon and let the sun rise dynamically.
    m_keyframes.push_back(K(70.0f, glm::vec3(300, 230,  70), glm::vec3(520, 150, 70), 53.0f, SegmentType::FREEZE, -1.0f, 0.35f));

    // Act 3 (70-110): sunrise hero angle around mountain.
    m_keyframes.push_back(K(82.0f,  glm::vec3(240, 220, -20), center, 52.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(K(95.0f,  glm::vec3(190, 215, -75), center, 50.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(K(110.0f, glm::vec3(110, 212, -95), center, 50.0f, SegmentType::SPLINE, -1.0f, 1.0f));

    // Act 4 (110-165): forest canopy region viewed from above, then full around-mountain rotation.
    m_keyframes.push_back(K(125.0f, glm::vec3( 25, 210, -60), center, 52.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(K(140.0f, glm::vec3(240, 215,  55), center, 53.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    // Explicit center-above-mountain setup before the rotation shot.
    m_keyframes.push_back(K(148.0f, glm::vec3(125, 250, 125), center, 50.0f, SegmentType::FREEZE, -1.0f, 1.0f));
    // True moving orbit around mountain center with clear parallax.
    {
        CameraKeyframe orb;
        orb.time = 150.0f;
        orb.position = glm::vec3(325, 235, 125);
        orb.target = center;
        orb.fov = 54.0f;
        orb.segmentType = SegmentType::ORBIT;
        orb.orbitPivot = center;
        orb.orbitRadius = 200.0f;
        orb.orbitHeight = 235.0f;
        orb.orbitDuration = 18.0f;
        orb.orbitDegrees = 320.0f;
        orb.forcedTimeOfDay = -1.0f;
        orb.lightingSpeed = 1.0f;
        m_keyframes.push_back(orb);
    }
    // Orbit handoff at expected arc exit to keep transition smooth.
    m_keyframes.push_back(K(168.0f, glm::vec3(278, 235,  -4), center, 54.0f, SegmentType::SPLINE, -1.0f, 1.0f));

    // Act 5 (165-200): wide plains and coastline while keeping mountain centered.
    m_keyframes.push_back(K(178.0f, glm::vec3(250, 215,  80), center, 56.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(K(190.0f, glm::vec3(290, 210, 220), center, 56.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(K(200.0f, glm::vec3(230, 205, 300), center, 55.0f, SegmentType::SPLINE, -1.0f, 1.0f));

    // Act 6 (200-220): noon-adjacent rotation above mountain center without time-of-day snapping.
    m_keyframes.push_back(K(206.0f, glm::vec3(125, 270, 125), center, 52.0f, SegmentType::FREEZE, -1.0f, 0.7f));
    {
        CameraKeyframe orb;
        orb.time = 208.0f;
        orb.position = glm::vec3(125, 250, 338);
        orb.target = center;
        orb.fov = 54.0f;
        orb.segmentType = SegmentType::ORBIT;
        orb.orbitPivot = center;
        orb.orbitRadius = 220.0f;
        orb.orbitHeight = 250.0f;
        orb.orbitDuration = 12.0f;
        orb.orbitDegrees = 300.0f;
        orb.forcedTimeOfDay = -1.0f;
        orb.lightingSpeed = 0.7f;
        m_keyframes.push_back(orb);
    }
    // Noon orbit handoff at expected arc exit.
    m_keyframes.push_back(K(220.0f, glm::vec3(315, 250, 235), center, 54.0f, SegmentType::SPLINE, -1.0f, 0.7f));

    // Act 7 (220-230): sunset pass, continued travel.
    m_keyframes.push_back(K(225.0f, glm::vec3(260, 220,  20), center, 56.0f, SegmentType::SPLINE, -1.0f, 0.8f));
    m_keyframes.push_back(K(230.0f, glm::vec3(185, 225, -170), center, 56.0f, SegmentType::SPLINE, -1.0f, 1.0f));

    // Act 8 (230-300): night return + moon counterpart of sunrise shot.
    m_keyframes.push_back(K(240.0f, glm::vec3( 95, 235, -215), center, 56.0f, SegmentType::SPLINE, -1.0f, 1.0f));
    m_keyframes.push_back(K(250.0f, glm::vec3( 35, 240, -160), center, 54.0f, SegmentType::SPLINE, -1.0f, 0.45f));
    // Moon shot mirrored from sunrise composition.
    m_keyframes.push_back(K(258.0f, glm::vec3(190, 215, -75), moonTarget, 50.0f, SegmentType::FREEZE, -1.0f, 0.4f));
    m_keyframes.push_back(K(270.0f, glm::vec3(260, 250,  20), moonTarget, 52.0f, SegmentType::SPLINE, -1.0f, 0.4f));
    m_keyframes.push_back(K(282.0f, glm::vec3(230, 295, 170), center, 56.0f, SegmentType::SPLINE, -1.0f, 0.35f));
    // Freeze end state smoothly using timeScale=0 without forcing a new time-of-day.
    m_keyframes.push_back(K(294.0f, glm::vec3(125, 340, 260), center, 58.0f, SegmentType::FREEZE, -1.0f, 0.0f));
    m_keyframes.push_back(K(300.0f, glm::vec3(125, 340, 260), center, 58.0f, SegmentType::FREEZE, -1.0f, 0.0f));

    std::sort(m_keyframes.begin(), m_keyframes.end(),
              [](const CameraKeyframe& a, const CameraKeyframe& b) {
                  return a.time < b.time;
              });
}

void CinematicSystem::evaluateCamera(float t,
                                     glm::vec3& outPos,
                                     glm::vec3& outTarget,
                                     float& outFov) const {
    if (m_keyframes.empty()) {
        return;
    }

    int idx = findSegment(t);
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
    float easeT = smoothstep(glm::clamp(rawT, 0.0f, 1.0f));

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

glm::vec3 CinematicSystem::evalOrbit(const CameraKeyframe& kf, float elapsed) const {
    float t = glm::clamp(elapsed / kf.orbitDuration, 0.0f, 1.0f);

    glm::vec3 startOff = kf.position - kf.orbitPivot;
    float startAngle = std::atan2(startOff.z, startOff.x);

    float totalRad = glm::radians(kf.orbitDegrees);
    float angle = startAngle + totalRad * t;

    return glm::vec3(kf.orbitPivot.x + kf.orbitRadius * std::cos(angle),
                     kf.orbitHeight,
                     kf.orbitPivot.z + kf.orbitRadius * std::sin(angle));
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
                                    Camera& camera) const {
    // The cinematic keyframes are authored around a local center near (125, *, 125),
    // while the generated island/mountain sits near world center (~400, *, 400).
    // Shift cinematic coordinates into island-centered world space.
    const glm::vec3 worldOffset(275.0f, 0.0f, 275.0f);
    glm::vec3 worldPos = pos + worldOffset;
    glm::vec3 worldTarget = target + worldOffset;

    camera.Position = worldPos;

    glm::vec3 diff = worldTarget - worldPos;
    if (glm::length(diff) < 0.001f) {
        camera.updateCameraVectors();
        return;
    }

    glm::vec3 dir = glm::normalize(diff);

    camera.Pitch = glm::degrees(std::asin(glm::clamp(dir.y, -1.0f, 1.0f)));
    camera.Yaw = glm::degrees(std::atan2(dir.z, dir.x));
    camera.Zoom = fov;

    camera.updateCameraVectors();
}

std::string CinematicSystem::currentActName() const {
    if (m_time < 35.0f) return "Act 1 - The Deep";
    if (m_time < 70.0f) return "Act 2 - First Light";
    if (m_time < 110.0f) return "Act 3 - The Mountain Wakes";
    if (m_time < 165.0f) return "Act 4 - The Forest";
    if (m_time < 200.0f) return "Act 5 - The Breathing Plains";
    if (m_time < 220.0f) return "Act 6 - Full Noon";
    if (m_time < 230.0f) return "Act 7 - The Water Turns Gold";
    return "Act 8 - The Stars Return";
}

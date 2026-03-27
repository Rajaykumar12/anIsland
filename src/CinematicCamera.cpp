#include "CinematicCamera.h"
#include <glm/gtc/constants.hpp>
#include <algorithm>

CinematicCamera::CinematicCamera() : keyframes() {
}

void CinematicCamera::Initialize() {
    keyframes.clear();
    
    // "The Island Remembers" - A wordless journey through one complete day
    // The island is the protagonist. Ocean, forest, fireflies, and time.
    // Total duration: ~186 seconds (3m6s) - one full day cycle
    
    // ============ ACT 1: THE LAST HOUR OF DARK (0:00 – 0:35) ============
    // Underwater opening - the world before sunrise
    keyframes.push_back({{125.0f,  -8.0f, -60.0f}, {125.0f,   0.0f, -60.0f},   0.0f, 60.0f});  // underwater look-up
    keyframes.push_back({{125.0f,   1.0f, -80.0f}, {125.0f,   1.0f, -20.0f},   4.0f, 60.0f});  // surface breach
    keyframes.push_back({{125.0f,   3.0f,-120.0f}, {125.0f,  20.0f, 125.0f},  10.0f, 65.0f});  // island approach
    keyframes.push_back({{ 60.0f,   8.0f,  60.0f}, { 80.0f,  15.0f,  90.0f},  22.0f, 55.0f});  // firefly treeline
    
    // ============ ACT 2: SUN ARGUES WITH HORIZON (0:35 – 1:05) ============
    // Dawn breaks over the island
    keyframes.push_back({{ 30.0f,   4.0f, -30.0f}, {  0.0f,  10.0f,-200.0f},  35.0f, 65.0f});  // horizon watch east
    keyframes.push_back({{ 30.0f,   4.0f, -30.0f}, {125.0f,  80.0f, 125.0f},  44.0f, 60.0f});  // terrain reveal inland
    keyframes.push_back({{ 50.0f, 120.0f,  50.0f}, {125.0f,  60.0f, 125.0f},  52.0f, 60.0f});  // shadow descent on peak
    keyframes.push_back({{125.0f,   5.0f, 125.0f}, {125.0f,   5.0f, 200.0f},  62.0f, 55.0f});  // valley dawn ground level
    
    // ============ ACT 3: THE FOREST BREATHES (1:05 – 1:40) ============
    // Morning light inside 15,000 trees
    keyframes.push_back({{ 30.0f,   4.0f,  80.0f}, { 60.0f,   4.0f, 110.0f},  65.0f, 60.0f});  // forest entry
    keyframes.push_back({{ 85.0f,   4.0f, 125.0f}, {105.0f,   4.0f, 140.0f},  72.0f, 55.0f});  // trunk drift path
    keyframes.push_back({{ 70.0f,   4.0f, 110.0f}, { 70.0f,  60.0f, 110.0f},  80.0f, 50.0f});  // canopy look-up
    keyframes.push_back({{ 80.0f, 100.0f, 110.0f}, {125.0f,  60.0f, 125.0f},  88.0f, 60.0f});  // canopy break ascent
    keyframes.push_back({{180.0f,  90.0f, 125.0f}, {125.0f,  75.0f, 125.0f},  95.0f, 65.0f});  // forest orbit start
    keyframes.push_back({{125.0f,  90.0f, 205.0f}, {125.0f,  75.0f, 125.0f}, 105.0f, 65.0f});  // forest orbit continue
    
    // ============ ACT 4: WHAT THE WIND DOES (1:40 – 2:00) ============
    // Grasslands - blade level through rolling plains
    keyframes.push_back({{200.0f,   0.5f, 200.0f}, {230.0f,   0.4f, 200.0f}, 105.0f, 50.0f});  // blade level entry
    keyframes.push_back({{180.0f,   1.5f, 240.0f}, {210.0f,   1.0f, 240.0f}, 112.0f, 55.0f});  // wind wave track
    keyframes.push_back({{155.0f,  10.0f, 155.0f}, {170.0f,  20.0f, 170.0f}, 118.0f, 55.0f});  // slope boundary
    keyframes.push_back({{200.0f,  50.0f, 200.0f}, {200.0f,   0.0f, 200.0f}, 124.0f, 60.0f});  // plains pull-back
    
    // ============ ACT 5: HIGH NOON (2:00 – 2:25) ============
    // The island at maximum light - everything visible
    keyframes.push_back({{ 50.0f, 165.0f,  50.0f}, {125.0f, 100.0f, 125.0f}, 115.0f, 65.0f});  // peak arrival
    keyframes.push_back({{185.0f, 165.0f, 125.0f}, {125.0f, 100.0f, 125.0f}, 125.0f, 65.0f});  // 360° orbit start
    keyframes.push_back({{125.0f, 165.0f, 205.0f}, {125.0f, 100.0f, 125.0f}, 135.0f, 65.0f});  // orbit back
    keyframes.push_back({{125.0f, 165.0f, -100.0f}, {125.0f, -10.0f,  0.0f}, 142.0f, 70.0f});  // ocean look-down
    
    // ============ ACT 6: THE SLOW FIRE (2:25 – 2:50) ============
    // Dusk - the island's most dramatic hour
    keyframes.push_back({{ 30.0f,   2.0f, -40.0f}, {  0.0f,   5.0f,-200.0f}, 135.0f, 65.0f});  // shore sunset view
    keyframes.push_back({{ 80.0f,   1.5f, -50.0f}, {125.0f,   0.0f, -20.0f}, 142.0f, 55.0f});  // wave crests gold
    keyframes.push_back({{ 30.0f,   2.0f, -40.0f}, {125.0f,  40.0f, 125.0f}, 152.0f, 60.0f});  // inland shadow reveal
    keyframes.push_back({{ 30.0f,  15.0f,  30.0f}, {200.0f,  20.0f, 200.0f}, 160.0f, 65.0f});  // evening fog creeping
    
    // ============ ACT 7: FIREFLIES INHERIT THE ISLAND (2:50 – 3:06) ============
    // Night returns - the fireflies drift eternal
    keyframes.push_back({{ 60.0f,   3.0f,  85.0f}, { 80.0f,   8.0f, 110.0f}, 165.0f, 60.0f});  // firefly emergence
    keyframes.push_back({{118.0f,   6.0f, 155.0f}, {122.0f,   7.0f, 160.0f}, 170.0f, 30.0f});  // single particle close
    keyframes.push_back({{125.0f,  80.0f, 125.0f}, {125.0f,  20.0f, 125.0f}, 176.0f, 60.0f});  // slow ascent through swarm
    keyframes.push_back({{125.0f, 300.0f, 125.0f}, {125.0f,   0.0f, 125.0f}, 182.0f, 60.0f});  // god's eye final
    keyframes.push_back({{125.0f, 300.0f, 125.0f}, {125.0f,   0.0f, 125.0f}, 186.0f, 60.0f});  // fade to black hold
}

float CinematicCamera::GetTotalDuration() const {
    if (keyframes.empty()) return 0.0f;
    return keyframes.back().timestamp;
}

bool CinematicCamera::IsFinished(float elapsed) const {
    return elapsed >= GetTotalDuration();
}

int CinematicCamera::FindSegment(float elapsed, float& outLocalT) const {
    if (keyframes.size() < 2) return 0;
    
    // Find the segment and local time
    for (size_t i = 0; i < keyframes.size() - 1; ++i) {
        float t0 = keyframes[i].timestamp;
        float t1 = keyframes[i + 1].timestamp;
        
        if (elapsed >= t0 && elapsed <= t1) {
            float segmentDuration = t1 - t0;
            if (segmentDuration > 0.0f) {
                outLocalT = (elapsed - t0) / segmentDuration;
            } else {
                outLocalT = 0.0f;
            }
            return static_cast<int>(i);
        }
    }
    
    // If beyond last segment, clamp to end
    outLocalT = 1.0f;
    return static_cast<int>(keyframes.size() - 2);
}

glm::vec3 CinematicCamera::CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
                                      const glm::vec3& p2, const glm::vec3& p3,
                                      float t, float alpha) {
    // Catmull-Rom matrix coefficients (uniform parameterization, alpha=0.5)
    float t2 = t * t;
    float t3 = t2 * t;
    
    // Coefficients for uniform catmull-rom
    float c0 = -0.5f * t3 + t2 - 0.5f * t;
    float c1 =  1.5f * t3 - 2.5f * t2 + 1.0f;
    float c2 = -1.5f * t3 + 2.0f * t2 + 0.5f * t;
    float c3 =  0.5f * t3 - 0.5f * t2;
    
    return c0 * p0 + c1 * p1 + c2 * p2 + c3 * p3;
}

float CinematicCamera::Smoothstep(float t) {
    // Hermite smoothstep: 3t^2 - 2t^3
    t = glm::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

CinematicKeyframe CinematicCamera::GetInterpolatedState(float elapsed) const {
    if (keyframes.size() < 2) {
        CinematicKeyframe kf = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, 0.0f, 45.0f};
        return kf;
    }
    
    float localT = 0.0f;
    int segmentIdx = FindSegment(elapsed, localT);
    
    // Apply smoothstep easing for cinematic feel
    float smoothT = Smoothstep(localT);
    
    // For Catmull-Rom, we need 4 control points: p0, p1, p2, p3
    // We interpolate from p1 to p2 using p0 and p3 as neighbors
    
    int p0idx = glm::max(0, segmentIdx - 1);
    int p1idx = segmentIdx;
    int p2idx = segmentIdx + 1;
    int p3idx = glm::min(static_cast<int>(keyframes.size() - 1), segmentIdx + 2);
    
    const CinematicKeyframe& p0 = keyframes[p0idx];
    const CinematicKeyframe& p1 = keyframes[p1idx];
    const CinematicKeyframe& p2 = keyframes[p2idx];
    const CinematicKeyframe& p3 = keyframes[p3idx];
    
    // Interpolate position and target using Catmull-Rom
    glm::vec3 interpPos = CatmullRom(p0.position, p1.position, p2.position, p3.position, smoothT);
    glm::vec3 interpTarget = CatmullRom(p0.target, p1.target, p2.target, p3.target, smoothT);
    
    // Linear interpolation for FOV (simpler smooth effect)
    float interpFov = glm::mix(p1.fov, p2.fov, smoothT);
    
    CinematicKeyframe result = {
        interpPos,
        interpTarget,
        elapsed,
        interpFov
    };
    
    return result;
}

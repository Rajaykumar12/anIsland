#include "CinematicController.h"

#include <algorithm>
#include <cmath>

CinematicController::CinematicController()
    : time(0.0f), duration(300.0f), active(false), finished(false) {}

void CinematicController::Start() {
    time = 0.0f;
    active = true;
    finished = false;
}

void CinematicController::Stop() {
    active = false;
    finished = false;
    time = 0.0f;
}

void CinematicController::Update(float deltaTime) {
    if (!active) {
        return;
    }

    time += std::max(0.0f, deltaTime);
    if (time >= duration) {
        time = duration;
        active = false;
        finished = true;
    }
}

float CinematicController::EaseInOut(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

glm::vec3 CinematicController::Lerp(const glm::vec3& a, const glm::vec3& b, float t) {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

float CinematicController::LerpFloat(float a, float b, float t) {
    return a + (b - a) * std::clamp(t, 0.0f, 1.0f);
}

glm::vec3 CinematicController::Bezier3(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    float omt = 1.0f - t;
    return omt * omt * p0 + 2.0f * omt * t * p1 + t * t * p2;
}

CinematicFrame CinematicController::Evaluate() const {
    CinematicFrame frame{};

    const glm::vec3 islandCenter(400.0f, 0.0f, 400.0f);
    const glm::vec3 summit(400.0f, 155.0f, 400.0f);

    frame.cameraPosition = glm::vec3(360.0f, 190.0f, 352.0f);
    frame.cameraTarget = glm::vec3(520.0f, 118.0f, 500.0f);
    frame.cameraZoom = 47.0f;
    frame.windStrength = 0.25f;
    frame.fadeAlpha = 0.0f;
    frame.fadeColor = glm::vec3(0.03f, 0.05f, 0.11f);
    frame.fireflyBoost = 1.0f;

    // 0:00 - 0:45 Opening summit hold + subtle right pan
    if (time <= 45.0f) {
        if (time <= 12.0f) {
            frame.cameraPosition = glm::vec3(360.0f, 190.0f, 352.0f);
            frame.cameraTarget = glm::vec3(520.0f, 118.0f, 500.0f);
        } else {
            float u = EaseInOut((time - 12.0f) / 33.0f);
            float yaw = LerpFloat(-0.55f, -0.08f, u);
            float radius = 150.0f;
            frame.cameraPosition = glm::vec3(
                summit.x + std::cos(yaw) * radius,
                188.0f,
                summit.z + std::sin(yaw) * radius
            );
            frame.cameraTarget = glm::vec3(
                summit.x + std::cos(yaw + 0.70f) * 210.0f,
                110.0f,
                summit.z + std::sin(yaw + 0.70f) * 210.0f
            );
        }

        frame.cameraZoom = 47.0f;
        frame.windStrength = 0.20f;
    }
    // 0:45 - 1:45 Orbit around summit
    else if (time <= 105.0f) {
        float u = EaseInOut((time - 45.0f) / 60.0f);
        float theta = LerpFloat(-0.15f, 2.0f * 3.1415926f - 0.15f, u);
        float orbitRadius = 92.0f;
        frame.cameraPosition = glm::vec3(
            summit.x + std::cos(theta) * orbitRadius,
            170.0f + 4.0f * std::sin(theta * 1.4f),
            summit.z + std::sin(theta) * orbitRadius
        );
        frame.cameraTarget = summit + glm::vec3(0.0f, 8.0f, 0.0f);
        frame.cameraZoom = 44.0f;
        frame.windStrength = 0.28f;
    }
    // 1:45 - 2:15 Descent one (kept on the slope shoulder, never under the mountain)
    else if (time <= 135.0f) {
        float u = EaseInOut((time - 105.0f) / 30.0f);
        glm::vec3 p0(488.0f, 166.0f, 422.0f);
        glm::vec3 p1(522.0f, 138.0f, 455.0f);
        glm::vec3 p2(566.0f, 104.0f, 506.0f);
        glm::vec3 downslopeLookStart(546.0f, 132.0f, 462.0f);
        glm::vec3 downslopeLookEnd(644.0f, 78.0f, 572.0f);

        if (u < 0.70f) {
            float k = EaseInOut(u / 0.70f);
            frame.cameraPosition = Bezier3(p0, p1, p2, k);
            frame.cameraTarget = Lerp(downslopeLookStart, downslopeLookEnd, k);
            frame.cameraZoom = 42.0f;
        } else {
            float k = EaseInOut((u - 0.70f) / 0.30f);
            frame.cameraPosition = Lerp(p2, glm::vec3(574.0f, 96.0f, 520.0f), k);
            frame.cameraTarget = Lerp(downslopeLookEnd, glm::vec3(430.0f, 152.0f, 414.0f), k);
            frame.cameraZoom = 44.0f;
        }
        frame.windStrength = 0.33f;
    }
    // 2:15 - 3:00 Forest
    else if (time <= 180.0f) {
        float u = (time - 135.0f) / 45.0f;
        if (u < 0.45f) {
            float k = EaseInOut(u / 0.45f);
            frame.cameraPosition = Bezier3(
                glm::vec3(574.0f, 96.0f, 520.0f),
                glm::vec3(596.0f, 62.0f, 536.0f),
                glm::vec3(586.0f, 34.0f, 552.0f),
                k
            );
            frame.cameraTarget = Lerp(glm::vec3(590.0f, 40.0f, 552.0f), glm::vec3(610.0f, 30.0f, 590.0f), k);
            frame.cameraZoom = 40.0f;
        } else if (u < 0.78f) {
            float k = EaseInOut((u - 0.45f) / 0.33f);
            frame.cameraPosition = Bezier3(
                glm::vec3(586.0f, 34.0f, 552.0f),
                glm::vec3(572.0f, 30.0f, 578.0f),
                glm::vec3(556.0f, 32.0f, 604.0f),
                k
            );
            frame.cameraTarget = Lerp(glm::vec3(600.0f, 34.0f, 588.0f), glm::vec3(590.0f, 34.0f, 618.0f), k);
            frame.cameraZoom = 38.0f;
        } else {
            float k = EaseInOut((u - 0.78f) / 0.22f);
            frame.cameraPosition = Lerp(glm::vec3(556.0f, 32.0f, 604.0f), glm::vec3(548.0f, 86.0f, 598.0f), k);
            frame.cameraTarget = Lerp(glm::vec3(570.0f, 44.0f, 620.0f), glm::vec3(556.0f, 132.0f, 606.0f), k);
            frame.cameraZoom = 41.0f;
        }
        frame.windStrength = 0.58f;
    }
    // 3:00 - 3:30 Grasslands and water
    else if (time <= 210.0f) {
        float u = (time - 180.0f) / 30.0f;
        if (u < 0.65f) {
            float k = EaseInOut(u / 0.65f);
            frame.cameraPosition = Lerp(glm::vec3(548.0f, 86.0f, 598.0f), glm::vec3(644.0f, 10.0f, 612.0f), k);
            frame.cameraTarget = Lerp(glm::vec3(602.0f, 24.0f, 640.0f), glm::vec3(700.0f, 8.0f, 656.0f), k);
            frame.cameraZoom = 38.0f;
        } else {
            float k = EaseInOut((u - 0.65f) / 0.35f);
            frame.cameraPosition = Lerp(glm::vec3(644.0f, 10.0f, 612.0f), glm::vec3(704.0f, 5.8f, 640.0f), k);
            frame.cameraTarget = Lerp(glm::vec3(700.0f, 8.0f, 656.0f), glm::vec3(780.0f, 6.0f, 654.0f), k);
            frame.cameraZoom = 39.0f;
        }
        frame.windStrength = 0.78f;
    }
    // 3:30 - 4:20 Dusk and fireflies
    else if (time <= 260.0f) {
        float u = (time - 210.0f) / 50.0f;
        if (u < 0.50f) {
            float k = EaseInOut(u / 0.50f);
            frame.cameraPosition = Bezier3(
                glm::vec3(704.0f, 5.8f, 640.0f),
                glm::vec3(658.0f, 18.0f, 606.0f),
                glm::vec3(606.0f, 28.0f, 560.0f),
                k
            );
            frame.cameraTarget = Lerp(glm::vec3(770.0f, 7.0f, 650.0f), glm::vec3(560.0f, 46.0f, 520.0f), k);
            frame.cameraZoom = 42.0f;
        } else {
            // Deliberately dip to forest-edge altitude so fireflies cross the lens depth.
            float k = EaseInOut((u - 0.50f) / 0.50f);
            frame.cameraPosition = Bezier3(
                glm::vec3(606.0f, 28.0f, 560.0f),
                glm::vec3(560.0f, 18.5f, 532.0f),
                glm::vec3(528.0f, 58.0f, 506.0f),
                k
            );
            frame.cameraTarget = Lerp(glm::vec3(560.0f, 42.0f, 520.0f), glm::vec3(508.0f, 78.0f, 496.0f), k);
            frame.cameraZoom = 44.0f;
        }
        frame.windStrength = LerpFloat(0.60f, 0.40f, EaseInOut(u));

        // Lift firefly readability as dusk deepens, so particles are already present
        // when the camera enters the late-night wide shot.
        if (time > 238.0f) {
            float k = EaseInOut((time - 238.0f) / 22.0f);
            frame.fireflyBoost = LerpFloat(1.1f, 2.1f, k);
        }
    }
    // 4:20 - 5:00 Final rise and fade
    else {
        float u = EaseInOut((time - 260.0f) / 40.0f);
        float spin = LerpFloat(1.05f, 1.95f, u);
        float radius = LerpFloat(240.0f, 610.0f, u);
        frame.cameraPosition = glm::vec3(
            islandCenter.x + std::cos(spin) * radius,
            LerpFloat(130.0f, 500.0f, u),
            islandCenter.z + std::sin(spin) * radius
        );
        frame.cameraTarget = glm::vec3(400.0f, 28.0f, 400.0f);
        frame.cameraZoom = LerpFloat(48.0f, 52.0f, u);
        frame.windStrength = LerpFloat(0.30f, 0.18f, u);
        frame.fireflyBoost = LerpFloat(2.1f, 3.6f, u);

        if (time > 286.0f) {
            float f = EaseInOut((time - 286.0f) / 14.0f);
            frame.fadeAlpha = f;
            frame.fadeColor = Lerp(glm::vec3(0.03f, 0.06f, 0.14f), glm::vec3(0.0f, 0.0f, 0.0f), f);
        }
    }

    // Sun choreography: pre-dawn -> sunrise -> day -> dusk -> night
    float angle = -0.48f;
    if (time <= 45.0f) {
        angle = LerpFloat(-0.48f, -0.18f, time / 45.0f);
    } else if (time <= 105.0f) {
        angle = LerpFloat(-0.18f, 0.06f, (time - 45.0f) / 60.0f);
    } else if (time <= 210.0f) {
        angle = LerpFloat(0.06f, 1.05f, (time - 105.0f) / 105.0f);
    } else if (time <= 235.0f) {
        angle = LerpFloat(1.05f, -0.30f, (time - 210.0f) / 25.0f);
    } else if (time <= 260.0f) {
        angle = LerpFloat(-0.30f, -0.44f, (time - 235.0f) / 25.0f);
    } else {
        angle = LerpFloat(-0.44f, -0.50f, (time - 260.0f) / 40.0f);
    }

    float radius = 125.0f;
    frame.sunPosition = glm::vec3(radius * std::cos(angle), radius * std::sin(angle), 0.0f);

    return frame;
}

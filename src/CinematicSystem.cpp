#include "CinematicSystem.h"

#include <glm/glm.hpp>
#include <algorithm>

namespace {
float smoothLerp(float a, float b, float t) {
    float u = glm::clamp(t, 0.0f, 1.0f);
    u = u * u * (3.0f - 2.0f * u);
    return glm::mix(a, b, u);
}
}

CinematicSystem::CinematicSystem()
    : currentTime(0.0f), duration(300.0f), playing(true), complete(false) {
    keyframes = {
        // Act 1 (0-35s): opening mountain hero and shoreline approach
        {0.0f,   glm::vec3(120.0f, 65.0f, -250.0f), glm::vec3(120.0f, 55.0f, -30.0f), 68.0f, 0.07f, 0.0f},
        {14.0f,  glm::vec3(120.0f, 42.0f, -175.0f), glm::vec3(120.0f, 35.0f, -20.0f), 62.0f, 0.10f, 0.0f},
        {26.0f,  glm::vec3(118.0f, 12.0f, -80.0f),  glm::vec3(115.0f, 5.0f, -15.0f),  56.0f, 0.14f, 0.0f},
        {35.0f,  glm::vec3(112.0f, 3.5f, -28.0f),  glm::vec3(115.0f, 0.5f, -5.0f),   52.0f, 0.18f, 0.0f},

        // Act 2 (35-75s): NPC_A beat then reflective pullback
        {50.0f,  glm::vec3(102.0f, 3.0f, -16.0f),  glm::vec3(115.0f, 0.5f, -5.0f),   48.0f, 0.23f, 0.0f},
        {64.0f,  glm::vec3(95.0f, 18.0f, -78.0f),  glm::vec3(110.0f, 8.0f, -20.0f),  58.0f, 0.30f, 0.0f},
        {75.0f,  glm::vec3(48.0f, 20.0f, -95.0f),  glm::vec3(120.0f, 35.0f, 15.0f),  62.0f, 0.38f, 0.0f},

        // Act 3 (75-115s): exploration arc across interior
        {90.0f,  glm::vec3(52.0f, 64.0f, 18.0f),   glm::vec3(120.0f, 30.0f, 100.0f), 60.0f, 0.45f, 0.0f},
        {105.0f, glm::vec3(132.0f, 18.0f, 92.0f),  glm::vec3(145.0f, 8.0f, 165.0f),  54.0f, 0.50f, 0.0f},
        {115.0f, glm::vec3(90.0f, 12.0f, 120.0f),  glm::vec3(70.0f, 8.0f, 135.0f),   52.0f, 0.54f, 0.0f},

        // Act 4 (115-150s): transition to valley and handoff setup
        {132.0f, glm::vec3(70.0f, 40.0f, 100.0f),  glm::vec3(125.0f, 18.0f, 125.0f), 58.0f, 0.57f, 0.0f},
        {145.0f, glm::vec3(152.0f, 34.0f, 95.0f),  glm::vec3(125.0f, 5.0f, 125.0f),  54.0f, 0.59f, 0.0f},
        {150.0f, glm::vec3(136.0f, 10.0f, 115.0f), glm::vec3(125.0f, 5.0f, 125.0f),  50.0f, 0.61f, 0.0f},

        // Act 5 (150-185s): NPC_B presence and valley geography
        {166.0f, glm::vec3(112.0f, 9.0f, 108.0f),  glm::vec3(125.0f, 5.0f, 125.0f),  48.0f, 0.65f, 0.0f},
        {180.0f, glm::vec3(75.0f, 42.0f, 78.0f),   glm::vec3(125.0f, 18.0f, 125.0f), 58.0f, 0.69f, 0.0f},

        // Act 6 (185-215s): decision energy and sunset drift
        {198.0f, glm::vec3(58.0f, 16.0f, 42.0f),   glm::vec3(130.0f, 6.0f, 112.0f),  56.0f, 0.74f, 0.0f},
        {215.0f, glm::vec3(52.0f, 10.0f, -40.0f),  glm::vec3(108.0f, 8.0f, -5.0f),   58.0f, 0.80f, 0.0f},

        // Act 7 (215-250s): emptying valley and move toward mountain finale
        {232.0f, glm::vec3(90.0f, 6.0f, -24.0f),   glm::vec3(115.0f, 0.5f, -5.0f),   52.0f, 0.84f, 0.0f},
        {242.0f, glm::vec3(78.0f, 28.0f, 32.0f),   glm::vec3(98.0f, 28.0f, 110.0f),  60.0f, 0.88f, 0.0f},
        {250.0f, glm::vec3(82.0f, 34.0f, 60.0f),   glm::vec3(62.0f, 78.0f, 68.0f),   54.0f, 0.12f, 0.0f},

        // Act 8 (250-300s): mountain closing hero and fade
        {266.0f, glm::vec3(64.0f, 72.0f, 66.0f),   glm::vec3(62.0f, 78.0f, 68.0f),   46.0f, 0.09f, 0.0f},
        {282.0f, glm::vec3(86.0f, 130.0f, 90.0f),  glm::vec3(62.0f, 78.0f, 68.0f),   56.0f, 0.07f, 0.0f},
        {294.0f, glm::vec3(72.0f, 96.0f, 78.0f),   glm::vec3(62.0f, 78.0f, 68.0f),   50.0f, 0.05f, 0.0f},
        {299.0f, glm::vec3(68.0f, 86.0f, 72.0f),   glm::vec3(62.0f, 78.0f, 68.0f),   48.0f, 0.04f, 0.35f},
        {300.0f, glm::vec3(68.0f, 86.0f, 72.0f),   glm::vec3(62.0f, 78.0f, 68.0f),   48.0f, 0.04f, 1.0f}
    };
}

void CinematicSystem::Update(float deltaTime) {
    if (!playing || complete) {
        return;
    }

    currentTime += deltaTime;
    if (currentTime >= duration) {
        currentTime = duration;
        playing = false;
        complete = true;
    }
}

void CinematicSystem::Reset() {
    currentTime = 0.0f;
    playing = true;
    complete = false;
}

void CinematicSystem::TogglePlayPause() {
    if (complete) {
        return;
    }
    playing = !playing;
}

void CinematicSystem::Seek(float deltaSeconds) {
    currentTime = glm::clamp(currentTime + deltaSeconds, 0.0f, duration);
    complete = (currentTime >= duration);
    if (complete) {
        playing = false;
    }
}

float CinematicSystem::SampleScalar(float CinematicKeyframe::*field) const {
    if (keyframes.empty()) {
        return 0.0f;
    }
    if (currentTime <= keyframes.front().time) {
        return keyframes.front().*field;
    }
    if (currentTime >= keyframes.back().time) {
        return keyframes.back().*field;
    }

    for (size_t i = 0; i + 1 < keyframes.size(); ++i) {
        const CinematicKeyframe& a = keyframes[i];
        const CinematicKeyframe& b = keyframes[i + 1];
        if (currentTime >= a.time && currentTime <= b.time) {
            float span = std::max(0.0001f, b.time - a.time);
            float t = (currentTime - a.time) / span;
            return smoothLerp(a.*field, b.*field, t);
        }
    }

    return keyframes.back().*field;
}

glm::vec3 CinematicSystem::SampleVec3(glm::vec3 CinematicKeyframe::*field) const {
    if (keyframes.empty()) {
        return glm::vec3(0.0f);
    }
    if (currentTime <= keyframes.front().time) {
        return keyframes.front().*field;
    }
    if (currentTime >= keyframes.back().time) {
        return keyframes.back().*field;
    }

    for (size_t i = 0; i + 1 < keyframes.size(); ++i) {
        const CinematicKeyframe& a = keyframes[i];
        const CinematicKeyframe& b = keyframes[i + 1];
        if (currentTime >= a.time && currentTime <= b.time) {
            float span = std::max(0.0001f, b.time - a.time);
            float t = (currentTime - a.time) / span;
            float u = glm::clamp(t, 0.0f, 1.0f);
            u = u * u * (3.0f - 2.0f * u);
            return glm::mix(a.*field, b.*field, u);
        }
    }

    return keyframes.back().*field;
}

glm::vec3 CinematicSystem::GetCameraPosition() const {
    return SampleVec3(&CinematicKeyframe::position);
}

glm::vec3 CinematicSystem::GetCameraTarget() const {
    return SampleVec3(&CinematicKeyframe::target);
}

float CinematicSystem::GetCurrentFOV() const {
    return SampleScalar(&CinematicKeyframe::fov);
}

float CinematicSystem::GetForcedTimeOfDay() const {
    return glm::clamp(SampleScalar(&CinematicKeyframe::timeOfDay), 0.0f, 1.0f);
}

float CinematicSystem::GetFadeAlpha() const {
    return glm::clamp(SampleScalar(&CinematicKeyframe::fadeAlpha), 0.0f, 1.0f);
}

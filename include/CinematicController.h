#pragma once

#include <glm/glm.hpp>

struct CinematicFrame {
    glm::vec3 cameraPosition;
    glm::vec3 cameraTarget;
    glm::vec3 sunPosition;
    glm::vec3 fadeColor;
    float cameraZoom;
    float windStrength;
    float fadeAlpha;
    float fireflyBoost;
};

class CinematicController {
public:
    CinematicController();

    void Start();
    void Stop();
    void Update(float deltaTime);

    bool IsActive() const { return active; }
    bool IsFinished() const { return finished; }
    float GetDuration() const { return duration; }
    float GetTime() const { return time; }

    CinematicFrame Evaluate() const;

private:
    float time;
    float duration;
    bool active;
    bool finished;

    static float EaseInOut(float t);
    static glm::vec3 Lerp(const glm::vec3& a, const glm::vec3& b, float t);
    static float LerpFloat(float a, float b, float t);
    static glm::vec3 Bezier3(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, float t);
};

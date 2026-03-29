#pragma once

#include <glm/glm.hpp>
#include <vector>

struct CinematicKeyframe {
    float time;
    glm::vec3 position;
    glm::vec3 target;
    float fov;
    float timeOfDay;
    float fadeAlpha;
};

class CinematicSystem {
public:
    CinematicSystem();

    void Update(float deltaTime);
    void Reset();
    void TogglePlayPause();
    void Seek(float deltaSeconds);

    bool IsPlaying() const { return playing; }
    bool IsComplete() const { return complete; }

    float GetCurrentTime() const { return currentTime; }
    float GetDuration() const { return duration; }

    glm::vec3 GetCameraPosition() const;
    glm::vec3 GetCameraTarget() const;
    float GetCurrentFOV() const;
    float GetForcedTimeOfDay() const;
    float GetFadeAlpha() const;

private:
    float currentTime;
    float duration;
    bool playing;
    bool complete;
    std::vector<CinematicKeyframe> keyframes;

    float SampleScalar(float CinematicKeyframe::*field) const;
    glm::vec3 SampleVec3(glm::vec3 CinematicKeyframe::*field) const;
};

#pragma once
#include <glm/glm.hpp>
#include <vector>

/**
 * Represents a single keyframe in the cinematic camera sequence.
 * Includes position, look-at target, timestamp, and FOV.
 */
struct CinematicKeyframe {
    glm::vec3 position;      // Camera position
    glm::vec3 target;        // Camera look-at target
    float timestamp;         // Time in seconds from start
    float fov;              // Field of view in degrees (45-60 range typical)
};

/**
 * Manages cinematic camera interpolation using Catmull-Rom splines.
 * Provides smooth camera motion between keyframes for cinematic sequences.
 */
class CinematicCamera {
public:
    CinematicCamera();
    
    /**
     * Initialize the cinematic sequence with predefined keyframes.
     * Called once during setup.
     */
    void Initialize();
    
    /**
     * Get interpolated camera state at a specific elapsed time.
     * @param elapsed Time in seconds from start of cinematic
     * @return CinematicKeyframe with interpolated position, target, FOV
     */
    CinematicKeyframe GetInterpolatedState(float elapsed) const;
    
    /**
     * Get the total duration of the cinematic sequence in seconds.
     * @return Total duration in seconds
     */
    float GetTotalDuration() const;
    
    /**
     * Check if the cinematic has finished playing.
     * @param elapsed Time in seconds from start
     * @return True if elapsed >= total duration
     */
    bool IsFinished(float elapsed) const;

private:
    std::vector<CinematicKeyframe> keyframes;
    
    /**
     * Catmull-Rom spline interpolation for smooth camera motion.
     * Interpolates between p1 and p2 using neighboring control points p0 and p3.
     * @param p0, p1, p2, p3 Four control points (p0->p1 is interpolation segment)
     * @param t Local parameter [0, 1] within the segment
     * @param alpha Catmull-Rom alpha parameter (0.5 default for uniform catmull)
     * @return Interpolated value at parameter t
     */
    static glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1, 
                                 const glm::vec3& p2, const glm::vec3& p3, 
                                 float t, float alpha = 0.5f);
    
    /**
     * Smooth step ease function for transition smoothing.
     * @param t Value in range [0, 1]
     * @return Smoothed value using Hermite interpolation
     */
    static float Smoothstep(float t);
    
    /**
     * Find which keyframe segment the current time falls into.
     * @param elapsed Time in seconds
     * @param outLocalT Local time within segment [0, 1]
     * @return Index of first keyframe in the segment
     */
    int FindSegment(float elapsed, float& outLocalT) const;
};

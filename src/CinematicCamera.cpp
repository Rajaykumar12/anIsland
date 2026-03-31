#include "CinematicCamera.h"
#include <cmath>
#include <iostream>

CinematicCamera::CinematicCamera(const std::vector<float>& heightmap, int hmW, int hmH,
                                 float terrW, float terrD)
    : Camera(glm::vec3(400.0f, 120.0f, 400.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -20.0f),
      heightmapData(heightmap),
      hmWidth(hmW), hmHeight(hmH),
      terrainWidth(terrW), terrainDepth(terrD),
      currentTime(0.0f),
      isActive(false),
      userControlEnabled(true),
      currentShotIndex(0)
{
    hmWorldScaleX = terrainWidth / (hmWidth - 1);
    hmWorldScaleZ = terrainDepth / (hmHeight - 1);

    InitializeShots();
}

void CinematicCamera::InitializeShots() {
    // Total duration: ~6 minutes = 360 seconds for fuller experience
    // Time of day progression: 0.0=midnight, 0.25=dawn, 0.5=noon, 0.75=dusk, 1.0=midnight

    // === SHOT 1: "The Sleeping Island" - Wide night establishing shot (0:00 - 0:30) ===
    shots.push_back({
        0.0f, 30.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            // Start with wide aerial view of island at night
            float smoothT = t / 30.0f;
            smoothT = smoothT * smoothT * (3.0f - 2.0f * smoothT);

            // High altitude looking down at dark island
            pos = glm::vec3(
                Lerp(300.0f, 500.0f, smoothT),  // Slow drift
                180.0f,                          // High altitude
                300.0f
            );

            yaw = -45.0f + smoothT * 20.0f;
            pitch = -55.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "The Sleeping Island"
    });

    // === SHOT 2: "Approaching the Summit" - Descend toward peak (0:30 - 1:00) ===
    shots.push_back({
        30.0f, 30.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 30.0f;
            smoothT = smoothT * smoothT * (3.0f - 2.0f * smoothT);

            // Descend from high altitude to summit
            glm::vec3 startPos(450.0f, 180.0f, 350.0f);
            glm::vec3 endPos(350.0f, 110.0f, 350.0f);  // Just above peak

            pos = Lerp(startPos, endPos, smoothT);

            // Look at mountain peak
            yaw = -45.0f;
            pitch = Lerp(-55.0f, -20.0f, smoothT);

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "Approaching the Summit"
    });

    // === SHOT 3: "The First Light" - Sunrise on eastern horizon (1:00 - 1:45) ===
    shots.push_back({
        60.0f, 45.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            // Watch sunrise from eastern side of peak
            float smoothT = t / 45.0f;

            pos = glm::vec3(320.0f, 100.0f, 400.0f);  // East of peak

            // Start looking at dark horizon, gradually turn to rising sun
            yaw = Lerp(60.0f, 90.0f, smoothT);  // Turn toward east/sunrise
            pitch = -10.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "The First Light"
    });

    // === SHOT 4: "Orbital Awakening" - Wide orbit showing full island (1:45 - 2:45) ===
    shots.push_back({
        105.0f, 60.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            // Wide orbit as sun rises - shows entire island geography
            float orbitProgress = t / 60.0f;
            float angle = orbitProgress * 4.712f; // 270 degrees of rotation

            float centerX = 400.0f;
            float centerZ = 400.0f;
            float radius = 200.0f;  // Wide radius to see full island
            float height = 140.0f;   // High enough to see everything

            pos.x = centerX + cos(angle) * radius;
            pos.z = centerZ + sin(angle) * radius;
            pos.y = height;

            // Always look at island center
            float dx = centerX - pos.x;
            float dz = centerZ - pos.z;
            yaw = atan2(dz, dx) * 57.2958f;
            pitch = -35.0f;

            front = glm::normalize(glm::vec3(dx, -50.0f, dz));
        },
        "Orbital Awakening"
    });

    // === SHOT 5: "Ridge to Valley Glide" - Wide descent away from summit (2:45 - 3:30) ===
    shots.push_back({
        165.0f, 45.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 45.0f;
            smoothT = smoothT * smoothT * (3.0f - 2.0f * smoothT);

            // Leave the summit area with a broad glide toward the valley.
            glm::vec3 startPos(600.0f, 140.0f, 400.0f);
            glm::vec3 endPos(470.0f, 70.0f, 330.0f);

            pos = Lerp(startPos, endPos, smoothT);

            yaw = Lerp(-145.0f, -130.0f, smoothT);
            pitch = Lerp(-35.0f, -18.0f, smoothT);

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "Ridge to Valley Glide"
    });

    // === SHOT 6: "Valley Panorama" - Wide lateral pass with mountain context (3:30 - 4:15) ===
    shots.push_back({
        210.0f, 45.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float moveT = t / 45.0f;
            float smoothT = moveT * moveT * (3.0f - 2.0f * moveT);

            // Broad side pass to reduce summit lock-in and expose island scale.
            pos = glm::vec3(
                Lerp(470.0f, 340.0f, smoothT),
                72.0f,
                Lerp(330.0f, 520.0f, smoothT)
            );

            yaw = Lerp(-130.0f, -65.0f, smoothT);
            pitch = -20.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "Valley Panorama"
    });

    // === SHOT 7: "Sunline Interlude" - Mid-sequence direct sunrise callback (4:15 - 4:45) ===
    shots.push_back({
        255.0f, 30.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float riseT = t / 30.0f;
            float smoothT = riseT * riseT * (3.0f - 2.0f * riseT);

            // Hold a direct look to the sunrise line, then ease toward grassland framing.
            glm::vec3 startPos(340.0f, 72.0f, 520.0f);
            glm::vec3 endPos(300.0f, 20.0f, 400.0f);
            pos = Lerp(startPos, endPos, smoothT);

            if (riseT < 0.65f) {
                float holdT = riseT / 0.65f;
                yaw = Lerp(82.0f, 96.0f, holdT);
                pitch = Lerp(-10.0f, -2.0f, holdT);
            } else {
                float outT = (riseT - 0.65f) / 0.35f;
                yaw = Lerp(96.0f, 180.0f, outT);
                pitch = Lerp(-2.0f, -8.0f, outT);
            }

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "Sunline Interlude"
    });

    // === SHOT 8: "Ocean of Grass" - Wide grassland shot (4:45 - 5:15) ===
    shots.push_back({
        285.0f, 30.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 30.0f;

            // Low over grasslands, moving sideways to show wind waves
            pos = glm::vec3(
                Lerp(300.0f, 500.0f, smoothT),
                12.0f,  // Just above grass
                400.0f
            );

            // Look across grass toward mountain
            yaw = 180.0f;
            pitch = -8.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "Ocean of Grass"
    });

    // === SHOT 9: "The Shoreline" - Water's edge (5:15 - 5:45) ===
    shots.push_back({
        315.0f, 30.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 30.0f;
            smoothT = smoothT * smoothT * (3.0f - 2.0f * smoothT);

            // Move to shoreline at water level
            glm::vec3 startPos(500.0f, 12.0f, 400.0f);
            glm::vec3 endPos(550.0f, 3.0f, 420.0f);  // Near water

            pos = Lerp(startPos, endPos, smoothT);

            // Look along coast
            yaw = Lerp(180.0f, -120.0f, smoothT);
            pitch = -5.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "The Shoreline"
    });

    // === SHOT 10: "Dusk Arrives" - Sunset on western horizon (5:45 - 6:30) ===
    shots.push_back({
        345.0f, 45.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 45.0f;

            // Position on western side looking toward sunset
            pos = glm::vec3(550.0f, 50.0f, 400.0f);

            // Turn to face west (sunset)
            yaw = Lerp(-120.0f, -90.0f, smoothT);
            pitch = -15.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "Dusk Arrives"
    });

    // === SHOT 11: "The Moon Rises" - Night with moon visible (6:30 - 7:15) ===
    shots.push_back({
        390.0f, 45.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 45.0f;

            // Wide shot showing moon rising opposite to where sun set
            pos = glm::vec3(400.0f, 80.0f, 250.0f);

            // Rotate to find moon in sky
            yaw = Lerp(-90.0f, 90.0f, smoothT);  // Full sweep to find moon
            pitch = -20.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "The Moon Rises"
    });

    // === SHOT 12: "Firefly Dance" - Move through fireflies at forest edge (7:15 - 7:45) ===
    shots.push_back({
        435.0f, 30.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 30.0f;
            smoothT = smoothT * smoothT * (3.0f - 2.0f * smoothT);

            // Move through forest edge where fireflies appear
            glm::vec3 startPos(350.0f, 25.0f, 350.0f);
            glm::vec3 endPos(330.0f, 20.0f, 370.0f);

            pos = Lerp(startPos, endPos, smoothT);

            // Gentle rotation showing fireflies at different depths
            yaw = Lerp(-135.0f, -45.0f, smoothT);
            pitch = -5.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "Firefly Dance"
    });

    // === SHOT 13: "The Island at Night" - Final wide shot (7:45 - 8:30) ===
    shots.push_back({
        465.0f, 45.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 45.0f;
            smoothT = smoothT * smoothT * (3.0f - 2.0f * smoothT);

            // Rise to high altitude for final wide shot
            glm::vec3 startPos(350.0f, 30.0f, 380.0f);
            glm::vec3 endPos(400.0f, 200.0f, 400.0f);  // Maximum altitude

            pos = Lerp(startPos, endPos, smoothT);

            // Final rotation showing full island
            yaw = Lerp(-45.0f, -90.0f, smoothT);
            pitch = Lerp(-15.0f, -50.0f, smoothT);  // Look down at island

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "The Island at Night"
    });

    // === SHOT 14: "Fade to Stars" - Final rotation before fade (8:30 - 8:45) ===
    shots.push_back({
        510.0f, 15.0f,
        [](float t, glm::vec3& pos, glm::vec3& front, float& yaw, float& pitch) {
            float smoothT = t / 15.0f;

            // Hold high position, slow rotation
            pos = glm::vec3(400.0f, 200.0f, 400.0f);

            yaw = Lerp(-90.0f, 0.0f, smoothT);
            pitch = -50.0f;

            front = glm::vec3(
                cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
                sin(glm::radians(pitch)),
                sin(glm::radians(yaw)) * cos(glm::radians(pitch))
            );
        },
        "Fade to Stars"
    });
}

float CinematicCamera::GetTerrainHeight(float worldX, float worldZ) const {
    float hmX = worldX / hmWorldScaleX;
    float hmZ = worldZ / hmWorldScaleZ;

    int x0 = (int)hmX;
    int z0 = (int)hmZ;
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    x0 = std::max(0, std::min(x0, hmWidth - 1));
    z0 = std::max(0, std::min(z0, hmHeight - 1));
    x1 = std::max(0, std::min(x1, hmWidth - 1));
    z1 = std::max(0, std::min(z1, hmHeight - 1));

    float fx = hmX - x0;
    float fz = hmZ - z0;

    float h00 = heightmapData[z0 * hmWidth + x0];
    float h10 = heightmapData[z0 * hmWidth + x1];
    float h01 = heightmapData[z1 * hmWidth + x0];
    float h11 = heightmapData[z1 * hmWidth + x1];

    float h0 = h00 * (1.0f - fx) + h10 * fx;
    float h1 = h01 * (1.0f - fx) + h11 * fx;

    return (h0 * (1.0f - fz) + h1 * fz) * 150.0f;
}

float CinematicCamera::GetTerrainHeightClamped(float worldX, float worldZ) const {
    if (worldX < 0.0f || worldX >= terrainWidth || worldZ < 0.0f || worldZ >= terrainDepth) {
        return -5.0f; // Water level
    }
    return GetTerrainHeight(worldX, worldZ);
}

float CinematicCamera::GetMinimumCameraHeight(float worldX, float worldZ) const {
    float terrainH = GetTerrainHeightClamped(worldX, worldZ);
    return terrainH + 3.0f; // 3 units clearance
}

void CinematicCamera::Update(float cinematicTime, float deltaTime) {
    if (!isActive) return;

    currentTime = cinematicTime;

    int shotIndex = FindCurrentShot(currentTime);
    if (shotIndex < 0 || shotIndex >= (int)shots.size()) {
        isActive = false;
        return;
    }

    currentShotIndex = shotIndex;
    const ShotSegment& shot = shots[shotIndex];
    float localTime = currentTime - shot.startTime;

    glm::vec3 shotPos, shotFront;
    float shotYaw, shotPitch;
    shot.updateFunc(localTime, shotPos, shotFront, shotYaw, shotPitch);

    // Apply terrain constraint - camera must stay above terrain
    float minHeight = GetMinimumCameraHeight(shotPos.x, shotPos.z);
    if (shotPos.y < minHeight) {
        shotPos.y = minHeight;
    }

    // Check forward terrain to prevent clipping
    for (int i = 1; i <= 5; ++i) {
        float checkDist = i * 8.0f;
        glm::vec3 checkPos = shotPos + shotFront * checkDist;
        float checkMinHeight = GetMinimumCameraHeight(checkPos.x, checkPos.z);
        if (shotPos.y < checkMinHeight - checkDist * 0.3f) {
            shotPos.y = std::max(shotPos.y, checkMinHeight - checkDist * 0.3f);
        }
    }

    Position = shotPos;
    Front = glm::normalize(shotFront);
    Yaw = shotYaw;
    Pitch = shotPitch;

    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void CinematicCamera::StartCinematic() {
    isActive = true;
    currentTime = 0.0f;
    userControlEnabled = false;
    std::cout << "[Cinematic] Started: The Island - A Day's Journey" << std::endl;
}

void CinematicCamera::StopCinematic() {
    isActive = false;
    userControlEnabled = true;
    std::cout << "[Cinematic] Stopped at time: " << currentTime << "s" << std::endl;
}

const char* CinematicCamera::GetCurrentShotName() const {
    if (currentShotIndex >= 0 && currentShotIndex < (int)shots.size()) {
        return shots[currentShotIndex].name;
    }
    return "None";
}

int CinematicCamera::FindCurrentShot(float time) const {
    for (int i = 0; i < (int)shots.size(); ++i) {
        if (time >= shots[i].startTime && time < shots[i].startTime + shots[i].duration) {
            return i;
        }
    }
    if (!shots.empty() && time >= shots.back().startTime + shots.back().duration) {
        return -1;
    }
    return 0;
}

glm::vec3 CinematicCamera::Lerp(const glm::vec3& a, const glm::vec3& b, float t) {
    return a * (1.0f - t) + b * t;
}

float CinematicCamera::Lerp(float a, float b, float t) {
    return a * (1.0f - t) + b * t;
}

float CinematicCamera::SmoothStep(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float CinematicCamera::EaseInOut(float t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
}

# Terrain Visualization - Architecture

## Overview

This project is a modular OpenGL 3.3 terrain renderer built around independent simulation/render systems.
The active application entry point is `main_new.cpp`.

Current render flow:

1. Depth pass (shadow map generation)
2. Color pass (terrain + environment systems with shadow sampling)
3. Skybox pass
4. Fade overlay pass (cinematic outro blend)

## Active Systems

### Terrain
- Files: `include/Terrain.h`, `src/Terrain.cpp`
- 800x800 terrain grid in world space
- Height-based coloring (grass -> dirt -> rock -> snow)
- Receives dynamic lighting, fog, and shadow map sampling

### LightingSystem
- Files: `include/LightingSystem.h`, `src/LightingSystem.cpp`
- Controls day/night sun orbit, fog values, sky-driving light values
- Owns shadow resources (depth FBO + depth texture)
- Exposes cinematic hooks:
  - `setTimeOfDay(float)`
  - `setTimeScale(float)`

### TreeSystem
- Files: `include/TreeSystem.h`, `src/TreeSystem.cpp`
- 15,000 instanced trees with terrain placement filters
- Wind sway in shader
- Rendered in depth pass and color pass (casts and receives shadows)

### GrassSystem
- Files: `include/GrassSystem.h`, `src/GrassSystem.cpp`
- Dense instanced grass aligned to terrain normals
- Slope and elevation filtering
- Wind motion with stability clamping in shader

### WaterSystem
- Files: `include/WaterSystem.h`, `src/WaterSystem.cpp`
- Procedural wave displacement
- Specular/diffuse lighting response tied to dynamic sun

### ParticleSystem (Fireflies)
- Files: `include/ParticleSystem.h`, `src/ParticleSystem.cpp`
- 500 particles constrained to forest elevation bands
- Terrain-sampled altitude, additive blending, night-aware visibility

### RainSystem
- Files: `include/RainSystem.h`, `src/RainSystem.cpp`
- Camera-centered rain particle field
- Runtime toggle with `R`

### SplashSystem
- Files: `include/SplashSystem.h`, `src/SplashSystem.cpp`
- Terrain-aware rain impact splashes
- Enabled/updated alongside rain state

### CinematicSystem
- Files: `include/CinematicSystem.h`, `src/CinematicSystem.cpp`
- Camera timeline with keyframes and segment types:
  - `SPLINE`
  - `ORBIT`
  - `FREEZE`
- Drives camera transform + FOV and coordinates lighting pace
- Supports forced time-of-day anchors and dynamic lighting speed per shot
- Timeline constants:
  - total duration: 300s
  - fade starts near end of timeline
- Current startup behavior: timeline resets to `t=70s` (sunrise opening)

## Camera And Control Model

### Camera
- Files: `include/Camera.h`, `src/Camera.cpp`
- FPS-style movement and look controls in manual mode
- Cinematic mode writes directly to camera pose and orientation

### Input Behavior
- App starts with `g_cinematicMode = true`
- During cinematic:
  - mouse/scroll callbacks are ignored
  - `ESC` exits cinematic to manual mode
- In manual mode:
  - `W/A/S/D`, `Q/E`, mouse look enabled
  - `F` toggles wireframe
  - `R` toggles rain

## Rendering Pipeline Details

### Pass 1: Shadow Depth Pass
- Viewport set to `2048x2048`
- Render target: `LightingSystem` depth FBO
- Shader: `assets/shaders/depth.vert` + `assets/shaders/depth.frag`
- Scene contributors: terrain and trees

### Pass 2: Main Color Pass
- Viewport restored to framebuffer size
- Clears with current sky color from lighting
- Draw order:
  1. Terrain
  2. Grass
  3. Trees
  4. Water
  5. Fireflies
  6. Rain
  7. Splashes
- Shadow depth texture bound for terrain shading

### Pass 3: Skybox
- Rendered last with `GL_LEQUAL`
- Procedural sky shader reads current sun position

### Pass 4: Fade Overlay
- Fullscreen quad
- Shader: `assets/shaders/fade.vert` + `assets/shaders/fade.frag`
- Enabled when cinematic fade alpha > 0

## Build Composition

`CMakeLists.txt` builds these active modules:

- `main_new.cpp`
- `src/Terrain.cpp`
- `src/Camera.cpp`
- `src/NoiseMap.cpp`
- `src/Shader.cpp`
- `src/TreeSystem.cpp`
- `src/ParticleSystem.cpp`
- `src/WaterSystem.cpp`
- `src/GrassSystem.cpp`
- `src/LightingSystem.cpp`
- `src/RainSystem.cpp`
- `src/SplashSystem.cpp`
- `src/CinematicSystem.cpp`
- `glad.c`

Assets are copied to the build directory post-build.

## Removed Components

The following systems were removed and are not part of the current runtime/build:

- BuildingSystem
- NPCSystem
- ColonySystem

Likewise, legacy shaders tied to those systems were removed from the active project flow.

## Notes For Extension

To add a new system:

1. Add header under `include/` and implementation under `src/`
2. Register source in `CMakeLists.txt`
3. Instantiate/update/render from `main_new.cpp`
4. If the system should cast shadows, render it in depth pass before unbinding the depth FBO

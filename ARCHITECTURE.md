# Terrain Visualization Architecture

## Overview

The project uses a modular C++ architecture with dedicated systems for terrain, vegetation, water, particles, weather, lighting, and a timeline-based cinematic controller.

Entry point: `main_new.cpp`

## Active Runtime Systems

- `Terrain`
  - Generates and draws the 800x800 terrain mesh.
  - Uses a 512x512 procedural noise heightmap.

- `TreeSystem`
  - Renders 15,000 instanced trees.
  - Placement filtered by terrain shape.

- `GrassSystem`
  - Dense instanced grass aligned to terrain normals.
  - Wind sway uses time + strength envelope.

- `WaterSystem`
  - Procedural wave displacement and color depth response.

- `ParticleSystem`
  - 500 fireflies with terrain-aware placement.
  - Constrained to forest elevation range.

- `RainSystem`
  - Toggleable rain particle field.

- `SplashSystem`
  - Terrain-aware splash particles during rain.

- `LightingSystem`
  - Sun orbit, sky color, fog color, sun intensity.
  - Shadow framebuffer + light-space matrices.
  - Supports manual sun override for cinematic mode.

- `CinematicController`
  - 300-second timeline for "The Island at Dawn".
  - Produces per-frame cinematic state:
    - camera position/target/zoom
    - sun position
    - wind strength
    - fade color/alpha
    - firefly visibility boost

## Render Flow (Per Frame)

1. Input + timing
   - Handle keys (`C`, `R`, `F`, movement)
   - Update cinematic timeline if active

2. Camera and sun application
   - If cinematic active:
     - Apply timeline camera pose
     - Apply terrain safety clamp to camera and target heights
     - Apply manual sun position override
   - Else:
     - Use free camera controls
     - Use automatic day/night sun update

3. Shadow depth pass
   - Render depth from light POV to 2048x2048 depth texture

4. Main color pass
   - Render terrain
   - Render grass, trees, water
   - Render fireflies, rain, and splashes

5. Skybox pass
   - Procedural sky and stars

6. Fade overlay pass (cinematic only)
   - Fullscreen color+alpha fade for ending shot

## Two-Pass Lighting Pipeline

- Depth pass shader: `assets/shaders/depth.vert`, `assets/shaders/depth.frag`
- Color pass sampling:
  - Shadow map bound to texture unit 1
  - Terrain and scene shaders use light direction/color from `LightingSystem`

## Cinematic Design Notes

The cinematic follows a single-shot arc across seven acts:

1. Summit pre-dawn hold/pan
2. Summit orbit into sunrise
3. Descent along mountain shoulder
4. Forest traversal and canopy rise
5. Grassland + shoreline pass
6. Dusk transition with fireflies
7. Final night wide rise and fade

Implementation details:
- Duration: `300s`
- Camera remains terrain-safe with per-frame clearance clamp
- Final act includes boosted firefly readability for wide night composition

## Shader Responsibilities

- `terrain.*`: terrain shading + shadows + fog
- `tree.*`: instanced tree shading + wind
- `grass.*`: grass sway + lighting
- `water.*`: wave displacement + light response
- `particle.*`: firefly point rendering, day/night gating, cinematic boost
- `rain.*`: rain particle rendering
- `splash.*`: splash points at sampled terrain heights
- `skybox.*`: procedural sky/stars with time-driven behavior
- `fade.*`: fullscreen cinematic fade

## Data Dependencies

- Heightmap noise drives:
  - terrain geometry
  - firefly spawn heights
  - camera terrain clearance checks
  - splash surface placement

- Lighting outputs drive:
  - scene light direction and color
  - fog tint/intensity behavior
  - firefly day/night visibility

## Build Wiring

`CMakeLists.txt` compiles these modules:
- `main_new.cpp`
- all system `.cpp` files in `src/`
- `src/CinematicController.cpp`
- `glad.c`

Assets are copied to build output via CMake post-build step.

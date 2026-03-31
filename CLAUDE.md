# CLAUDE.md

This file provides guidance when working on this repository.

## Build Commands

```bash
cd build
cmake ..
make
./OpenGLTerrainInstancing
```

## Runtime Controls

- `W/A/S/D`, `Q/E`, mouse: free camera movement
- `C`: toggle cinematic mode (start/stop/reset)
- `R`: toggle rain
- `F`: toggle wireframe
- `ESC`: exit

When cinematic mode is active (or finished), manual camera input is locked.

## Architecture Summary

### Core Render Pipeline

1. **Depth Pass**
   - Render from light perspective into 2048x2048 depth texture.
2. **Color Pass**
   - Render scene from camera and sample shadow map.
3. **Skybox Pass**
   - Procedural sky and stars.
4. **Fade Pass**
   - Fullscreen cinematic fade in final shot.

### Active Systems (in `src/` with headers in `include/`)

- `Terrain`
- `TreeSystem`
- `GrassSystem`
- `WaterSystem`
- `ParticleSystem` (fireflies)
- `RainSystem`
- `SplashSystem`
- `LightingSystem`
- `CinematicController`

### Cinematic System

`CinematicController` drives a 300-second sequence and outputs:
- camera position/target
- camera zoom
- sun position override
- wind strength
- fade color/alpha
- firefly boost for late-night wide shot visibility

Main integration points:
- `main_new.cpp` applies cinematic frame values every update when active.
- Terrain safety clamping prevents camera/target from dropping below terrain.
- `LightingSystem` supports manual sun override for cinematic beats.

## Key Technical Facts

- Terrain mesh: `800x800`
- Heightmap: `512x512` procedural noise
- Camera spawn: `(125, 200, 175)`
- Shadow map: `2048x2048`
- Day cycle speed (free-run): ~63s per cycle
- Fireflies: 500 particles in forest elevation band `8..85`
- Fog model: `exp(-(distance * density)^2)`

## Shader Map (`assets/shaders/`)

- `depth.vert/.frag`: shadow map generation
- `terrain.vert/.frag`: terrain lighting, fog, shadows
- `tree.vert/.frag`: tree instancing + wind
- `grass.vert/.frag`: grass instancing + wind
- `water.vert/.frag`: wave shading
- `particle.vert/.frag`: fireflies (day/night + cinematic boost)
- `rain.vert/.frag`: rain particles
- `splash.vert/.frag`: splash particles
- `skybox.vert/.frag`: procedural sky and stars
- `fade.vert/.frag`: cinematic fullscreen fade

## Extension Workflow

To add a new system:
1. Add `include/YourSystem.h` and `src/YourSystem.cpp`.
2. Register `src/YourSystem.cpp` in `CMakeLists.txt` sources.
3. Instantiate and update/render from `main_new.cpp`.
4. If it casts shadows, include it in depth pass before unbinding the shadow FBO.

## Recent Session Changes

- Added `CinematicController` and integrated 5-minute timeline sequence.
- Added cinematic camera locking and `C` key toggle flow.
- Added terrain-aware camera/target safety clamp for cinematic shots.
- Added manual sun override hooks in `LightingSystem` for timeline lighting.
- Added final fullscreen fade pass (`fade.vert/.frag`).
- Added firefly visibility boosting in late cinematic night shot.

## Troubleshooting

- **Black screen**
  - Verify shader assets are copied into `build/assets/shaders`.
- **No shadows**
  - Confirm depth pass executes before color pass.
- **Fireflies not visible at night**
  - Check `dayIntensity` progression and particle `fireflyBoost` uniforms.
- **Cinematic camera clipping terrain**
  - Verify terrain height sampling and clearance clamp path in `main_new.cpp`.

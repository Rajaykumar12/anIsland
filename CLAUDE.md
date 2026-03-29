# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
cd build
cmake ..
make
./OpenGLTerrainInstancing
```

## Architecture

**Two-Pass Rendering Pipeline:**
1. **Depth Pass** - Renders scene from light's perspective to 2048×2048 depth texture (shadow map)
2. **Color Pass** - Renders scene from camera, samples shadow map for PCF-filtered directional shadows

**Core Systems** (all in `src/` with headers in `include/`):
- `LightingSystem` - Day/night cycle (63s orbit), sun color/intensity, fog parameters, shadow FBO
- `Terrain` - 800×800 vertex grid with Perlin noise heightmap, height-based color bands
- `TreeSystem` - 15,000 instanced trees placed by terrain height/slope
- `GrassSystem` - Dense instanced grass with terrain-normal alignment, wind animation, slope filtering
- `ParticleSystem` - 500 fireflies terrain-sampled, constrained to forest elevation bands (8-85 world units)
- `WaterSystem` - 500×500 wave mesh at Y ≈ -1.5 with procedural sine displacement
- `RainSystem` - Toggleable (R key) rain particles, camera-centered
- `SplashSystem` - Rain impact splashes terrain-aware, rendered at surface level during rain
- `NPCSystem` - Static cinematic anchor NPCs with timeline visibility windows and pose transforms
- `CinematicSystem` - 300s keyframed story timeline (camera, FOV, time-of-day, fade) with smooth interpolation and mountain-focused opening/ending shots

**Disabled Systems**:
- None

**Key Shader Files** (`assets/shaders/`):
- `depth.vert/frag` - Shadow map generation (light space transform)
- `terrain.vert/frag` - Terrain with shadow sampling, fog, Phong lighting
- `grass.vert/frag` - Terrain-normal-aligned grass with wind sway animation
- `tree.vert/frag` - Instanced trees with wind effect
- `particle.vert/frag` - Firefly rendering with glow
- `water.vert/frag` - Procedural wave displacement
- `rain.vert/frag` - Rain particle rendering
- `splash.vert/frag` - Terrain-aware splash particles
- `skybox.vert/frag` - Procedural sky with stars, sunset glow, day/night color
- `person.vert/frag` - Cinematic NPC rendering
- `fade.vert/frag` - Fullscreen fade-to-black overlay

## Key Implementation Details

- **Camera spawn**: (125, 200, 175)
- **Terrain size**: 800×800 vertices spanning 800×800 world units
- **Heightmap**: 512×512 Perlin noise with fractal Brownian motion
- **Shadow map**: 2048×2048, orthographic projection -150 to +150 units
- **Fog formula**: `exp(-(distance * density)^2)` - exponential squared
- **Fireflies**: Terrain-sampled height, constrained to forest band (8-85 world units), GL_POINTS with additive blending, spawn/update relocates out-of-band particles
- **Grass**: Terrain world space [0..800], blade orientation via terrain normals (tangent/bitangent basis), wind animation with quadratic sway falloff, clamped for stability, filtered at slope > 55°
- **Splashes**: Terrain height-aware spawning, positioned relative to rain camera, GL_POINTS with lifetime-scaled size, disabled depth write

## Adding a New System

1. Create `include/YourSystem.h` and `src/YourSystem.cpp`
2. Add `src/YourSystem.cpp` to `SOURCES` in CMakeLists.txt
3. Include header in `main_new.cpp`, instantiate, call render() in main loop
4. For shadow casting: render in depth pass before shadow FBO unbind

## Recent Improvements (Latest Session)

**Completed Features:**
- Fixed grass coordinate frame alignment to world space [0..800] × [0..800]
- Implemented terrain-normal-aligned grass blade orientation (tangent/bitangent basis)
- Added wind animation with quadratic falloff and clamped height factor for stability
- Implemented terrain height sampling in ParticleSystem for firefly placement
- Constrained fireflies to forest elevation band (8-85 world units) with retry spawning
- Completed SplashSystem: terrain-aware spawn positions, integrated rendering in main loop
- Re-enabled NPCSystem for cinematic anchor storytelling
- Removed the legacy settlement system entirely from the project
- Removed BuildingSystem and building shader assets from the project
- Implemented CinematicSystem 8-act timeline with camera/FOV/time-of-day/fade keyframes
- Stabilized cinematic camera orientation by recomputing Front/Right/Up each frame with near-vertical fallback
- Simplified cinematic keyframe density to reduce jitter and improve mountain readability
- Added cinematic controls: C toggle, P play/pause, [ ] seek, , . act jumps, B restart

## Troubleshooting

- **Black screen**: Check `assets/shaders/` exists in build output (auto-copied by CMake)
- **No shadows**: Verify depth pass renders before color pass; shadow texture bound to GL_TEXTURE1
- **Fireflies underground**: Ensure `ParticleSystem::init()` samples terrain heightmap correctly
- **Grass not visible**: Verify grass coordinate frame [0..800]; check heightmap is loaded in GrassSystem
- **Splashes in air**: Ensure SplashSystem constructor receives correct heightmap and terrain dimensions
- **Cinematic tilt/jitter**: Verify cinematic camera path in `main_new.cpp` recomputes basis vectors from target direction each frame

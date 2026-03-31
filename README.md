# OpenGL Terrain Visualization

A modern C++/OpenGL terrain project with procedural generation, dynamic lighting, shadow mapping, weather effects, and a fully scripted 5-minute cinematic sequence.

## Highlights

- Procedural terrain (800x800 grid) from Perlin/fBm noise (512x512 heightmap)
- Two-pass shadow mapping (2048x2048 depth map + PCF sampling)
- Dynamic day/night lighting with sky, fog, and sun color transitions
- Instanced environment rendering:
  - 15,000 trees
  - Dense terrain-aligned grass
  - 500 fireflies (terrain-sampled, forest-band constrained)
- Water surface with procedural wave displacement
- Rain + terrain-aware splashes
- Scripted cinematic: "The Island at Dawn" (single-shot, 300 seconds)

## Build

```bash
cd build
cmake ..
make
./OpenGLTerrainInstancing
```

## Controls

| Key | Action |
|-----|--------|
| W / A / S / D | Move camera |
| Q / E | Move up / down |
| Mouse | Look around |
| C | Toggle cinematic mode (start/stop/reset) |
| R | Toggle rain |
| F | Toggle wireframe |
| ESC | Exit |

Notes:
- While cinematic mode is active (or completed), manual camera input is locked.
- Cinematic camera automatically keeps safe clearance above terrain.

## Cinematic: The Island at Dawn

The runtime includes a timeline-driven cinematic controller (`300s`) that drives:
- Camera position and target
- Camera zoom
- Sun position override (for dawn -> day -> dusk -> night choreography)
- Wind intensity envelope
- Firefly visibility boost in late-night wide shot
- Final deep-blue-to-black fade

Main cinematic phases:
1. Summit pre-dawn hold and pan
2. Slow summit orbit through sunrise
3. Mountain descent (shoulder path, no under-mountain framing)
4. Forest traversal and canopy rise
5. Grasslands and shoreline water pass
6. Dusk transition and firefly reveal
7. Final wide rise over island and ocean with night fade-out

## Rendering Pipeline

1. Depth pass from light POV into shadow map
2. Color pass from camera POV with shadow sampling
3. Skybox pass (procedural sky and stars)
4. Cinematic fade overlay pass (when active)

## Project Layout

```text
CGAssignment/
├── CMakeLists.txt
├── main_new.cpp
├── README.md
├── ARCHITECTURE.md
├── CLAUDE.md
├── include/
│   ├── Camera.h
│   ├── CinematicController.h
│   ├── GrassSystem.h
│   ├── LightingSystem.h
│   ├── NoiseMap.h
│   ├── ParticleSystem.h
│   ├── RainSystem.h
│   ├── Shader.h
│   ├── SplashSystem.h
│   ├── Terrain.h
│   ├── TreeSystem.h
│   └── WaterSystem.h
├── src/
│   ├── Camera.cpp
│   ├── CinematicController.cpp
│   ├── GrassSystem.cpp
│   ├── LightingSystem.cpp
│   ├── NoiseMap.cpp
│   ├── ParticleSystem.cpp
│   ├── RainSystem.cpp
│   ├── Shader.cpp
│   ├── SplashSystem.cpp
│   ├── Terrain.cpp
│   ├── TreeSystem.cpp
│   └── WaterSystem.cpp
└── assets/shaders/
    ├── depth.vert/.frag
    ├── fade.vert/.frag
    ├── grass.vert/.frag
    ├── particle.vert/.frag
    ├── rain.vert/.frag
    ├── skybox.vert/.frag
    ├── splash.vert/.frag
    ├── terrain.vert/.frag
    ├── tree.vert/.frag
    └── water.vert/.frag
```

## Technical Notes

- Terrain world span: `800 x 800`
- Camera spawn: `(125, 200, 175)`
- Shadow map: `2048 x 2048`
- Day speed (default free-run): one cycle ~63 seconds
- Firefly placement band: elevation `8..85`
- Fog model: exponential squared (`exp(-(distance * density)^2)`)

## Troubleshooting

- Black screen:
  - Ensure `assets/shaders` is copied to `build/assets/shaders`
  - Check OpenGL 3.3+ support
- No shadows:
  - Verify depth pass runs before color pass
  - Confirm shadow map is bound to texture unit 1
- Camera clipping terrain during cinematic:
  - Confirm cinematic terrain safety clamp is enabled in `main_new.cpp`
- Fireflies not visible in late shot:
  - Check cinematic night progression and `fireflyBoost` uniforms in particle shader path

## License

Developed as a Computer Graphics assignment using GLFW, GLM, and GLAD.

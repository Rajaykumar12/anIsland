# OpenGL Terrain Visualization

A modern C++ OpenGL terrain project with a cinematic intro sequence, dynamic day/night lighting, PCF shadows, fog, and multiple instanced environment systems.

## Current Highlights

- Procedural 800x800 terrain from a 512x512 Perlin fBm heightmap
- Two-pass shadow pipeline (2048x2048 depth map + color pass sampling)
- Cinematic camera timeline with spline/orbit/freeze segments and fade-out
- Dynamic skybox driven by sun position and day intensity
- 15,000 instanced trees with wind animation
- Dense terrain-aligned grass with slope filtering
- 500 fireflies constrained to forest elevation bands
- Toggleable rain with terrain-aware splash particles
- Procedural water surface with wave displacement

## Removed Systems

These systems were removed from the project and are no longer built or rendered:

- Building system
- NPC system
- Colony system

## Build Requirements

- C++17+
- CMake 3.10+
- OpenGL 3.3+
- GLFW3
- GLM
- GLAD (included via source)

## Build And Run

```bash
cd build
cmake ..
make
./OpenGLTerrainInstancing
```

## Controls

| Key | Action |
|-----|--------|
| W/A/S/D | Move camera (manual mode) |
| Q/E | Move up/down (manual mode) |
| Mouse | Look around (manual mode) |
| R | Toggle rain |
| F | Toggle wireframe |
| ESC | Exit app, or skip cinematic and enter manual mode |

## Runtime Flow

On launch, the app starts in cinematic mode (`g_cinematicMode = true`) and drives camera and lighting from `CinematicSystem`.

- Cinematic duration: 300s total timeline
- Current cinematic start point: 70s (sunrise sequence)
- Fade overlay enabled near the end of the timeline
- Pressing ESC during cinematic exits to manual camera control

## Project Layout

```text
CGAssignment/
	CMakeLists.txt
	main_new.cpp
	include/
		Camera.h
		CinematicSystem.h
		GrassSystem.h
		LightingSystem.h
		NoiseMap.h
		ParticleSystem.h
		RainSystem.h
		Shader.h
		SplashSystem.h
		Terrain.h
		TreeSystem.h
		WaterSystem.h
	src/
		Camera.cpp
		CinematicSystem.cpp
		GrassSystem.cpp
		LightingSystem.cpp
		NoiseMap.cpp
		ParticleSystem.cpp
		RainSystem.cpp
		Shader.cpp
		SplashSystem.cpp
		Terrain.cpp
		TreeSystem.cpp
		WaterSystem.cpp
	assets/shaders/
		depth.vert/.frag
		fade.vert/.frag
		grass.vert/.frag
		particle.vert/.frag
		rain.vert/.frag
		skybox.vert/.frag
		splash.vert/.frag
		terrain.vert/.frag
		tree.vert/.frag
		water.vert/.frag
```

## Notes

- Shader assets are copied from `assets/` into the build directory by CMake post-build.
- Default camera spawn in manual mode: `(125, 200, 175)`.
- Terrain world span: `800 x 800` world units.
- Day/night cycle base speed: approximately one orbit every 63 seconds at normal time scale.

## Troubleshooting

### Build fails

- Verify toolchain: `cmake --version`
- Check GLFW install: `pkg-config --modversion glfw3`
- Reconfigure cleanly from `build/` if cache is stale.

### Black screen or missing visuals

- Ensure `assets/shaders` exists in the build output.
- Confirm OpenGL 3.3 support on your GPU/driver.
- Verify depth pass runs before color pass in `main_new.cpp`.

### Scene looks too heavy

- Reduce counts in the corresponding system constructors in `main_new.cpp` (trees, rain particles, splashes, fireflies).
- Lower shadow map resolution in lighting system setup if needed.

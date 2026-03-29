# OpenGL Terrain Visualization

A modern C++ OpenGL application featuring a procedurally generated terrain with dynamic lighting, advanced visual effects, and a modular system architecture. Includes day/night cycles, shadow mapping, volumetric fog, forest-confined firefly particles, and interactive environmental elements.

## Features

### Rendering & Graphics
- **Procedural Terrain** - 800×800 vertex grid with fractal Brownian motion (Perlin noise)
- **Shadow Mapping** - Real-time directional shadows with PCF filtering (2048×2048 depth map)
- **Volumetric Fog** - Exponential squared fog dynamically tied to day/night cycle
- **Dynamic Procedural Skybox** - Multi-octave stars with twinkling, sunset glow, and atmospheric effects
- **Height-based Terrain Coloring** - Dynamic color bands (grass → dirt → rock → snow)

### Simulation Systems
- **15,000 Instanced Trees** - Intelligent placement based on terrain height/slope with Phong lighting
- **Procedural Grass** - Dense grass on mountain slopes with terrain-normal alignment, wind sway animation (quadratic falloff with stability clamping)
- **Dynamic Water + Coastline** - Procedural wave surface with irregular island shoreline and beach transition
- **Firefly Particles** - 500 glowing particles with terrain-sampled height, constrained to forest elevation bands (8–85 world units)
- **Rain Splashes** - Terrain-aware splash particles rendered at surface level during rain events

### Lighting & Atmosphere
- **Day/Night Cycle** - Sun orbits every ~63 seconds with dynamic color interpolation
- **Real-time Shadows** - Two-pass rendering pipeline (depth pass + color pass with shadow sampling)
- **Fog Integration** - Color and intensity change with time of day
- **Directional Lighting** - With dynamic intensity (0.2 night → 1.0 day)

### Dynamic Features
- **Interactive FPS Camera** - WASD movement, mouse look, vertical flight
- **5-Minute Cinematic Story Mode** - 8-act camera timeline with static NPC anchor storytelling
- **Wireframe Toggle** - Real-time visual debugging
- **Optimized Rendering** - Instanced rendering for efficient batch processing
- **Modern OpenGL** - OpenGL 3.3 Core Profile with VAO/VBO/shader architecture

## Requirements

### Build Dependencies
- **C++17** or later
- **CMake 3.10+**
- **OpenGL 3.3+**
- **GLFW3** - Window and input management
- **GLM** - Graphics mathematics library
- **GLAD** - OpenGL function loader

### System Requirements
- Linux/macOS/Windows with modern GPU supporting OpenGL 3.3+
- ~100MB disk space for build artifacts
- Dedicated graphics card recommended for smooth performance

## Building

### Quick Start

```bash
# Navigate to project directory
cd CGAssignment

# Create and enter build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make

# Run the application
./OpenGLTerrainInstancing
```

### macOS with Homebrew
```bash
# Install dependencies (if not already installed)
brew install glfw glm

cd build
cmake ..
make
./OpenGLTerrainInstancing
```

### Ubuntu/Debian
```bash
# Install dependencies
sudo apt-get install libglfw3-dev libglm-dev

cd build
cmake ..
make
./OpenGLTerrainInstancing
```

## Controls

| Key | Action |
|-----|--------|
| **W/A/S/D** | Move forward/left/backward/right |
| **Q/E** | Move up/down (vertical flight) |
| **Mouse** | Look around (captured) |
| **C** | Toggle cinematic / free camera mode |
| **P** | Play/pause cinematic timeline |
| **[ / ]** | Seek cinematic timeline by -/+10s |
| **, / .** | Jump to previous/next act boundary |
| **B** | Restart cinematic timeline |
| **F** | Toggle wireframe mode |
| **R** | Toggle rain |
| **ESC** | Exit application |

## Project Structure

```
CGAssignment/
├── CMakeLists.txt              # Build configuration
├── ARCHITECTURE.md             # Detailed system documentation
├── main_new.cpp               # Main application entry point
├── glad.c                     # OpenGL loader implementation
├── include/
│   ├── Terrain.h              # Heightmap-based terrain
│   ├── Camera.h               # FPS camera controller
│   ├── Shader.h               # OpenGL shader utility
│   ├── NoiseMap.h             # Perlin noise generation
│   ├── TreeSystem.h           # 15,000 instanced trees (NEW)
│   ├── GrassSystem.h          # Static slope-aligned mountain grass (NEW)
│   ├── ParticleSystem.h       # Firefly particles (NEW)
│   ├── WaterSystem.h          # Dynamic water (NEW)
│   ├── LightingSystem.h       # Day/night cycle & shadows (NEW)
│   ├── RainSystem.h           # Rain simulation (NEW)
│   └── glad/glad.h            # OpenGL headers
├── src/
│   ├── [Corresponding .cpp implementations]
├── assets/shaders/
│   ├── terrain.vert/.frag     # Terrain rendering with shadows
│   ├── grass.vert/.frag       # Static slope-aligned grass
│   ├── tree.vert/.frag        # Instanced tree rendering
│   ├── particle.vert/.frag    # Firefly rendering
│   ├── water.vert/.frag       # Procedural water
│   ├── depth.vert/.frag       # Shadow map generation
│   ├── rain.vert/.frag        # Rain particles
│   └── skybox.vert/.frag      # Dynamic sky rendering
└── build/                      # CMake build directory
```

## Architecture

This project uses a **modular system-based architecture** for maintainability and scalability:

### Core Systems

- **Terrain System** - Procedurally generates heightmap using fractal Brownian motion
- **Lighting System** - Manages day/night cycle, shadow mapping, and fog parameters
- **Tree System** - Renders 15,000 trees with intelligent placement
- **Particle System** - Manages 500 forest-band firefly particles above terrain
- **Grass System** - Generates dense, static, slope-aligned grass across mountain surfaces
- **Water System** - Creates dynamic water surface with wave displacement
- **Rain System** - Rain particle simulation
- **NPC System** - Three static anchor NPC states (beach, valley, mountain) with timeline visibility windows
- **Cinematic System** - Keyframed camera/FOV/time-of-day/fade control over a 300-second narrative

### Rendering Pipeline

The application uses a **two-pass rendering approach**:

1. **Depth Pass** - Renders scene from light's perspective to 2048×2048 depth texture
2. **Color Pass** - Renders scene from camera with shadows applied via depth texture sampling

See [ARCHITECTURE.md](ARCHITECTURE.md) for detailed documentation of all systems.

## Visual Features

### Lighting Effects
- Dynamic sun orbit with realistic light attenuation
- PCF (Percentage Closer Filtering) for soft shadow edges
- Bias system to prevent shadow acne

### Atmospheric Effects
- Real-time volumetric fog with exponential squared formula
- Dynamic fog color transitioning between day (light blue) and night (dark blue)
- Procedural skybox with stars, planet, and atmospheric glow

### Terrain Visualization
- Height-based color banding: grass → dirt → rock → snow
- Normal mapping derived from heightmap slopes
- Phong lighting on all surfaces

## Performance Considerations

- **Instancing** - Trees and grass rendered efficiently in batches
- **Frustum Culling** - Optional optimization for large object counts
- **Level of Detail** - Terrain LOD can be adjusted via grid resolution
- **Shadow Type** - Directional shadows optimized for landscape rendering

## Development Notes

- **Shader Assets** - Located in `assets/shaders/`, automatically copied to build directory
- **Camera Position** - Default spawn: (125, 200, 175)
- **Terrain Size** - 800×800 vertices spanning 800×800 world units
- **Day Speed** - One full day/night cycle ≈ 63 seconds

## Troubleshooting

### Application won't build
- Verify CMake is installed: `cmake --version`
- Check GLFW3 installation: `pkg-config --modversion glfw3`
- Ensure GLM headers are in system path: `find /usr/include -name glm`

### Black screen on launch
- Verify `assets/shaders/` directory exists in build output
- Check OpenGL version support: `glxinfo | grep "OpenGL version"`
- Try updating GPU drivers

### Poor performance
- Reduce terrain grid resolution in [Terrain.h](include/Terrain.h)
- Lower shadow map resolution (2048 → 1024) in [LightingSystem.cpp](src/LightingSystem.cpp)
- Reduce particle count in [ParticleSystem.h](include/ParticleSystem.h)

## Further Reading

For detailed system architecture and implementation specifics, see [ARCHITECTURE.md](ARCHITECTURE.md).

## License

This project was developed as a Computer Graphics assignment. Uses GLAD for OpenGL loading and GLM for mathematics.

## Credits

- **GLFW** - Window management and input handling
- **GLM** - Graphics mathematics
- **GLAD** - OpenGL loader generator
- **Perlin Noise** - Procedural terrain generation algorithm

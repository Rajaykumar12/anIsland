# Terrain Visualization - Modular Architecture

## Project Structure

This project has been refactored from a monolithic `main.cpp` into a modular system architecture for better organization and maintainability.

### Core Systems

#### 1. **TreeSystem** (`include/TreeSystem.h`, `src/TreeSystem.cpp`)
- Manages 15,000 instanced trees
- Filters tree placement based on terrain height and slope
- Handles tree geometry (trunk + multi-level canopy)
- Renders with Phong lighting and dynamic sky color

#### 2. **GrassSystem** (`include/GrassSystem.h`, `src/GrassSystem.cpp`)
- Generates dense grass blade instances across mountain terrain
- Places grass at world coordinates [0..800] × [0..800], precisely aligned to terrain space
- Aligns each blade to local terrain normal via computed tangent/bitangent basis for slope-following orientation
- Wind animation: sinusoidal sway with quadratic falloff from blade base, clamped height factor [0,1] for stability
- Filters: excludes underwater (Y < 2.0), extreme slopes (> 55°), and terrain shadow regions
- Each blade is a tapered quad (2 triangles), tuned for finer and shorter appearance (baseWidth 0.09, tipWidth 0.018, height 0.62)
- Placement uses 0.68 world-unit spacing with dense 6-8 blade clumps per sample point for high-volume fields
- Uses efficient instanced rendering (single draw call)
- Dependencies: heightmap (512×512), terrain size (800×800)

#### 3. **BuildingSystem** (`include/BuildingSystem.h`, `src/BuildingSystem.cpp`)
- Creates a 4x4 grid of houses in the town center (100-150, 150-200)
- Each building has walls, roof (pyramid), and windows
- All geometry stored as triangles for GL_TRIANGLES rendering
- Instanced rendering for efficient batch processing

#### 4. **RainSystem** (`include/RainSystem.h`, `src/RainSystem.cpp`)
- Simulates large rain particle fields over the terrain
- Toggleable in runtime (R key)
- Integrates with camera-centered updates for coverage

#### 5. **ParticleSystem** (`include/ParticleSystem.h`, `src/ParticleSystem.cpp`)
- Manages 500 firefly particles with physics simulation
- **Terrain Sampling**: Bilinear heightmap lookup for precise above-ground placement; samples at normalized world coordinates [0..800]
- **Forest Confinement**: Constrains spawn and relocation to elevation band [8..85] world units (tree habitat range); up to 32 retry attempts on out-of-bounds spawn
- **Height Offset**: Hovers 5–10 units above local terrain via `y = terrainHeight + BASE_HOVER`; update relocates out-of-band particles
- **Fluttering**: Adds sinusoidal flutter with random phase and frequency while preserving elevation constraints
- Wraps particles within terrain bounds on X/Z drift
- Renders as GL_POINTS with additive blending; intensity fades with day/night lighting
- Dependencies: heightmap (512×512), terrain size (800×800)

#### 6. **WaterSystem** (`include/WaterSystem.h`, `src/WaterSystem.cpp`)
- Generates 500x500 grid water mesh
- Covers ±500 units around terrain center
- Water tuned near shoreline level for beach contact (current level near Y = -1.5)
- Uses procedural sine wave displacement for wave simulation
- Implements height-based color blending (deep blue to shallow cyan)

#### **Legacy Systems (Disabled in active main loop)**
- **NPCSystem** (`include/NPCSystem.h`, `src/NPCSystem.cpp`) is present in the codebase but not instantiated/rendered in `main_new.cpp`
- **ColonySystem** (`include/ColonySystem.h`, `src/ColonySystem.cpp`) is present in the codebase but not instantiated/rendered in `main_new.cpp`

#### 7. **SplashSystem** (`include/SplashSystem.h`, `src/SplashSystem.cpp`)
- Renders rain impact splashes on terrain surface during active rain
- **Terrain Sampling**: Normalizes camera-relative spawn offset to heightmap indices; applies 150.0f height scale
- **Spawn Rate**: 3–7 splashes per frame when raining; randomized lifetime [0.2..0.4] seconds
- **Visual**: GL_POINTS with triangle-wave size scaling, GL_SRC_ALPHA blending, disabled depth write for layering
- **Position Offset**: +0.08 world units above sampled terrain height for visual clarity
- Constructor signature: `SplashSystem(int maxSplashes, const std::vector<float>& heightmap, int hmWidth, int hmHeight, float terrainWidth, float terrainDepth)`
- Dependencies: heightmap (512×512), terrain size (800×800), camera position

#### 8. **LightingSystem** (`include/LightingSystem.h`, `src/LightingSystem.cpp`)
- Implements day/night cycle with sun orbit
- Calculates dynamic light color based on time of day
- Interpolates sky color from night (dark) to day (bright)
- Provides sun intensity (0.2 at night, 1.0 at day)
- **Day Speed**: 0.1f (one full day = ~63 seconds)
- **Shadow Mapping**: Manages depth FBO (2048×2048), depth texture, light matrices
- **Fog Integration**: Provides dynamic fog color tied to day/night cycle

#### 10. **CinematicCamera** (`include/CinematicCamera.h`, `src/CinematicCamera.cpp`)
- Extends Camera with a scripted 14-shot cinematic sequence (525 seconds total)
- Keeps camera 3+ units above terrain using bilinear height sampling and forward collision checks
- Synchronizes shot progression with day/night transitions in the main loop
- Mid-sequence rebalanced framing includes broader wide shots and a direct sunrise callback shot

### Supporting Systems (Existing)

- **Camera** (`include/Camera.h`, `src/Camera.cpp`)
  - FPS camera with mouse look and WASD movement
  - Default position: (125, 200, 175) above town center

- **Terrain** (`include/Terrain.h`, `src/Terrain.cpp`)
  - 800x800 vertex grid with procedural Perlin noise heightmap
  - Height range: 0-150 units
  - Includes normal mapping from heightmap slopes
  - Dynamic coloring based on height bands (grass → dirt → rock → snow)

- **NoiseMap** (`include/NoiseMap.h`, `src/NoiseMap.cpp`)
  - Perlin noise generation (512x512 heightmap)
  - Octave-based fractal brownian motion
  - Configurable scale and amplitude

- **Shader** (`include/Shader.h`, `src/Shader.cpp`)
  - Shader compilation and linking utility
  - Uniform setters for matrices, vectors, floats

### Shader Assets

```
assets/shaders/
├── terrain.vert/frag    - Heightmap-based terrain with Phong lighting, shadow mapping, fog
├── grass.vert/frag      - Terrain-normal-aligned grass with wind sway (quadratic falloff, clamped)
├── tree.vert/frag       - Instanced tree rendering with fog and wind
├── building.vert/frag   - Instanced building rendering with fog
├── person.vert/frag     - Legacy NPC shader (system currently disabled in main_new.cpp)
├── particle.vert/frag   - GL_POINTS firefly rendering with additive glow
├── water.vert/frag      - Procedural sine wave water with light response, fog
├── rain.vert/frag       - Rain particle rendering, camera-centered
├── splash.vert/frag     - Terrain-aware rain splash particles with lifetime scaling
├── depth.vert/frag      - Shadow mapping depth pass (light space rendering)
└── skybox.vert/frag     - Dynamic procedural skybox with stars, sunset glow
```

### Build System

- **Build Directory**: `build/`
- **CMakeLists.txt**: Updated to compile all modular sources
- **Dependencies**: OpenGL 3.3 Core, GLFW3, GLM, GLAD

### File Organization

```
CGAssignment/
├── main_new.cpp          # Clean refactored main (uses systems)
├── main.cpp              # Original monolithic version (deprecated)
├── CMakeLists.txt        # Updated build system
├── include/
│   ├── Camera.h
│   ├── Terrain.h
│   ├── Shader.h
│   ├── NoiseMap.h
│   ├── TreeSystem.h      # NEW
│   ├── GrassSystem.h     # NEW
│   ├── BuildingSystem.h  # NEW
│   ├── NPCSystem.h       # NEW
│   ├── ParticleSystem.h  # NEW
│   ├── WaterSystem.h     # NEW
│   └── LightingSystem.h  # NEW
├── src/
│   ├── Camera.cpp
│   ├── Terrain.cpp
│   ├── Shader.cpp
│   ├── NoiseMap.cpp
│   ├── TreeSystem.cpp    # NEW
│   ├── GrassSystem.cpp   # NEW
│   ├── BuildingSystem.cpp # NEW
│   ├── NPCSystem.cpp     # NEW
│   ├── ParticleSystem.cpp # NEW
│   ├── WaterSystem.cpp   # NEW
│   └── LightingSystem.cpp # NEW
├── assets/shaders/       # Shader files
├── build/                # CMake build directory
└── glad.c                # OpenGL loader
```

### Compilation

```bash
cd build
cmake ..
make
./OpenGLTerrainInstancing
```

### Controls

- **W/A/S/D** - Move forward/left/backward/right
- **Q/E** - Move up/down
- **Mouse** - Look around (captured)
- **ESC** - Exit
- **F** - Toggle wireframe mode
- **R** - Toggle rain
- **C** - Toggle cinematic mode
- **SPACE** - Pause/resume cinematic
- **BACKSPACE** - Reset cinematic timeline

### Key Features

✅ **Shadow Mapping** - Real-time directional shadows with PCF filtering (2048×2048 depth map)
✅ **Volumetric Fog** - Exponential squared fog formula, dynamically tied to day/night cycle
✅ **Dynamic Procedural Skybox** - Multi-octave stars with colors, soft halos, twinkling
✅ **Sunset Glow** - Orange/red atmospheric glow when sun is near horizon
✅ **Day/Night Cycle** - Sun orbits every ~63 seconds, all lighting changes dynamically
✅ **Firefly Particles** - 500 glowing particles with physics-based movement
✅ **Dynamic Water + Shoreline** - Procedural waves with irregular coast and beach transition
✅ **Terrain Instancing** - 15,000 trees, 16 buildings, and dense mountain grass rendered efficiently
✅ **Two-Pass Rendering** - Depth pass (light space) + Color pass (camera space with shadows)
✅ **Cinematic Sequence** - 14 timed shots (8:45) with terrain-safe camera movement and synchronized lighting progression

### Rendering Pipeline

The modern rendering pipeline uses a **two-pass approach**:

**Pass 1: Depth Rendering (Shadow Map)**
- Viewport: 2048×2048 (shadow resolution)
- Framebuffer: Depth FBO (writes to depth texture)
- Shader: depth.vert/frag (minimal, light space transformation)
- Renders: Terrain geometry from light's perspective

**Pass 2: Color Rendering (Scene)**
- Viewport: Restored to window size (1280×720 default)
- Framebuffer: Screen framebuffer
- Shaders: All standard shaders (terrain, tree, building, water, etc.)
- Shadow Map: Bound as GL_TEXTURE1, sampled in fragment shaders
- Fog: Applied using camera distance, color varies with day/night cycle

**Skybox (Final)**
- Rendered last after all scene geometry
- Depth function: GL_LEQUAL (always behind scene)
- Procedural shader calculates sky color, sunset, stars based on sun position
- View matrix: Modified to remove translation (follows camera perspective)

### Advanced Features

#### Shadow Mapping with PCF Filtering
- **Depth Map**: 2048×2048 floating-point depth texture
- **Light Projection**: Orthographic (-150, 150) units with 1-300 plane distances
- **PCF Kernel**: 3×3 filter for soft shadow edges
- **Bias System**: Dynamic bias (0.005-0.05) prevents shadow acne
- **Performance**: Single depth pass + shadow sampling in color pass

#### Volumetric Exponential Fog
- **Formula**: $fog\_factor = e^{-(distance \cdot density)^2}$
- **Dynamic Color**: Fog color changes with day/night cycle (light blue day → dark blue night)
- **Shader Integration**: Applied in terrain, tree, grass, water, building, person shaders
- **Distance Calculation**: Computed in vertex shader for per-pixel accuracy

#### Procedural Skybox System
- **Technology**: Pure GLSL procedural generation (no texture files)
- **Sky Gradient**: Smooth interpolation from deep black (night) to bright blue (day)
- **Sunset Effect**: Orange/red glow when sun is near horizon, fades with distance from sun
- **Star Field**: Three-layer system with 1000+ stars visible
  - Layer 1 (6%): Primary bright stars with color variation (yellow, white, blue)
  - Layer 2 (3%): Secondary stars with medium brightness
  - Layer 3 (1.5%): Tertiary faint stars for depth
- **Star Features**: Gaussian glow halos, color variation, subtle twinkling, smooth fade-in/out
- **No Loading**: Entire skybox computed in real-time fragment shader

### Performance Notes

- **Tree Count**: 15,000 (instanced, 1 draw call)
- **Building Count**: 16 (instanced, 1 draw call)
- **NPC Count**: Disabled in active `main_new.cpp` pipeline
- **Terrain Resolution**: 800x800 vertices
- **Particle Count**: 500 (updated each frame)
- **Water Grid**: 500x500 triangles (100k + polygons)
- **Heightmap**: 512x512 (procedural, no file I/O)

### Extension Points

To add new systems:

1. Create `include/YourSystem.h` with class definition
2. Create `src/YourSystem.cpp` with implementation
3. Add `src/YourSystem.cpp` to CMakeLists.txt SOURCES
4. Include header in `main_new.cpp`
5. Instantiate and call render methods in main loop

### Visual Settings (Adjustable in Code)

- **Fog Density**: 0.0002-0.0005 (varies by shader - terrain/grass/water/etc.)
- **Fog Formula**: Exponential squared ($visibility = e^{-(distance \cdot density)^2}$)
- **Day Speed**: 0.1f (LightingSystem.cpp) - lower = longer days
- **Water Level**: ~-1.5f (WaterSystem)
- **Particle Count**: 500 (main_new.cpp, ParticleSystem)
- **Sun Radius**: 100.0f (LightingSystem.cpp)
- **Shadow Map Resolution**: 2048×2048 (LightingSystem.cpp)
- **Shadow Projection**: Orthographic -150 to +150 units (LightingSystem.cpp)
- **Star Density**: Three layers (primary 6%, secondary 3%, tertiary 1.5% occurrence)


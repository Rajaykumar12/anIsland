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
- Generates thousands of grass blade instances across flat terrain
- Procedurally places grass on slopes < 15° and heights < 35 units
- Each blade is a simple quad (2 triangles) with wind animation
- Uses efficient instanced rendering (single draw call)
- Wind effect adds swaying motion to all blades simultaneously
- Includes randomized positioning to avoid grid artifacts

#### 3. **BuildingSystem** (`include/BuildingSystem.h`, `src/BuildingSystem.cpp`)
- Creates a 4x4 grid of houses in the town center (100-150, 150-200)
- Each building has walls, roof (pyramid), and windows
- All geometry stored as triangles for GL_TRIANGLES rendering
- Instanced rendering for efficient batch processing

#### 4. **NPCSystem** (`include/NPCSystem.h`, `src/NPCSystem.cpp`)
- Spawns 30 NPCs with walking animations
- Each NPC modeled as head + body + arms + legs
- Positioned randomly within the town coordinates
- Uses time-based shader animation for walking motion

#### 5. **ParticleSystem** (`include/ParticleSystem.h`, `src/ParticleSystem.cpp`)
- Manages 500 firefly particles with physics simulation
- Updates positions each frame with velocity-based movement
- Adds fluttering effect with random jitter
- Renders as GL_POINTS with additive blending for glow effect

#### 6. **WaterSystem** (`include/WaterSystem.h`, `src/WaterSystem.cpp`)
- Generates 500x500 grid water mesh
- Covers ±500 units around terrain center
- Water positioned at Y = -10 to fill natural valleys
- Uses procedural sine wave displacement for wave simulation
- Implements height-based color blending (deep blue to shallow cyan)

#### 7. **LightingSystem** (`include/LightingSystem.h`, `src/LightingSystem.cpp`)
- Implements day/night cycle with sun orbit
- Calculates dynamic light color based on time of day
- Interpolates sky color from night (dark) to day (bright)
- Provides sun intensity (0.2 at night, 1.0 at day)
- **Day Speed**: 0.1f (one full day = ~63 seconds)
- **Shadow Mapping**: Manages depth FBO (2048×2048), depth texture, light matrices
- **Fog Integration**: Provides dynamic fog color tied to day/night cycle

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
├── grass.vert/frag      - Instanced grass blades with wind animation, fog
├── tree.vert/frag       - Instanced tree rendering with fog
├── building.vert/frag   - Instanced building rendering with fog
├── person.vert/frag     - NPCs with walking animation, fog
├── particle.vert/frag   - GL_POINTS firefly rendering with glow
├── water.vert/frag      - Procedural sine wave water with light response, fog
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

### Key Features

✅ **Shadow Mapping** - Real-time directional shadows with PCF filtering (2048×2048 depth map)
✅ **Volumetric Fog** - Exponential squared fog formula, dynamically tied to day/night cycle
✅ **Dynamic Procedural Skybox** - Multi-octave stars with colors, soft halos, twinkling
✅ **Sunset Glow** - Orange/red atmospheric glow when sun is near horizon
✅ **Day/Night Cycle** - Sun orbits every ~63 seconds, all lighting changes dynamically
✅ **Firefly Particles** - 500 glowing particles with physics-based movement
✅ **Dynamic Water** - Procedural sine waves fill terrain valleys, island surrounded by ocean
✅ **Terrain Instancing** - 15,000 trees, 16 buildings, 30 NPCs rendered efficiently
✅ **Two-Pass Rendering** - Depth pass (light space) + Color pass (camera space with shadows)

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
- **NPC Count**: 30 (instanced, 1 draw call)
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
- **Water Level**: -10.0f (WaterSystem)
- **Particle Count**: 500 (main_new.cpp, ParticleSystem)
- **Sun Radius**: 100.0f (LightingSystem.cpp)
- **Shadow Map Resolution**: 2048×2048 (LightingSystem.cpp)
- **Shadow Projection**: Orthographic -150 to +150 units (LightingSystem.cpp)
- **Star Density**: Three layers (primary 6%, secondary 3%, tertiary 1.5% occurrence)


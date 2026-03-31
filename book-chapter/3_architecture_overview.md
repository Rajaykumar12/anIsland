# 3. Architecture Overview

## 3.1 System Architecture and Component Design

The rendering engine follows a **component-based architecture** where each major feature (terrain, vegetation, water, lighting, particles) is implemented as an independent system. This design promotes code organization, testability, and maintainability.

```
┌─────────────────────────────────────────────────────────────┐
│                      Main Render Loop                       │
└────────────┬────────────────────────────────────────────────┘
             │
    ┌────────┴─────────────────────────────────────────────────┐
    │                                                           │
    ▼                                                           ▼
┌──────────────────────┐                    ┌──────────────────────┐
│   Update Phase       │                    │   Render Phase       │
├──────────────────────┤                    ├──────────────────────┤
│ • Lighting cycle     │                    │ • Depth pass         │
│ • Particle physics   │                    │ • Color pass         │
│ • Wind strength      │                    │ • Shadow filtering   │
│ • Camera movement    │                    │ • Composition        │
└──────────────────────┘                    └──────────────────────┘
```

### Core Systems

Each system implements three standard methods:

```cpp
class ISystem {
public:
    virtual void Initialize()  = 0;    // GPU resource allocation
    virtual void Update(float dt)      = 0;    // CPU-side logic
    virtual void Render(const ShaderProgram& shader) = 0;    // Draw calls
};
```

**System Registry:**

| System | File | Primary Responsibility |
|--------|------|----------------------|
| Terrain | `Terrain.h/cpp` | Heightmap generation, mesh rendering |
| TreeSystem | `TreeSystem.h/cpp` | Tree placement, wind animation, instancing |
| GrassSystem | `GrassSystem.h/cpp` | Grass placement, terrain alignment, wind |
| WaterSystem | `WaterSystem.h/cpp` | Wave simulation, surface rendering |
| ParticleSystem | `ParticleSystem.h/cpp` | Firefly placement, physics, rendering |
| RainSystem | `RainSystem.h/cpp` | Rain particle generation, camera tracking |
| SplashSystem | `SplashSystem.h/cpp` | Splash spawning, lifetime management |
| LightingSystem | `LightingSystem.h/cpp` | Sun orbit, day/night, shadow maps |
| Camera | `Camera.h/cpp` | View matrix, perspective, movement |

## 3.2 Two-Pass Rendering Pipeline

The rendering architecture employs a **two-pass approach** optimized for shadow mapping:

### Pass 1: Depth Pass (Shadow Map Generation)

The first pass renders the scene from the light source's perspective, capturing depth information into a high-resolution texture (shadow map).

```
Step 1: Configure Light Space
────────────────────────────
lightPos = (100 * cos(t*0.1), 100 * sin(t*0.1), 0)
viewMatrix = LookAt(lightPos, terrainCenter, up)
projMatrix = Orthographic(-150, 150, -150, 150, near, far)

Step 2: Render to Shadow Map
────────────────────────────
glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO)
glViewport(0, 0, 2048, 2048)
glClear(GL_DEPTH_BUFFER_BIT)
depthShader.use()
depthShader.setMat4("u_LightSpaceMatrix", lightSpaceMatrix)

Step 3: Draw Depth-Contributing Geometry
────────────────────────────────────────
terrain.Draw()           // Terrain casts shadows
treeSystem.Render()      // Trees cast shadows
// Result: Shadow map texture
```

**Key Data Structure:**

```cpp
struct DepthFramebuffer {
    GLuint framebuffer;      // FBO handle
    GLuint depthTexture;     // GL_TEXTURE_2D, 2048×2048
    glm::mat4 lightSpaceMatrix;    // Light view-projection
};
```

### Pass 2: Color Pass (Camera View Rendering)

The second pass renders the complete scene from the camera perspective, sampling the shadow map to compute shadowed pixels.

```
Step 1: Reset Render Target
──────────────────────────
glBindFramebuffer(GL_FRAMEBUFFER, 0)  // Back buffer
glViewport(0, 0, screenWidth, screenHeight)
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

Step 2: Render Order (Back to Front)
───────────────────────────────────
Depth Buffer: GL_LESS (standard z-testing)
Blending: GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA

  1. Terrain
     ├─ Sample shadow map at each fragment
     ├─ Apply height-based colors
     └─ PCF-filtered directional light

  2. Water
     ├─ Procedural wave displacement
     ├─ Normal map sampling for reflection
     └─ Fresnel blending

  3. Vegetation (Trees + Grass)
     ├─ Instance rendering
     ├─ Wind animation in shader
     └─ Simple shadow blending

  4. Particles (Fireflies)
     ├─ Additive blending: GL_SRC_ALPHA, GL_ONE
     ├─ Point sprite rendering
     └─ Distance-based sizing

  5. Rain/Splashes
     ├─ Camera-centered spawning
     ├─ Depth disabled: GL_DEPTH_FUNC(GL_ALWAYS)
     └─ Lifetime-scaled opacity

  6. Skybox
     ├─ Depth test: GL_LEQUAL
     ├─ Procedural generation
     └─ Day/night color interpolation
```

## 3.3 Data Flow Diagram

```
┌──────────────────────────────────────────────────────────────┐
│                    Startup Initialization                    │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  NoiseMap::Generate()                                        │
│      ↓                                                       │
│  Heightmap (512×512 float texture)                          │
│      │                                                       │
│      ├─→ Terrain::SetHeightmap()                            │
│      ├─→ TreeSystem::SetHeightmap()                         │
│      ├─→ GrassSystem::SetHeightmap()                        │
│      ├─→ ParticleSystem::SetHeightmap()                     │
│      └─→ SplashSystem::SetHeightmap()                       │
│                                                              │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│                    Per-Frame Update Phase                    │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  dt = 1/60 seconds (for 60 FPS)                            │
│  currentTime += dt                                          │
│  windStrength = 0.5 + 0.5 * sin(currentTime * 0.15)        │
│                                                              │
│  LightingSystem::Update(dt)                                 │
│      ├─→ sunPosition = orbit(currentTime)                   │
│      ├─→ lightColor = interpolate(night, day)              │
│      └─→ updateSkyColor()                                   │
│                                                              │
│  Camera::Update(input)                                      │
│      ├─→ updateViewMatrix()                                │
│      └─→ updateViewFrustum()                               │
│                                                              │
│  ParticleSystem::Update(dt)                                │
│      ├─→ updatePhysics(dt)                                 │
│      ├─→ enforceConstraints()                              │
│      └─→ respawnOutOfBounds()                              │
│                                                              │
│  RainSystem::Update(dt)                                     │
│      └─→ updateParticlePositions(cameraPos)               │
│                                                              │
│  SplashSystem::Update(dt)                                   │
│      └─→ decrementLifetimes()                              │
│      └─→ spawnNewSplashes(rainEnabled)                     │
│                                                              │
└──────────────────────────────────────────────────────────────┘

┌──────────────────────────────────────────────────────────────┐
│                    Per-Frame Render Phase                    │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│  ═══ DEPTH PASS ═══                                         │
│  LightingSystem::RenderDepthPass()                          │
│      ├─→ bindShadowMapFBO()                                │
│      ├─→ terrain.Draw()                                    │
│      └─→ treeSystem.Render()                               │
│                                                              │
│  ═══ COLOR PASS ═══                                         │
│  glBindFramebuffer(0)  // Back buffer                       │
│                                                              │
│  terrain.Draw()     → Phong + PCF shadow                   │
│  water.Draw()       → Wave + Fresnel                       │
│  treeSystem.Render()     → Wind animation                   │
│  grassSystem.Render()    → Terrain alignment + wind        │
│  particleSystem.Render() → Additive glow                   │
│  rainSystem.Render()     → Point sprites                   │
│  splashSystem.Render()   → Lifetime scaling                │
│  skybox.Draw()      → Procedural sky + stars              │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

## 3.4 Memory Layout and GPU Resource Management

### GPU Buffer Organization

```cpp
// Terrain Example
struct TerrainBuffers {
    GLuint VAO;        // Vertex Array Object (manages state)
    GLuint VBO;        // 800×800 vertices: position + texCoord
    GLuint EBO;        // Element buffer: indices for triangulation
    GLuint heightmapTexture;  // Texture unit for height queries
};

// Tree Instancing Example
struct TreeBuffers {
    GLuint geometryVAO;          // Tree mesh topology
    GLuint geometryVBO;          // Vertex positions + normals
    GLuint instanceVBO;          // Per-instance data buffer
    GLuint windHeightVBO;        // Optional: precomputed wind influence
};

// Instance Data Layout (16 bytes aligned)
struct TreeInstance {
    glm::vec3 position;          // 12 bytes
    float     _padding;          // 4 bytes (alignment)
};
```

### Texture Binding Strategy

```cpp
enum TextureUnit {
    TU_Heightmap = 0,       // Terrain height queries (all systems)
    TU_ShadowMap = 1,       // Depth pass result
    TU_SkyEnvironment = 2,  // Skybox procedural texture
    TU_WaterNormal = 3      // Normal map for water reflection
};

// Sample Usage in Shader
layout(binding = 0) uniform sampler2D u_Heightmap;
layout(binding = 1) uniform sampler2DShadow u_ShadowMap;
```

## 3.5 Performance Profiling Structure

Each frame is subdivided for performance analysis:

```
Frame Time (16.67ms target for 60 FPS)
├─ Update Phase (~3-4ms)
│  ├─ Lighting update
│  ├─ Particle physics
│  ├─ Camera logic
│  └─ Spawn checks
│
└─ Render Phase (~12-13ms)
   ├─ Depth pass (~3ms)
   │  ├─ Terrain draw
   │  └─ Tree draw
   │
   └─ Color pass (~9-10ms)
      ├─ Terrain draw (~1.5ms)
      ├─ Water draw (~0.8ms)
      ├─ Vegetation draw (~2ms trees + ~1.5ms grass)
      ├─ Particles draw (~0.7ms fireflies + ~0.5ms rain)
      ├─ Splashes draw (~0.3ms)
      └─ Skybox draw (~0.2ms)
```

---

## Summary

The architecture employs a modular component-based design with a two-pass rendering pipeline optimized for dynamic shadow mapping. Each system maintains its own GPU resources and implements a standard initialization-update-render interface, enabling independent development and testing. The next sections detail implementation of each individual system.

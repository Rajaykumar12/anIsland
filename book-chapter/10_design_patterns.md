# 10. Design Patterns and Architectural Best Practices

## 10.1 Component-Based Architecture

The entire project follows a **component-based system design** where each major feature (terrain, vegetation, water, etc.) is implemented as an independent, self-contained system.

### System Interface Pattern

Every system implements a consistent interface:

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    
    /// Initialize GPU resources (called once at startup)
    virtual void Initialize() = 0;
    
    /// Update CPU-side logic (called each frame)
    virtual void Update(float deltaTime) = 0;
    
    /// Render system (called each frame in render loop)
    virtual void Render(const ShaderProgram& shader) = 0;
};

// Example implementation
class TerrainSystem : public ISystem {
public:
    void Initialize() override {
        GenerateHeightmap();
        SetupMesh();
        UploadToGPU();
    }
    
    void Update(float deltaTime) override {
        // Terrain is static - no per-frame updates
    }
    
    void Render(const ShaderProgram& shader) override {
        shader.use();
        glBindVertexArray(VAO);
        glDrawElements(...);
    }
};
```

**Advantages:**
- **Testability:** Each system can be initialized/updated independently
- **Extensibility:** New systems follow the same pattern
- **Maintainability:** Clear separation of concerns
- **Flexibility:** Systems can be enabled/disabled by instantiation

### Main Loop Integration

```cpp
int main() {
    // Step 1: Initialize all systems
    Terrain terrain;
    terrain.Initialize();
    
    TreeSystem trees;
    trees.Initialize();
    
    GrassSystem grass;
    grass.Initialize();
    
    WaterSystem water;
    water.Initialize();
    
    ParticleSystem particles;
    particles.Initialize();
    
    RainSystem rain;
    rain.Initialize();
    
    // Step 2: Main render loop
    while (running) {
        float dt = clock.DeltaTime();
        
        // Update phase
        particles.Update(dt);
        rain.Update(dt);
        lighting.Update(dt);
        
        // Render phase
        lighting.RenderDepthPass(terrain, trees);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);  // Render to back buffer
        terrain.Render(terrainShader);
        water.Render(waterShader);
        trees.Render(treeShader);
        grass.Render(grassShader);
        particles.Render(particleShader);
        rain.Render(rainShader);
        skybox.Render(skyboxShader);
        
        window.SwapBuffers();
    }
    
    // Step 3: Cleanup (RAII handles this automatically)
    return 0;
}
```

## 10.2 Resource Management: RAII Pattern

C++ Resource Acquisition Is Initialization (RAII) ensures proper GPU resource cleanup:

```cpp
class GLTexture {
    GLuint handle = 0;
    
public:
    GLTexture() {
        glGenTextures(1, &handle);
    }
    
    ~GLTexture() {
        if (handle) glDeleteTextures(1, &handle);
    }
    
    // Prevent copying (GPU resources can't duplicate)
    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;
    
    // Allow moving
    GLTexture(GLTexture&& other) noexcept 
        : handle(other.release()) {}
    
    GLuint get() const { return handle; }
    GLuint release() {
        GLuint temp = handle;
        handle = 0;
        return temp;
    }
};

// Usage - automatic cleanup on scope exit
{
    GLTexture heightmap;
    // ... use heightmap
} // ~GLTexture() automatically called, GPU memory freed
```

## 10.3 Heightmap Caching Strategy

Multiple systems (terrain, trees, grass, particles) need terrain height queries. Rather than each maintaining separate heightmap copies, they share a single instance:

```cpp
class TerrainManager {
    NoiseMap heightmap;
    Terrain terrain;
    TreeSystem trees;
    GrassSystem grass;
    ParticleSystem particles;
    
public:
    void Initialize() {
        // Generate heightmap once
        heightmap.Generate(512, 512);
        
        // Pass reference to all systems
        terrain.SetHeightmap(heightmap);
        trees.SetHeightmap(heightmap);
        grass.SetHeightmap(heightmap);
        particles.SetHeightmap(heightmap);
        
        // Now initialize all systems
        terrain.Initialize();
        trees.Initialize();
        grass.Initialize();
        particles.Initialize();
    }
};
```

**Benefits:**
- **Memory efficiency:** Single heightmap (256 KB for 512×512), shared across 4+ systems
- **Consistency:** All systems query identical height data
- **Performance:** Shared cache locality

## 10.4 Separation of Concerns: Data vs. Rendering

Systems clearly separate data and rendering logic:

```cpp
// DATA REPRESENTATION
struct ParticleData {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> velocities;
    std::vector<float> lifetimes;
};

class ParticleSystem {
    ParticleData data;          // CPU-side logic
    GLuint particleVBO;         // GPU-side rendering
    
public:
    void Update(float dt) {
        // CPU: Update positions, velocities
        for (auto& p : data.positions) {
            p.x += data.velocities[i].x * dt;
            // ... more CPU logic
        }
    }
    
    void Render(const Shader& shader) {
        // GPU: Submit to rendering pipeline
        glBindBuffer(GL_COPY_WRITE_BUFFER, particleVBO);
        glBufferSubData(...);  // Upload updated data
        
        shader.use();
        glBindVertexArray(particleVAO);
        glDrawArrays(GL_POINTS, 0, data.positions.size());
    }
};
```

**Advantages:**
- **Testability:** Data logic can be tested without GPU
- **Flexibility:** Can swap rendering backends (DirectX, Vulkan)
- **Performance:** Clear CPU/GPU workload division

## 10.5 Error Handling and Debugging

### Compile-Time Error Prevention

```cpp
// Prevent common mistakes at compile time
class ShaderProgram {
    GLuint handle;
    
public:
    void use() const {
        glUseProgram(handle);  // Can only use if initialized
    }
    
    // Prevent accidental copying
    ShaderProgram(const ShaderProgram&) = delete;
    ShaderProgram& operator=(const ShaderProgram&) = delete;
};

// This won't compile - good!
ShaderProgram a, b;
b = a;  // Error: ReferenceCountedShader operator= deleted

// But this is safe
ShaderProgram a, b;
std::swap(a, b);  // Compiles - explicit move semantics
```

### Runtime Error Detection

```cpp
#ifdef DEBUG
    #define CHECK_GL_ERROR() \
        { GLenum err = glGetError(); \
          if (err != GL_NO_ERROR) \
            fprintf(stderr, "GL Error %d at %s:%d\n", err, __FILE__, __LINE__); }
#else
    #define CHECK_GL_ERROR()
#endif

// Usage
glDrawElements(...);
CHECK_GL_ERROR();
```

## 10.6 Naming Conventions

Consistent naming enables rapid code navigation:

```cpp
// Member variables: m_ prefix
class TreeSystem {
    std::vector<TreeInstance> m_instances;
    GLuint m_treeVAO;
    ShaderProgram m_treeShader;
};

// Local variables: snake_case
void TreeSystem::Update(float dt) {
    float wind_strength = 0.5f + 0.5f * sin(current_time * 0.15f);
    
    for (auto& tree : m_instances) {
        float sway_angle = wind_strength * tree.height_factor;
        tree.position += glm::vec3(sin(sway_angle), 0, 0) * dt;
    }
}

// Functions: PascalCase
void InitializeShaders();
void UpdateParticles(float dt);
glm::vec3 SampleTerrainHeight(float x, float z);

// Constants: SCREAMING_SNAKE_CASE
static constexpr int MAX_PARTICLES = 500;
static constexpr float WATER_LEVEL = -1.5f;
```

## 10.7 Extensibility Example: Adding a New System

Implement a new system (smoke particles) following established patterns:

```cpp
// Step 1: Define the system class
class SmokeSystem : public ISystem {
private:
    struct SmokeParticle {
        glm::vec3 position;
        glm::vec3 velocity;
        float lifetime;
        float alpha;
    };
    
    std::vector<SmokeParticle> m_particles;
    GLuint m_smokeVAO, m_smokeVBO;
    
public:
    void Initialize() override;
    void Update(float dt) override;
    void Render(const ShaderProgram& shader) override;
};

// Step 2: Implement methods
void SmokeSystem::Initialize() {
    // ... standard VAO/VBO setup
}

void SmokeSystem::Update(float dt) {
    for (auto& p : m_particles) {
        p.position += p.velocity * dt;
        p.velocity.y += 0.1f * dt;  // Buoyancy
        p.lifetime -= dt;
        p.alpha = p.lifetime / max_lifetime;
    }
}

void SmokeSystem::Render(const ShaderProgram& shader) {
    shader.use();
    glBindVertexArray(m_smokeVAO);
    glDrawArraysInstanced(...);
}

// Step 3: Integrate into main loop
int main() {
    SmokeSystem smoke;
    smoke.Initialize();
    
    while (running) {
        smoke.Update(dt);
        smoke.Render(smokeShader);
    }
}
```

**Result:** New system works seamlessly with existing architecture without modifying core loop.

---

## Summary

The project demonstrates professional software architecture combining component-based design, RAII resource management, and clear separation of concerns. These patterns enable code maintainability, extensibility, and testability—essential for production graphics engines. The architecture scales gracefully, allowing systems to be added, modified, or disabled without affecting others.

**Key Architectural Principles:**
- **Component-based:** Each system is independent
- **Interface consistency:** All systems implement ISystem pattern
- **Resource safety:** RAII ensures GPU memory cleanup
- **Data separation:** Logical data distinct from rendering code
- **Naming conventions:** Consistent across codebase
- **Error handling:** Compile-time and runtime validation
- **Extensibility:** New systems follow established patterns

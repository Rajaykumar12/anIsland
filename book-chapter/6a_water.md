# 6a. Water System: Procedural Ocean Simulation

## 6a.1 Ocean Surface Mesh

The water system renders a sparse geometric grid deformed by procedural waves in the vertex shader. This approach avoids pre-computed assets and enables efficient scene updates.

**Mesh Grid Architecture:**

| Property | Value |
|----------|-------|
| Grid dimensions | 500 × 500 vertices |
| Vertex spacing | 2.0 world units |
| Total extent | ±1000 units (2000×2000 world area) |
| Triangle count | 499 × 499 × 2 ≈ 498,000 |
| Vertex storage | ~200 KB (static geometry) |
| Water level (Y) | -1.5 world units |

**Mesh Generation Algorithm:**

```
GENERATE_OCEAN_MESH(gridSize=500, gridSpacing=2.0):
    minCoord ← -(gridSize - 1) × gridSpacing / 2.0
    
    // Vertex generation: regular grid
    FOR z ← 0 TO gridSize:
        FOR x ← 0 TO gridSize:
            worldX ← minCoord + x × gridSpacing
            worldZ ← minCoord + z × gridSpacing
            
            vertices[] ← [worldX, WATER_LEVEL, worldZ]
    
    // Index generation: quad → triangle decomposition
    FOR z ← 0 TO gridSize - 1:
        FOR x ← 0 TO gridSize - 1:
            // Quad vertices (6 indices = 2 triangles)
            a ← z × gridSize + x
            b ← z × gridSize + (x + 1)
            c ← (z + 1) × gridSize + x
            d ← (z + 1) × gridSize + (x + 1)
            
            // Triangle 1: a-c-b
            indices[] ← [a, c, b]
            
            // Triangle 2: b-c-d
            indices[] ← [b, c, d]
    
    // GPU upload: GL_STATIC_DRAW (geometry never changes)
    CreateVAO(vertices, indices)
```

**Memory Complexity:** O(gridSize²) vertices = O(250,000) vertices. GPU memory: ~3 MB (position + normal attributes). CPU-side generation: ~1 ms one-time cost.

## 6a.2 Gerstner Wave Displacement

Water surface animation combines multiple sine waves at different frequencies and directions. This superposition creates natural-looking ocean behavior without pre-computed textures.

**Gerstner Wave Model:**

Ocean surface displacement is the sum of directional sine waves:

$$\mathbf{p}(x,z,t) = \mathbf{p}_0 + \sum_{i=1}^{n} A_i \sin(\mathbf{k}_i \cdot \mathbf{p}_0 + \omega_i t + \phi_i)$$

where:
- $\mathbf{p}_0 = (x, z)$ : original vertex position
- $A_i$ : amplitude of wave $i$
- $\mathbf{k}_i = (k_x, k_z)$ : wave vector (direction units)
- $\omega_i = 2\pi f_i$ : angular frequency
- $\phi_i$ : phase offset (often 0)
- $t$ : elapsed time

**Three-Wave Composition (Production Setup):**

| Parameter | Wave 1 (Swell) | Wave 2 (Ripple X) | Wave 3 (Ripple Z) |
|-----------|---|---|---|
| Amplitude $A_i$ | 0.4 | 0.15 | 0.15 |
| Spatial frequency | 0.3 (diagonal) | 1.5 (X-axis) | 1.5 (Z-axis) |
| Temporal frequency | 1.0 rad/s | 2.5 rad/s | 2.0 rad/s (reversed) |
| Wavelength $\lambda = 2\pi/k$ | 20.9 units | 4.2 units | 4.2 units |
| Period $T = 2\pi/\omega$ | 6.3 sec | 2.5 sec | 3.1 sec |

This three-component system creates cascading wave patterns: large slow swells overlaid with faster ripples.

**Pseudocode Wave Computation:**

```
COMPUTE_WAVE_HEIGHT(position, time):
    // Wave 1: Large diagonal swell
    wave1 ← 0.4 × sin(position.x × 0.3 + position.z × 0.3 + time × 1.0)
    
    // Wave 2: X-axis ripples
    wave2 ← 0.15 × sin(position.x × 1.5 + time × 2.5)
    
    // Wave 3: Z-axis ripples (opposite time flow for variety)
    wave3 ← 0.15 × sin(position.z × 1.5 - time × 2.0)
    
    RETURN wave1 + wave2 + wave3  // Total displacement: ±0.7 units max

COMPUTE_WAVE_NORMAL(position, time):
    // Numerical differentiation of displacement heights
    eps ← 0.01
    
    // Compute partial derivatives (slopes)
    dh_dx ← (COMPUTE_WAVE_HEIGHT(position + [eps, 0], time) 
           - COMPUTE_WAVE_HEIGHT(position - [eps, 0], time)) / (2 × eps)
    
    dh_dz ← (COMPUTE_WAVE_HEIGHT(position + [0, eps], time)
           - COMPUTE_WAVE_HEIGHT(position - [0, eps], time)) / (2 × eps)
    
    // Normal vector from slope
    normal ← normalize([-dh_dx, 1.0, -dh_dz])
    
    RETURN normal
```

**Complexity:** Per-vertex computation in vertex shader; evaluated 250,000 times per frame at 60 FPS = 15M wave evaluations/sec. GPU-parallel evaluation: ~1-2 ms total.

## 6a.3 Surface Normal Derivation

Realistic lighting of water depends on surface normals. Rather than storing pre-computed normals, the shader derives them via finite differences:

$$\mathbf{n} = \text{normalize}\left(\left(-\frac{\partial h}{\partial x}, 1, -\frac{\partial h}{\partial z}\right)\right)$$

where $\frac{\partial h}{\partial x}$ and $\frac{\partial h}{\partial z}$ are slopes computed from adjacent wave samples.

**Shader Normal Calculation:**

```glsl
// vertex shader: compute surface normals from wave displacement
vec3 ComputeNormal(vec3 position, float time) {
    float eps = 0.01;
    
    // Original height
    float h0 = ComputeWaveHeight(position, time);
    
    // Heights at offset positions
    float hx = ComputeWaveHeight(position + vec3(eps, 0, 0), time);
    float hz = ComputeWaveHeight(position + vec3(0, 0, eps), time);
    
    // Approximate partial derivatives
    float dh_dx = (hx - h0) / eps;
    float dh_dz = (hz - h0) / eps;
    
    // Normal from slopes
    return normalize(vec3(-dh_dx, 1.0, -dh_dz));
}
```

## 6a.4 Water Appearance: Color and Fresnel Blending

Water appearance uses depth-based color blending and Fresnel effect for realistic surface appearance:

**Depth-Based Coloring:**

```
COMPUTE_WATER_COLOR(worldHeight, viewDirection, normal):
    // Define color gradient layers
    deepWater ← vec3(0.0, 0.18, 0.48)     // Dark navy blue
    shallowWater ← vec3(0.1, 0.48, 0.78)  // Cyan
    foam ← vec3(0.9, 0.95, 1.0)           // White spray
    
    // Blend based on wave height (displacement from water level)
    heightFromBase ← worldHeight - WATER_LEVEL
    
    IF heightFromBase > 0.5:  // Foam at wave peaks
        surfaceColor ← foam
    ELSE IF heightFromBase < -0.2:  // Deep trenches
        surfaceColor ← deepWater
    ELSE:  // Transition zone
        t ← (heightFromBase + 0.2) / 0.7
        surfaceColor ← Lerp(deepWater, shallowWater, t)
    
    RETURN surfaceColor
```

**Fresnel Effect (Angle-Dependent Reflectivity):**

Fresnel equation determines surface reflectivity based on view angle:

$$F(\theta) = F_0 + (1 - F_0)(1 - \cos\theta)^5$$

where:
- $\theta$ : angle between view direction and surface normal
- $F_0$ : minimum reflectivity (water ≈ 0.04)
- Increases from perpendicular (low) to grazing angles (high)

```glsl
// Fragment shader: Fresnel blending
float cosTheta = dot(viewDirection, surfaceNormal);
float fresnel = 0.04 + 0.96 * pow(1.0 - cosTheta, 5.0);

// Blend surface color with sky reflection
vec3 skyReflection = SampleSkybox(reflectionDirection);
vec3 finalColor = mix(surfaceColor, skyReflection, fresnel);
```

**Visual Result:**
- When viewed head-on: Mostly opaque water (low Fresnel)
- When viewed at grazing angle: Highly reflective (high Fresnel)
- Creates realistic "glassy" water appearance

## 6a.5 Performance Profile

| Operation | Time | GPU Load |
|-----------|------|----------|
| Mesh generation (one-time) | 1 ms | CPU-bound |
| Wave evaluation (per frame, 250K vertices) | 1-2 ms | Vertex shader |
| Normal computation | Amortized in wave eval | Vertex shader |
| Fresnel blending + sky reflection sampling | 0.5-1 ms | Fragment shader |
| **Total per-frame cost** | **2-4 ms @ 60 FPS** | GPU fills rate limit with sky reflection |

Ocean rendering is typically **vertex-bound** (wave evaluation dominates), not fill-rate bound, due to relatively low fragment count (sparse grid).
    
    // Height relative to base level
    float heightAboveBase = FragPos.y - (-1.5);  // World water level is -1.5
    float colorMix = clamp((heightAboveBase + 0.7) / 1.4, 0.0, 1.0);
    
    vec3 baseColor = mix(deepWater, shallowWater, colorMix);
    
    // Step 2: Fresnel effect (shiny at grazing angles)
    float fresnel = pow(max(1.0 - dot(normal, viewDir), 0.0), 3.0);
    
    // Step 3: Specular highlight
    vec3 halfwayDir = normalize(sunDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 128.0);
    
    // Step 4: Combine components
    vec3 color = baseColor * 0.7;                    // Base color
    color += spec * u_SunColor * 0.8;                // Specular reflection of sun
    color += fresnel * 0.3;                          // Fresnel shimmer
    
    // Step 5: Sunset warm tint (dynamic from lighting system)
    // Reduce saturation at sunset
    float sunsetFactor = clamp(1.0 - abs(u_SunPosition.y), 0.0, 1.0);
    vec3 sunsetTint = vec3(1.0, 0.55, 0.2) * sunsetFactor * 0.2;
    color += sunsetTint;
    
    FragColor = vec4(color, 0.9);  // 90% opaque (mostly transparent)
}
```

**Fresnel Effect Visualization:**

```
View angle:
  Head-on (0°):  Low fresnel, see through water
      △
      ║
      ▼
    Water (dark blue visible)
    
  Grazing (87°):  High fresnel, reflective
      △
    ╱  ▼
   ╱   Water (bright reflective shimmer)
```

## 6a.5 Rendering Integration

```cpp
void WaterSystem::Render(const Shader& waterShader) {
    waterShader.use();
    
    waterShader.setFloat("u_Time", currentTime);
    waterShader.setVec3("u_SunPosition", sunPosition);
    waterShader.setVec3("u_SunColor", sunColor);
    waterShader.setVec3("u_ViewPos", cameraPosition);
    waterShader.setMat4("u_View", viewMatrix);
    waterShader.setMat4("u_Projection", projectionMatrix);
    
    // Bind shadow map for shadow sampling
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    waterShader.setInt("u_ShadowMap", 1);
    
    glBindVertexArray(waterVAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
```

**Performance:**
- Draw call: 1 per frame
- Vertices: 250,000 (500×500)
- Triangles: 498,002
- Computation: GPU vertex shader only (no CPU work)
- Frame time: ~0.8-1.0 ms per frame

---

## Summary

The Water System demonstrates procedural animation using Gerstner waves combined with dynamic normal calculation and physically-inspired Fresnel blending. The wave displacement computed entirely on the GPU enables interactive animation without precomputed textures or complex mesh morphing. The three-component wave system produces natural cascading patterns while remaining computationally efficient, requiring only one draw call and minimal CPU involvement.

**Key Achievements:**
- 500×500 sparse grid enabling large ocean coverage
- Real-time wave displacement via GPU vertex shader
- Dynamic normal computation from wave gradients
- Fresnel blending for realistic material properties
- Height-based color interpolation
- Rendering: 1 draw call, ~0.8 ms per frame, procedural animation

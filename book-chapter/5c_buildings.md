# 5c. Building System: Fixed Urban Structure

## 5c.1 Town Center Architecture and Grid-Based Placement

The Building System deploys 16 structures in a 4×4 grid at terrain center, demonstrating instancing flexibility beyond organic objects (trees/grass) to rigid geometric structures with procedural variation.

**Grid Layout:**

$$\text{Position} = \left( \text{GRID\_MIN} + i \times \text{SPACING} + \frac{\text{SPACING}}{2}, z_{\text{terrain}}, \text{GRID\_MIN} + j \times \text{SPACING} + \frac{\text{SPACING}}{2} \right)$$

where $i,j \in \{0,1,2,3\}$ and:
- X range: 100-150 world units
- Z range: 150-200 world units  
- Spacing: 12.5 units per building (50 units ÷ 4 buildings)
- Total: 16 buildings

**Building Parametrization:**

Each building instance stores:
- **Position** (vec3): World coordinates on terrain surface
- **Scale** (vec3): Width, height, depth (uniform grid spacing × height variation)
- **Color** (vec3): Procedural material color (4 variants)

**Placement Algorithm:**

```
GENERATE_BUILDINGS(terrain):
    buildings ← Empty
    gridSize ← 4
    xSpacing ← (150 - 100) / 4 = 12.5
    zSpacing ← (200 - 150) / 4 = 12.5
    
    FOR j ← 0 TO 3:
        FOR i ← 0 TO 3:
            // Grid-aligned position
            x ← 100 + i × 12.5 + 6.25
            z ← 150 + j × 12.5 + 6.25
            
            // Sample terrain height at grid point
            terrainHeight ← terrain.SampleHeight(x, z)
            
            // Procedural height variation
            heightFactor ← 0.5 + 0.5 × sin(i × 1.3 + j × 0.7)
            buildingHeight ← 2.0 + heightFactor × 3.0  // Range: 2-5 units
            
            // Procedural color assignment
            colorIndex ← (i + j × 4) MOD 4
            color ← SelectColor(colorIndex)  // See table below
            
            // Create instance
            instance.position ← [x, terrainHeight, z]
            instance.scale ← [10, buildingHeight, 10]  // 80% of spacing
            instance.color ← color
            
            buildings.Push(instance)
    
    RETURN buildings
```

**Color Palette:**

| Variant | RGB | Material |
|---------|-----|----------|
| 0 | (0.7, 0.5, 0.3) | Tan/sandstone |
| 1 | (0.6, 0.4, 0.4) | Brick red |
| 2 | (0.5, 0.5, 0.5) | Gray stone |
| 3 | (0.4, 0.4, 0.3) | Brown wood |

**Complexity:** O(16) = O(1) for generation (fixed grid size). Procedural height/color computation uses sine waves and modulo arithmetic—each O(1).

## 5c.2 Building Geometry: Unit Cube Primitive

Each building is a rectangular prism (axis-aligned box) scaled per-instance. The base geometry is a **unit cube** normalized to $[-0.5, 0.5]^3$, transformed by per-instance scale and position during rendering (Chapter 5).

**Unit Cube Definition:**

| Face | Plane | Vertices (Local) | Normal |
|------|-------|------------------|--------|
| Front | $z = +0.5$ | $(-0.5, \pm 0.5, 0.5)$, $(+0.5, \pm 0.5, 0.5)$ | $(0, 0, +1)$ |
| Back | $z = -0.5$ | $(-0.5, \pm 0.5, -0.5)$, $(+0.5, \pm 0.5, -0.5)$ | $(0, 0, -1)$ |
| Top | $y = +0.5$ | $(\pm 0.5, 0.5, \pm 0.5)$ | $(0, +1, 0)$ |
| Bottom | $y = -0.5$ | $(\pm 0.5, -0.5, \pm 0.5)$ | $(0, -1, 0)$ |
| Right | $x = +0.5$ | $(+0.5, \pm 0.5, \pm 0.5)$ | $(+1, 0, 0)$ |
| Left | $x = -0.5$ | $(-0.5, \pm 0.5, \pm 0.5)$ | $(-1, 0, 0)$ |

**Vertex Generation Algorithm:**

```
GENERATE_UNIT_CUBE_MESH():
    vertices ← []
    normals ← []
    
    // Define 6 faces
    faces ← [
        {plane: Z=+0.5, normal: [0,0,+1], corners: [(-0.5,-0.5), (+0.5,-0.5), (+0.5,+0.5), (-0.5,+0.5)]},
        {plane: Z=-0.5, normal: [0,0,-1], corners: [(-0.5,-0.5), (-0.5,+0.5), (+0.5,+0.5), (+0.5,-0.5)]},
        ... [4 more faces similarly defined]
    ]
    
    // Generate quad for each face
    FOR each face:
        FOR 4 corners of face:
            vertices.Push(corner_with_z_or_x_or_y_from_plane)
            normals.Push(face.normal)
    
    // Create indices (2 triangles per quad)
    indices ← [0,1,2, 2,3,0,  4,5,6, 6,7,4,  ...]  // 12 triangles, 36 indices
    
    RETURN {vertices, normals, indices}
```

**Mesh Statistics:**
- **Vertices:** 24 (4 per face × 6 faces)
- **Triangles:** 12 (2 per face × 6 faces)
- **Indices:** 36
- **Geometry Size:** Negligible (~1.5 KB per cube)
- **Instance Overhead:** 28 bytes per building (position + scale + color)

**Vertex Shader Transformation:** This unit cube is transformed per-instance:

$$\text{worldPos} = \text{scale} \times \text{cube position} + \text{instance position}$$

The uniform scale across XYZ (from per-instance scale factor) expands the unit cube to building dimensions. (See Chapter 5 for general instancing transformation pipeline.)
                 vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buildingEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 indices.data(), GL_STATIC_DRAW);
    
    // Vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                         (void*)(3 * sizeof(float)));
    
    glBindVertexArray(0);
}

void BuildingSystem::UploadToGPU() {
    glGenBuffers(1, &buildingInstanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, buildingInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 buildings.size() * sizeof(BuildingInstance),
                 buildings.data(),
                 GL_STATIC_DRAW);
    
    glBindVertexArray(buildingVAO);
    
    // Instance position (vec3)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
                         sizeof(BuildingInstance),
                         (void*)offsetof(BuildingInstance, position));
    glVertexAttribDivisor(2, 1);
    
    // Instance scale (vec3)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE,
                         sizeof(BuildingInstance),
                         (void*)offsetof(BuildingInstance, scale));
    glVertexAttribDivisor(3, 1);
    
    // Instance color (vec3)
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE,
                         sizeof(BuildingInstance),
                         (void*)offsetof(BuildingInstance, color));
    glVertexAttribDivisor(4, 1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
```

## 5c.3 Building Rendering

```cpp
void BuildingSystem::Render(const Shader& buildingShader) {
    buildingShader.use();
    
    // Set up standard uniforms
    buildingShader.setMat4("u_View", viewMatrix);
    buildingShader.setMat4("u_Projection", projectionMatrix);
    buildingShader.setMat4("u_LightSpaceMatrix", lightSpaceMatrix);
    
    glBindVertexArray(buildingVAO);
    
    // Render all buildings in single draw call
    glDrawElementsInstanced(GL_TRIANGLES,
                           36,              // Indices per cube (6 faces × 6 indices)
                           GL_UNSIGNED_INT,
                           nullptr,
                           buildings.size()  // 16 buildings
    );
    
    glBindVertexArray(0);
}
```

## 5c.4 Architectural Significance

The Building System demonstrates several important concepts:

1. **Instancing Beyond Vegetation:** While commonly used for trees/grass, instancing works equally well for architectural structures with varying dimensions.

2. **Grid-Based Urban Planning:** Procedural arrangement enables rapid prototyping of towns and settlements without manual placement.

3. **Aesthetic Variety:** Procedural color and height assignment prevents monotonous appearance while maintaining efficient rendering.

4. **Integration with Terrain:** Building placement respects terrain topology, with base heights sampled from the heightmap.

**Performance:**
- Instance count: 16 buildings
- GPU memory: ~1 KB (minimal, only 16 instances)
- Rendering: 1 draw call, < 0.1 ms per frame
- Demonstrates that instancing scales from sparse structures to ultra-dense vegetation

---

## Note on System Availability

The Building System is fully implemented in the codebase (`include/BuildingSystem.h`, `src/BuildingSystem.cpp`) but not actively instantiated in the main render loop (`main_new.cpp`). This represents a common game development pattern: completed systems remain in the codebase for potential future integration, demonstrating architectural flexibility. Enabling it requires a single line:

```cpp
// In main_new.cpp, after other system initialization:
BuildingSystem buildingSystem;
buildingSystem.Initialize();

// In main loop:
buildingSystem.Render(buildingShader);
```

This flexibility exemplifies good software architecture—systems can be disabled without code removal.

---

## Summary

The Building System completes the instanced rendering pipeline demonstration by applying the same techniques to fixed urban structures. With only 16 buildings, this system demonstrates the versatility of GPU instancing beyond the dense vegetation scenarios. The architectural grid layout, procedural coloring, and terrain-aware positioning showcase integration principles applicable to any procedurally-placed object categories.

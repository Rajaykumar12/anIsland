# 2. Objectives and Performance Targets

## 2.1 Primary Objectives

This project establishes the following technical objectives:

### Objective 1: Rendering Large-Scale Terrain
Implement a procedurally generated 800×800 vertex terrain with natural appearance, efficiently rendered as a single mesh with height-based biome coloring. The terrain should exhibit natural features including mountains, valleys, beaches, and water-level boundaries, all generated via mathematical functions rather than pre-authored content.

**Success Criteria:**
- Terrain generation completes in < 100ms
- Heightmap features natural multi-scale variation
- Height-based color blending produces recognizable biomes
- Terrain renders at consistent 60+ FPS

### Objective 2: Deploy Massive Instanced Object Counts
Render 15,000+ procedurally-placed trees across the terrain with wind animation, and 407,000+ grass blades with terrain-aware orientation and per-blade physics. Both systems must maintain real-time performance through GPU instancing rather than individual draw calls.

**Success Criteria:**
- 15,000 trees rendered in < 2ms per frame
- 407,000 grass blades rendered in < 1.5ms per frame
- Placement respects terrain topology (no floating objects)
- Wind animation is convincing and physically-grounded

### Objective 3: Implement Efficient Shadow Mapping
Establish a two-pass rendering pipeline generating 2048×2048 depth maps from the dynamic light source perspective, with PCF filtering for soft shadow edges. Shadow computation must not degrade performance below 60 FPS.

**Success Criteria:**
- Depth pass completes in < 3ms
- Shadow map PCF filtering is visually smooth
- Shadow bias prevents shadow acne (self-shadowing artifacts)
- Color pass rendering completes in < 13ms total

### Objective 4: Environmental Simulation
Implement multiple environmental systems—water with procedural waves, firefly particles constrained to forest regions, dynamic rain with terrain-aware splashes, and day/night lighting cycle—demonstrating the breadth of modern real-time graphics techniques.

**Success Criteria:**
- 500 fireflies maintain elevation constraints autonomously
- 5,000 rain particles spawn camera-relative without popping
- Splash particles appear at rain impact points on terrain
- Day/night cycle spans 63 seconds with smooth color transitions

### Objective 5: Architectural Best Practices
Design the codebase using component-based architecture with clear separation of concerns, enabling maintainability, extensibility, and future enhancements. Each system should have well-defined initialization, update, and render phases.

**Success Criteria:**
- Systems are independently testable
- Adding a new system requires minimal changes to core loop
- Code is well-documented and uses consistent naming conventions
- Performance metrics are easily instrumentable

## 2.2 Performance Targets

The following performance targets guide implementation decisions:

| System | Target Performance | Constraint |
|--------|-------------------|-----------|
| Terrain Generation | < 100ms total, one-time | Heightmap: 512×512 |
| Terrain Rendering | < 3ms per frame | VAO-based, single draw call |
| Tree Instancing | < 2ms per frame | 15,000 instances across 4 types |
| Grass Instancing | < 1.5ms per frame | 407,000+ instances, terrain-aligned |
| Water Rendering | < 1ms per frame | Procedural waves, 500×500 grid |
| Firefly Particles | < 0.5ms per frame | 500 particles with height constraints |
| Rain System | < 1ms per frame | 5,000 camera-centered particles |
| Shadow Map Generation | < 3ms per frame | 2048×2048 depth, PCF 3×3 |
| Skybox Rendering | < 0.5ms per frame | Procedural stars and sky |
| Overall Frame Time | < 16.67ms (60 FPS) | Leaves margin for CPU logic |

## 2.3 Scalability Considerations

The design prioritizes scalability in multiple dimensions:

### Horizontal Scalability (Object Count)
- **Instancing enables arbitrary scaling:** Increasing tree count from 5,000 to 50,000 requires only changing a spawn density parameter; performance scales linearly with object count thanks to GPU parallelism.
- **Memory efficient:** Instance data uses 16 bytes per object (vec3 position + padding); 15,000 trees occupy ~250 KB.

### Vertical Scalability (Visual Quality)
- **Shadow quality:** Increasing depth map resolution from 2048 to 4096 improves shadow detail at 2-4× computation cost.
- **Particle count:** Firefly and rain systems scale efficiently; 5,000 particles can increase to 50,000 with minimal performance impact.
- **Grass density:** Spacing parameter controls blade count; denser grass beyond 1.0-unit spacing remains viable.

### Hardware Scalability
- **Target hardware:** Designed for NVIDIA GTX 1660 and AMD RX 5700 XT equivalents
- **Mobile scalability:** Reducing instancing counts by 50% and shadow resolution to 1024×1024 yields viable mobile version
- **High-end scaling:** Doubling all counts and using 4K shadow maps targets 4K capable systems

## 2.4 Constraint Analysis

Several design constraints shape the implementation:

### Memory Constraints
- **GPU VRAM:** Target 2GB maximum GPU memory usage (excludes texture streaming)
- **Heightmap caching:** Loaded once in system memory; sampled by multiple systems without recomputation
- **Draw call minimization:** Limited by GPU command buffer; target < 30 draw calls per frame

### Computational Constraints
- **CPU-bound operations:** Particle updates and spawning logic must complete in < 5ms
- **Shader compilation:** First-time compilation must complete before first draw call
- **Noise generation:** FBM computation limited to startup phase

### Visual Constraints
- **Field-of-view:** Must accommodate terrain seamlessly; shadow map orthographic bounds must cover visible area
- **Pop-in prevention:** Particles and vegetation must spawn smoothly without sudden appearance
- **Shadow acne:** Bias calculation must prevent visual artifacts while maintaining shadow precision

---

## Summary

This section established clear, measurable objectives across performance, scalability, and architectural quality dimensions. The subsequent sections detail how each objective was achieved through specific implementation strategies and algorithm choices.

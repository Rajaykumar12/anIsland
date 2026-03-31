# 6. Environmental Systems Overview

## 6.1 Scope and Integration

Environmental systems bring dynamic life to the scene through water simulation, particle effects, and weather phenomena. Unlike static systems (terrain, buildings), environmental systems exhibit continuous motion and update each frame, requiring careful performance management.

### System Hierarchy

```
Environmental Systems
│
├─ Water System
│  ├─ Procedural wave generation
│  ├─ Fresnel reflection blending
│  └─ Dynamic lighting integration
│
├─ Particle Systems
│  ├─ Firefly system (forest confinement)
│  ├─ Rain particles (weather)
│  └─ Splash system (impact effects)
│
└─ Physics Simulation
   ├─ Height constraint enforcement
   ├─ Particle lifetime management
   └─ Spawn rate modulation
```

## 6.2 Performance Budget

Environmental systems share a collective 4-5 ms performance budget per frame:

```cpp
Frame Budget: 16.67 ms (60 FPS)
├─ Depth pass (3 ms)
├─ Terrain rendering (1.5 ms)
├─ Vegetation (3.5 ms)
│
└─ Environmental Systems (4 ms target)
   ├─ Water (~0.8 ms)
   ├─ Fireflies (~0.5 ms)
   ├─ Rain (~0.6 ms)
   ├─ Splashes (~0.3 ms)
   └─ Physics updates (~1.2 ms CPU)
```

## 6.3 Rendering Order and Blending States

Environmental systems render in specific sequence with appropriate blending modes:

```glsl
// Rendering order within Color Pass

1. Opaque Geometry
   - Terrain
   - Trees
   - Grass
   - Water (GL_BLEND disabled, written to depth)

2. Translucent Systems (GL_BLEND enabled)
   Water (Fresnel-blended)
   │ glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
   │ Writes depth
   │
   Particles (Additive glow)
   │ glBlendFunc(GL_SRC_ALPHA, GL_ONE)
   │ Depth disabled: glDepthMask(GL_FALSE)
   │
   Rain (Semi-transparent)
   │ glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
   │ Depth disabled: glDepthMask(GL_FALSE)
   │
   Splashes (Impact effects)
   │ glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)
   │ Depth disabled: glDepthMask(GL_FALSE)
   │
   Skybox (GL_BLEND disabled, depth at far plane)
      glDepthFunc(GL_LEQUAL)
```

## 6.4 Update-Render Decoupling

Environmental systems use a two-phase update-render pattern:

```cpp
// Main loop structure
while (running) {
    // Phase 1: Update (CPU-side physics)
    lighting.Update(dt);
    particles.Update(dt);           // Update positions, lifetimes
    rain.Update(dt);
    splashes.Update(dt);
    
    // Phase 2: Render (GPU submission)
    // Depth pass
    lighting.RenderDepthPass();
    terrain.Draw();
    treeSystem.Render();
    
    // Color pass
    terrain.Draw();
    water.Render();                 // GPU wave simulation
    treeSystem.Render();
    grassSystem.Render();
    particles.Render();             // GPU point rendering
    rain.Render();
    splashes.Render();
    skybox.Render();
}
```

**Key Principle:** CPU updates particle positions/lifetimes; GPU renders with those updated values. This allows:
- Thousands of particles with CPU-efficient physics
- Efficient 3D-to-screen transformation on GPU
- Frame rate-independent updates via delta-time

---

This section provides architectural context for the three environmental systems detailed in sections 6a, 6b, and 6c.

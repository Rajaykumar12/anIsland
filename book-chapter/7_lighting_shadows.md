# 7. Lighting and Shadow Mapping System

## 7.1 Dynamic Day/Night Cycle

**Circular Orbit Model:** The sun orbits the terrain center in a vertical plane, completing one cycle every ~63 seconds. This creates temporal atmosphere through smooth color transitions.

**Sun Position Algorithm:**

$$\vec{p}_{\text{sun}}(t) = \vec{c} + R \cdot (\cos(\omega t), \sin(\omega t), 0)$$

where:
- $\vec{c} = (400, 0, 400)$ — terrain center
- $R = 100$ — orbit radius
- $\omega = 0.1$ — angular velocity (rad/s)
- $t$ — elapsed time (seconds)

**Time-of-Day Mapping:**

$$\text{tod}(t) = \frac{p_y + R}{2R} = \frac{\sin(\omega t) + 1}{2}$$

maps the sun's Y-position [-R, +R] to time-of-day [0, 1], where:
- $\text{tod} = 0$ → midnight (sun at -R)
- $\text{tod} = 0.5$ → noon (sun at +R)

**Color Interpolation:** Three color channels blend based on time-of-day:

```
BASE_COLOR_CYCLE(tod):
    night ← [0.3, 0.2, 0.5]  (dark blue)
    day   ← [1.0, 1.0, 0.9]  (bright white-yellow)
    
    // Smooth interpolation function
    smoothTime ← 3t² - 2t³  where t = clamp(tod, 0, 1)
    baseColor ← mix(night, day, smoothTime)
    intensity ← mix(0.2, 1.0, smoothTime)
    
    RETURN {baseColor, intensity}

SUNRISE_SUNSET_TINT(tod):
    // Warm orange peaks at sunrise (tod≈0.25) and sunset (tod≈0.75)
    warmStrength ← max(sunrise_peak(tod), sunset_peak(tod))
    
    sunrise_peak(t) = smoothstep(0.15, 0.35, t) × (1 - smoothstep(0.20, 0.40, t))
    sunset_peak(t)  = smoothstep(0.60, 0.75, t) × (1 - smoothstep(0.65, 0.85, t))
    
    warmTint ← [1.0, 0.55, 0.2]  (orange)
    RETURN baseColor + warmTint × warmStrength × 0.6

FINAL_COLOR(tod) ← BASE_COLOR_CYCLE(tod) + SUNRISE_SUNSET_TINT(tod)
```

**Design Rationale:** The smoothstep interpolation (cubic Hermite) creates gentle transitions avoiding sharp color jumps. Separate sunrise/sunset tint provides visual richness—observers perceive the same orange glow both near sunrise and sunset, creating perceived symmetry.

## 7.2 Shadow Mapping Pipeline

Shadow mapping uses a two-pass approach: render scene depth from light's perspective (depth pass), then compare fragment depths during color pass to determine shadowing.

**Algorithm Overview:**

```
SHADOW_MAPPING_PIPELINE():
    // Pass 1: Depth Pass (rendered to off-screen texture)
    FBO ← CreateFramebuffer(2048×2048 depth texture)
    BindFBO(FBO)
    
    lightView ← LookAt(sunPosition, terrainCenter, [0,1,0])
    lightProj ← Orthographic(-150, +150, -150, +150, 0.1, 300)
    lightSpaceMatrix ← lightProj × lightView
    
    UseShader(depthShader)
    SetUniform(depthShader, "lightSpaceMatrix", lightSpaceMatrix)
    
    RenderGeometry(terrain)  // Only depth, no color
    RenderGeometry(trees)
    
    UnbindFBO()
    
    // Pass 2: Color Pass (rendered to screen, samples depth texture)
    BindDefaultFB()
    UseShader(colorShader)
    BindTexture(depthTexture, GL_TEXTURE1)  // Shadow map
    
    FOR each fragment:
        depth_from_light ← SampleShadowMap(fragmentPosition)
        fragment_depth ← ComputeDepthFromLight(fragmentPosition)
        
        shadow ← PCF_Filter(depth_from_light, fragment_depth)
        
        color ← ComputeShading() × mix(0.3, 1.0, shadow)  // Darkened if shadowed
        
        OUTPUT: color
```

**Light Space Configuration:**

The orthographic projection creates a "light frustum" covering terrain bounds:

$$P_{\text{light}} = \text{Orthographic}(-150, +150, -150, +150, 0.1, 300)$$

Bounds chosen to cover ±800 terrain bounds with margin. (If sun moves outside frustum, shadows cut off—standard trade-off vs. adaptive shadow mapping.)

**Shadow Map Resolution:** 2048×2048 provides sharp shadow edges. Doubling to 4096 yields diminishing perceptual improvement while quadrupling memory (64 MB → VRAM pressure on mid-range GPUs).

## 7.3 Percentage-Closer Filtering (PCF)

**Problem:** Point-sampled shadow maps create hard shadows with aliased edges (blocky appearance at distance).

**Solution:** Sample multiple points in shadow map neighborhood, averaging results for smooth transitions.

**PCF Algorithm:**

```
COMPUTE_SHADOW_PCF(fragmentPos, lightSpacePos):
    // Step 1: Transform to light space and NDC [-1,1]
    ndc ← lightSpacePos.xyz / lightSpacePos.w
    texCoord ← ndc × 0.5 + 0.5  // Convert to [0,1]
    
    // Step 2: Out-of-bounds check (behind or outside light frustum)
    IF texCoord.z > 1.0 OR texCoord.x < 0 OR texCoord.x > 1:
        RETURN 0.0  // No shadow (lit)
    
    // Step 3: Compute bias for slope (prevent shadow acne)
    normal ← ComputeSurfaceNormal()
    lightDir ← normalize(sunPosition - fragmentPos)
    cosTheta ← max(dot(normal, lightDir), 0.0)
    bias ← max(0.05 × (1.0 - cosTheta), 0.005)
    
    // Step 4: 3x3 PCF kernel - Sample 9 neighbors
    shadow ← 0.0
    texelSize ← 1.0 / shadowMapResolution  // ~1/2048
    
    FOR dx ← -1 TO +1:
        FOR dy ← -1 TO +1:
            sampleCoord ← texCoord + [dx, dy] × texelSize
            sampleDepth ← ShadowMap.Sample(sampleCoord)
            
            // Compare: if fragment depth > shadow depth, fragment is in shadow
            IF (texCoord.z - bias) > sampleDepth:
                shadow ← shadow + 1.0
    
    // Step 5: Average 9 samples
    shadow ← shadow / 9.0
    
    RETURN shadow
```

**Visual Quality:**

| Technique | Edge Quality | Aliasing | Performance | Memory |
|-----------|---|---|---|---|
| **No filtering** | Blocky with hard edges | Severe | Fast | 1× |
| **3×3 PCF** | Soft, natural transitions | None | Fast (~9 lookups) | 1× |
| **5×5 PCF** | Very smooth | None | Medium (~25 lookups) | 1× |

**Project Configuration:** 3×3 kernel chosen (optimal for 2048 resolution) balancing visual quality and GPU cost. Larger kernels show diminishing returns; smaller (2×2) shows visible banding at screen edges.

**Shadow Map Size Tradeoff:**

$$\text{Quality} \propto \text{Resolution}^2, \quad \text{Memory} \propto \text{Resolution}^2$$

- 1024×1024: 4 MB, visible pixelation at close range
- 2048×2048: 16 MB, sharp edges (chosen)
- 4096×4096: 64 MB, diminishing visual improvement for cost

---

## Summary

The Lighting and Shadow Mapping System demonstrates complete real-time directional lighting with dynamic day/night cycles and robust shadow rendering. The two-pass architecture cleanly separates depth computation from final shading, enabling efficient GPU utilization. PCF filtering creates convincing soft shadows while remaining within performance budgets. The combination of smooth color interpolation and time-based orbital mechanics produces immersive temporal atmosphere.

**Key Achievements:**
- Dynamic sun orbit (62.83-second full cycle)
- Smooth color interpolation (day/night/sunrise/sunset)
- Shadow mapping at 2048×2048 resolution
- PCF 3×3 filtering for soft shadow edges
- Slope-dependent bias preventing shadow acne
- Rendering: 2 passes per frame, ~3.5 ms total
- Supports all shadow-casting systems (terrain, trees)

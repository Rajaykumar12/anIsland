#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 sunPos;

// A simple pseudo-random number generator for procedural stars
float hash(vec3 p) {
    p  = fract( p * 0.3183099 + 0.1 );
    p *= 17.0;
    return fract( p.x * p.y * p.z * (p.x + p.y + p.z) );
}

// Better hash function for smoother randomness
float hash2(vec3 p) {
    float h = dot(p, vec3(0.1031, 0.11369, 0.13787));
    return fract(sin(h) * 43758.5453123);
}

// Generate detailed stars with multiple octaves and properties
void generateStars(vec3 viewDir, float dayIntensity, inout vec3 skyColor) {
    if(viewDir.y <= 0.0) return; // Don't render stars below horizon
    
    float starVisibility = smoothstep(0.5, 0.1, dayIntensity); // Stars fade in as sun sets
    if(starVisibility < 0.01) return; // Early exit if stars are nearly invisible
    
    // Layer 1: Large, bright primary stars
    vec3 gridCoord1 = floor(viewDir * 80.0);
    float starValue1 = hash(gridCoord1);
    if(starValue1 > 0.94) {
        // Create soft glow around primary stars
        vec3 fractCoord = fract(viewDir * 80.0) - 0.5;
        float dist = length(fractCoord);
        float glow = exp(-dist * dist * 15.0); // Soft gaussian falloff
        
        // Color variation: mostly white, some yellow, some blue
        vec3 starColor = vec3(1.0, 1.0, 0.95); // Base white
        float colorRand = hash2(gridCoord1);
        if(colorRand < 0.15) {
            starColor = mix(vec3(1.0, 1.0, 0.95), vec3(1.0, 0.9, 0.6), 0.6); // Yellow
        } else if(colorRand < 0.25) {
            starColor = mix(vec3(1.0, 1.0, 0.95), vec3(0.8, 0.9, 1.0), 0.4); // Slight blue
        }
        
        // Primary star brightness
        float brightness = 0.7 + 0.3 * starValue1;
        skyColor += starColor * brightness * glow * (0.8 + 0.2 * starVisibility);
        
        // Add small twinkle
        skyColor += starColor * 0.15 * sin(gridCoord1.x * 0.5 + gridCoord1.y * 0.3) * glow * starVisibility;
    }
    
    // Layer 2: Medium secondary stars (more frequent)
    vec3 gridCoord2 = floor(viewDir * 180.0);
    float starValue2 = hash(gridCoord2);
    if(starValue2 > 0.97) {
        vec3 fractCoord = fract(viewDir * 180.0) - 0.5;
        float dist = length(fractCoord);
        float glow = exp(-dist * dist * 25.0);
        
        // Secondary stars are dimmer and mostly white
        vec3 starColor = mix(vec3(1.0, 1.0, 1.0), vec3(1.0, 0.95, 0.9), hash2(gridCoord2) * 0.3);
        float brightness = 0.4 + 0.2 * starValue2;
        skyColor += starColor * brightness * glow * 0.7 * starVisibility;
    }
    
    // Layer 3: Faint tertiary stars (many more, very dim)
    vec3 gridCoord3 = floor(viewDir * 300.0);
    float starValue3 = hash(gridCoord3);
    if(starValue3 > 0.985) {
        vec3 fractCoord = fract(viewDir * 300.0) - 0.5;
        float dist = length(fractCoord);
        float glow = exp(-dist * dist * 40.0);
        
        // Tertiary stars are very dim point lights
        vec3 starColor = vec3(1.0);
        float brightness = 0.15 + 0.1 * starValue3;
        skyColor += starColor * brightness * glow * 0.5 * starVisibility;
    }
}

void main()
{
    vec3 viewDir = normalize(TexCoords);
    vec3 sunDir = normalize(sunPos);
    
    // 1. Basic Sky Gradient (Blue at top, light blue at horizon)
    float horizonMix = clamp(viewDir.y + 0.2, 0.0, 1.0);
    vec3 daySky = mix(vec3(0.5, 0.7, 0.9), vec3(0.1, 0.3, 0.6), horizonMix);
    vec3 nightSky = vec3(0.01, 0.01, 0.02);
    
    // How high is the sun? (1.0 = noon, 0.0 = horizon, negative = night)
    float dayIntensity = smoothstep(-0.2, 0.2, sunDir.y);
    
    // Mix day and night based on sun height
    vec3 skyColor = mix(nightSky, daySky, dayIntensity);

    // 2. Sunset Glow (Orange/Red)
    float sunGlow = max(dot(viewDir, sunDir), 0.0);
    float sunsetIntensity = smoothstep(0.0, 0.4, sunDir.y) * smoothstep(0.8, 0.4, sunDir.y);
    vec3 sunsetColor = vec3(0.9, 0.4, 0.1);
    
    // Apply sunset color where the sun is
    skyColor += sunsetColor * pow(sunGlow, 8.0) * sunsetIntensity;

    // 3. The Sun Disk
    if(sunGlow > 0.995) {
        skyColor += vec3(1.0, 1.0, 0.9);
    }

    // 4. Detailed Procedural Stars (multi-octave, colored, with glow)
    generateStars(viewDir, dayIntensity, skyColor);

    FragColor = vec4(skyColor, 1.0);
}

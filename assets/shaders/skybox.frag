#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform vec3 sunPos;
uniform float u_Time;

// Pseudo-random hash functions for stars
float hash(vec3 p) {
    p  = fract( p * 0.3183099 + 0.1 );
    p *= 17.0;
    return fract( p.x * p.y * p.z * (p.x + p.y + p.z) );
}

float hash2(vec3 p) {
    float h = dot(p, vec3(0.1031, 0.11369, 0.13787));
    return fract(sin(h) * 43758.5453123);
}

void generateStars(vec3 viewDir, float dayIntensity, inout vec3 skyColor) {
    if(viewDir.y <= 0.0) return;
    
    float starVisibility = smoothstep(0.5, 0.1, dayIntensity);
    if(starVisibility < 0.01) return;

    // Layer 1: Large, bright primary stars
    vec3 gridCoord1 = floor(viewDir * 80.0);
    float starValue1 = hash(gridCoord1);
    if(starValue1 > 0.94) {
        vec3 fractCoord = fract(viewDir * 80.0) - 0.5;
        float dist = length(fractCoord);
        float glow = exp(-dist * dist * 15.0);
        
        vec3 starColor = vec3(1.0, 1.0, 0.95);
        float colorRand = hash2(gridCoord1);
        if(colorRand < 0.15)       starColor = mix(vec3(1.0,1.0,0.95), vec3(1.0,0.9,0.6), 0.6);
        else if(colorRand < 0.25)  starColor = mix(vec3(1.0,1.0,0.95), vec3(0.8,0.9,1.0), 0.4);
        
        float brightness = 0.7 + 0.3 * starValue1;
        float twinkle = 0.75 + 0.25 * sin(u_Time * 0.85 + gridCoord1.x * 0.5 + gridCoord1.y * 0.3);
        skyColor += starColor * brightness * glow * (0.8 + 0.2 * starVisibility) * twinkle;
        skyColor += starColor * 0.15 * sin(u_Time * 1.1 + gridCoord1.x * 0.5 + gridCoord1.y * 0.3) * glow * starVisibility;
    }
    
    // Layer 2: Medium secondary stars
    vec3 gridCoord2 = floor(viewDir * 180.0);
    float starValue2 = hash(gridCoord2);
    if(starValue2 > 0.97) {
        vec3 fractCoord = fract(viewDir * 180.0) - 0.5;
        float dist = length(fractCoord);
        float glow = exp(-dist * dist * 25.0);
        vec3 starColor = mix(vec3(1.0,1.0,1.0), vec3(1.0,0.95,0.9), hash2(gridCoord2) * 0.3);
        float twinkle = 0.80 + 0.20 * sin(u_Time * 1.3 + gridCoord2.x * 0.23 + gridCoord2.z * 0.41);
        skyColor += starColor * (0.4+0.2*starValue2) * glow * 0.7 * starVisibility * twinkle;
    }
    
    // Layer 3: Faint tertiary stars
    vec3 gridCoord3 = floor(viewDir * 300.0);
    float starValue3 = hash(gridCoord3);
    if(starValue3 > 0.985) {
        vec3 fractCoord = fract(viewDir * 300.0) - 0.5;
        float dist = length(fractCoord);
        float glow = exp(-dist * dist * 40.0);
        float twinkle = 0.85 + 0.15 * sin(u_Time * 1.7 + gridCoord3.x * 0.17 + gridCoord3.y * 0.11);
        skyColor += vec3(1.0) * (0.15+0.1*starValue3) * glow * 0.5 * starVisibility * twinkle;
    }
}

void main()
{
    vec3 viewDir = normalize(TexCoords);
    vec3 sunDir  = normalize(sunPos);

    // 1. Basic Sky Gradient
    float horizonMix = clamp(viewDir.y + 0.2, 0.0, 1.0);
    vec3 daySky   = mix(vec3(0.5,0.7,0.9), vec3(0.1,0.3,0.6), horizonMix);
    vec3 nightSky = vec3(0.01, 0.01, 0.02);

    float dayIntensity = smoothstep(-0.2, 0.2, sunDir.y);
    vec3 skyColor = mix(nightSky, daySky, dayIntensity);

    // 2. Sunset Glow (Orange/Red)
    float sunGlow        = max(dot(viewDir, sunDir), 0.0);
    float sunsetIntensity = smoothstep(0.0, 0.4, sunDir.y) * smoothstep(0.8, 0.4, sunDir.y);
    skyColor += vec3(0.9,0.4,0.1) * pow(sunGlow, 8.0) * sunsetIntensity;

    // 3. The Sun Disk
    if(sunGlow > 0.995) {
        skyColor += vec3(1.0, 1.0, 0.9);
    }

    // 4. Moon (opposite sun, visible at night)
    vec3 moonDir = -sunDir;
    float moonDot  = dot(viewDir, moonDir);
    // Moon disk
    float moonDisk = smoothstep(0.9980, 1.0000, moonDot);
    // Soft halo around moon
    float moonHalo = smoothstep(0.985, 0.9980, moonDot) * 0.12;
    float nightFactor = 1.0 - smoothstep(0.0, 0.35, dayIntensity);
    if(moonDot > 0.985 && viewDir.y > 0.0) {
        // Moon only above horizon
        skyColor += vec3(0.90, 0.93, 1.00) * moonDisk * nightFactor;
        skyColor += vec3(0.40, 0.45, 0.60) * moonHalo * nightFactor;
    }

    // 5. Detailed Procedural Stars
    generateStars(viewDir, dayIntensity, skyColor);

    FragColor = vec4(skyColor, 1.0);
}

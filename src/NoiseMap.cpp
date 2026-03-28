#include "NoiseMap.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

namespace {
float smoothstep(float edge0, float edge1, float x) {
    float t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}
} // namespace

int NoiseMap::perm[512] = {};

std::vector<float> NoiseMap::generate(int width, int height,
                                      int   octaves,
                                      float persistence,
                                      float lacunarity,
                                      float scale,
                                      unsigned int seed)
{
    initPermutation(seed);
    std::vector<float> map(width * height);

    float maxPossible = 0.0f;
    float amp = 1.0f;
    for (int o = 0; o < octaves; ++o) {
        maxPossible += amp;
        amp *= persistence;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float amplitude  = 1.0f;
            float frequency  = 1.0f;
            float noiseValue = 0.0f;

            for (int o = 0; o < octaves; ++o) {
                float sx = (float)x / scale * frequency;
                float sy = (float)y / scale * frequency;
                noiseValue += perlin(sx, sy) * amplitude;
                amplitude  *= persistence;
                frequency  *= lacunarity;
            }

            // Normalize to [0, 1]
            float normalized = (noiseValue / maxPossible) * 0.5f + 0.5f;

            // Radial base for island shape
            float cx = (width - 1) * 0.5f;
            float cy = (height - 1) * 0.5f;
            float dx = (x - cx) / cx;
            float dy = (y - cy) / cy;
            float dist = std::sqrt(dx * dx + dy * dy);

            // Layered coast noise for irregular, natural shoreline shape
            float coastMacro  = perlin(x * 0.012f + 37.2f, y * 0.012f - 11.4f);
            float coastDetail = perlin(x * 0.045f - 53.1f, y * 0.045f + 8.7f);
            float jitter = coastMacro * 0.10f + coastDetail * 0.035f;

            float shoreStart = 0.58f + jitter;
            float shoreEnd   = 0.97f + jitter;
            float falloff = 1.0f - smoothstep(shoreStart, shoreEnd, dist);

            // Push outer ring underwater so terrain border is hidden by ocean
            float edgeSubmerge = smoothstep(0.55f, 0.95f, dist);
            float heightValue = normalized * falloff - 0.09f * edgeSubmerge;

            // Preserve tall inland terrain while allowing shallow underwater edge
            map[y * width + x] = std::clamp(heightValue, -0.15f, 1.0f);
        }
    }
    return map;
}

void NoiseMap::initPermutation(unsigned int seed) {
    std::vector<int> p(256);
    std::iota(p.begin(), p.end(), 0);
    std::mt19937 rng(seed);
    std::shuffle(p.begin(), p.end(), rng);
    for (int i = 0; i < 256; ++i) perm[i] = perm[i + 256] = p[i];
}

float NoiseMap::fade(float t) { 
    return t * t * t * (t * (t * 6 - 15) + 10); 
}

float NoiseMap::lerp(float a, float b, float t) { 
    return a + t * (b - a); 
}

float NoiseMap::grad(int hash, float x, float y) {
    int h = hash & 7;
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float NoiseMap::perlin(float x, float y) {
    int X = (int)std::floor(x) & 255;
    int Y = (int)std::floor(y) & 255;
    x -= std::floor(x);
    y -= std::floor(y);
    float u = fade(x), v = fade(y);

    int aa = perm[perm[X  ] + Y  ];
    int ab = perm[perm[X  ] + Y+1];
    int ba = perm[perm[X+1] + Y  ];
    int bb = perm[perm[X+1] + Y+1];

    return lerp(lerp(grad(aa, x,   y  ), grad(ba, x-1, y  ), u),
                lerp(grad(ab, x,   y-1), grad(bb, x-1, y-1), u), v);
}

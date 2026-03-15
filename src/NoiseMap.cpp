#include "NoiseMap.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

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
            map[y * width + x] = (noiseValue / maxPossible) * 0.5f + 0.5f;
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

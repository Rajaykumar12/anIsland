#pragma once
#include <vector>

class NoiseMap {
public:
    static std::vector<float> generate(int width, int height,
                                       int   octaves     = 6,
                                       float persistence = 0.5f,
                                       float lacunarity  = 2.0f,
                                       float scale       = 50.0f,
                                       unsigned int seed = 42);

private:
    static int perm[512];
    static void initPermutation(unsigned int seed);
    static float fade(float t);
    static float lerp(float a, float b, float t);
    static float grad(int hash, float x, float y);
    static float perlin(float x, float y);
};

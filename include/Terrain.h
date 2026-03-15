#pragma once
#include <vector>
#include <glad/glad.h>

class Terrain {
public:
    Terrain(int width, int depth);
    void Draw();

private:
    unsigned int VAO, VBO, EBO;
    int indexCount;
    void setupMesh(int width, int depth);
};
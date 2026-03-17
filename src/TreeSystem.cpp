#include "TreeSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cstdlib>

TreeSystem::TreeSystem(const std::vector<float>& noiseData, int hmWidth, int hmHeight,
                       int terrainWidth, int terrainHeight) {
    SetupGeometry();
    SetupInstances(noiseData, hmWidth, hmHeight, terrainWidth, terrainHeight);
}

TreeSystem::~TreeSystem() {
    for (auto& type : treeTypes) {
        glDeleteVertexArrays(1, &type.VAO);
        glDeleteBuffers(1, &type.VBO);
        glDeleteBuffers(1, &type.instanceVBO);
    }
}

void TreeSystem::CreatePineTree() {
    // Tall, narrow, pointed tree (classic pine/spruce)
    float vertices[] = {
        // --- Trunk (Brown) ---
        -0.15f, 0.0f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.6f, 0.35f, 0.15f,
         0.15f, 0.0f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.6f, 0.35f, 0.15f,
         0.0f, 2.5f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.0f, 0.0f,   0.65f, 0.38f, 0.18f,
         
        0.0f, 0.0f, -0.15f,   0.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.6f, 0.35f, 0.15f,
        0.0f, 0.0f,  0.15f,   1.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.6f, 0.35f, 0.15f,
        0.0f, 2.5f,  0.0f,   0.5f, 1.0f,   0.0f,  0.0f, -1.0f,  0.65f, 0.38f, 0.18f,

        // --- Canopy 1 (Wide bottom) ---
        -1.8f, 1.2f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.35f, 0.75f, 0.25f,
         1.8f, 1.2f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.35f, 0.75f, 0.25f,
         0.0f, 4.5f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.38f, 0.80f, 0.28f,

        0.0f, 1.2f, -1.8f,    0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.32f, 0.70f, 0.20f,
        0.0f, 1.2f,  1.8f,    1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.32f, 0.70f, 0.20f,
        0.0f, 4.5f,  0.0f,    0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.35f, 0.75f, 0.23f,
        
        // --- Canopy 2 (Narrow middle) ---
        -1.3f, 2.5f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.33f, 0.72f, 0.22f,
         1.3f, 2.5f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.33f, 0.72f, 0.22f,
         0.0f, 5.5f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.36f, 0.78f, 0.26f,

        0.0f, 2.5f, -1.3f,    0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.30f, 0.65f, 0.18f,
        0.0f, 2.5f,  1.3f,    1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.30f, 0.65f, 0.18f,
        0.0f, 5.5f,  0.0f,    0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.33f, 0.72f, 0.22f,

        // --- Canopy 3 (Pointed top) ---
        -0.7f, 4.0f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.38f, 0.82f, 0.30f,
         0.7f, 4.0f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.38f, 0.82f, 0.30f,
         0.0f, 6.2f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.40f, 0.85f, 0.32f,

        0.0f, 4.0f, -0.7f,    0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.35f, 0.78f, 0.26f,
        0.0f, 4.0f,  0.7f,    1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.35f, 0.78f, 0.26f,
        0.0f, 6.2f,  0.0f,    0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.37f, 0.80f, 0.28f,
    };
    int vertexCount = sizeof(vertices) / (11 * sizeof(float));

    unsigned int VAO, VBO, instanceVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    int stride = 11 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    TreeType treeType;
    treeType.VAO = VAO;
    treeType.VBO = VBO;
    treeType.instanceVBO = instanceVBO;
    treeType.vertexCount = vertexCount;
    treeTypes.push_back(treeType);
}

void TreeSystem::CreateOakTree() {
    // Round, wide canopy, shorter
    float vertices[] = {
        // --- Trunk (Thicker) ---
        -0.25f, 0.0f, 0.0f,   0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.62f, 0.36f, 0.16f,
         0.25f, 0.0f, 0.0f,   1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.62f, 0.36f, 0.16f,
         0.0f, 1.8f, 0.0f,   0.5f, 1.0f,   -1.0f, 0.0f, 0.0f,   0.67f, 0.39f, 0.19f,
         
        0.0f, 0.0f, -0.25f,  0.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.62f, 0.36f, 0.16f,
        0.0f, 0.0f,  0.25f,  1.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.62f, 0.36f, 0.16f,
        0.0f, 1.8f,  0.0f,  0.5f, 1.0f,   0.0f,  0.0f, -1.0f,  0.67f, 0.39f, 0.19f,

        // --- Wide Canopy (Hemispherical) ---
        -2.0f, 0.8f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.2f, 0.0f,   0.37f, 0.77f, 0.26f,
         2.0f, 0.8f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.2f, 0.0f,   0.37f, 0.77f, 0.26f,
         0.0f, 3.5f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.2f, 0.0f,   0.40f, 0.82f, 0.30f,

        0.0f, 0.8f, -2.0f,    0.0f, 0.0f,   0.0f,  0.2f, -1.0f,  0.34f, 0.72f, 0.21f,
        0.0f, 0.8f,  2.0f,    1.0f, 0.0f,   0.0f,  0.2f, -1.0f,  0.34f, 0.72f, 0.21f,
        0.0f, 3.5f,  0.0f,    0.5f, 1.0f,   0.0f,  0.2f, -1.0f,  0.37f, 0.77f, 0.26f,

        // --- Top (Rounded) ---
        -1.2f, 2.2f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.36f, 0.76f, 0.25f,
         1.2f, 2.2f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.36f, 0.76f, 0.25f,
         0.0f, 4.0f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.39f, 0.81f, 0.29f,

        0.0f, 2.2f, -1.2f,    0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.33f, 0.71f, 0.20f,
        0.0f, 2.2f,  1.2f,    1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.33f, 0.71f, 0.20f,
        0.0f, 4.0f,  0.0f,    0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.36f, 0.76f, 0.25f,
    };
    int vertexCount = sizeof(vertices) / (11 * sizeof(float));

    unsigned int VAO, VBO, instanceVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    int stride = 11 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    TreeType treeType;
    treeType.VAO = VAO;
    treeType.VBO = VBO;
    treeType.instanceVBO = instanceVBO;
    treeType.vertexCount = vertexCount;
    treeTypes.push_back(treeType);
}

void TreeSystem::CreateBirchTree() {
    // Thin trunk, small delicate foliage
    float vertices[] = {
        // --- Thin Trunk ---
        -0.1f, 0.0f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.85f, 0.80f, 0.75f,
         0.1f, 0.0f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.85f, 0.80f, 0.75f,
         0.0f, 3.0f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.0f, 0.0f,   0.88f, 0.83f, 0.78f,
         
        0.0f, 0.0f, -0.1f,    0.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.85f, 0.80f, 0.75f,
        0.0f, 0.0f,  0.1f,    1.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.85f, 0.80f, 0.75f,
        0.0f, 3.0f,  0.0f,    0.5f, 1.0f,   0.0f,  0.0f, -1.0f,  0.88f, 0.83f, 0.78f,

        // --- Small Delicate Canopy ---
        -1.0f, 1.5f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.40f, 0.80f, 0.30f,
         1.0f, 1.5f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.40f, 0.80f, 0.30f,
         0.0f, 4.2f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.42f, 0.83f, 0.32f,

        0.0f, 1.5f, -1.0f,    0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.38f, 0.76f, 0.26f,
        0.0f, 1.5f,  1.0f,    1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.38f, 0.76f, 0.26f,
        0.0f, 4.2f,  0.0f,    0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.40f, 0.80f, 0.30f,
    };
    int vertexCount = sizeof(vertices) / (11 * sizeof(float));

    unsigned int VAO, VBO, instanceVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    int stride = 11 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    TreeType treeType;
    treeType.VAO = VAO;
    treeType.VBO = VBO;
    treeType.instanceVBO = instanceVBO;
    treeType.vertexCount = vertexCount;
    treeTypes.push_back(treeType);
}

void TreeSystem::CreateFirTree() {
    // Dense, conical, very thick foliage
    float vertices[] = {
        // --- Trunk ---
        -0.2f, 0.0f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.58f, 0.32f, 0.14f,
         0.2f, 0.0f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.0f, 0.0f,   0.58f, 0.32f, 0.14f,
         0.0f, 2.2f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.0f, 0.0f,   0.63f, 0.36f, 0.17f,
         
        0.0f, 0.0f, -0.2f,    0.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.58f, 0.32f, 0.14f,
        0.0f, 0.0f,  0.2f,    1.0f, 0.0f,   0.0f,  0.0f, -1.0f,  0.58f, 0.32f, 0.14f,
        0.0f, 2.2f,  0.0f,    0.5f, 1.0f,   0.0f,  0.0f, -1.0f,  0.63f, 0.36f, 0.17f,

        // --- Dense Bottom Layer ---
        -2.0f, 0.5f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.30f, 0.68f, 0.18f,
         2.0f, 0.5f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.30f, 0.68f, 0.18f,
         0.0f, 4.2f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.33f, 0.72f, 0.22f,

        0.0f, 0.5f, -2.0f,    0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.28f, 0.63f, 0.15f,
        0.0f, 0.5f,  2.0f,    1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.28f, 0.63f, 0.15f,
        0.0f, 4.2f,  0.0f,    0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.30f, 0.68f, 0.18f,

        // --- Middle Dense Layer ---
        -1.4f, 2.0f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.32f, 0.70f, 0.20f,
         1.4f, 2.0f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.32f, 0.70f, 0.20f,
         0.0f, 5.0f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.35f, 0.75f, 0.24f,

        0.0f, 2.0f, -1.4f,    0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.31f, 0.66f, 0.18f,
        0.0f, 2.0f,  1.4f,    1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.31f, 0.66f, 0.18f,
        0.0f, 5.0f,  0.0f,    0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.33f, 0.71f, 0.21f,

        // --- Pointed Top Layer ---
        -0.8f, 3.5f, 0.0f,    0.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.34f, 0.74f, 0.22f,
         0.8f, 3.5f, 0.0f,    1.0f, 0.0f,   -1.0f, 0.3f, 0.0f,   0.34f, 0.74f, 0.22f,
         0.0f, 5.8f, 0.0f,    0.5f, 1.0f,   -1.0f, 0.3f, 0.0f,   0.36f, 0.77f, 0.26f,

        0.0f, 3.5f, -0.8f,    0.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.32f, 0.69f, 0.19f,
        0.0f, 3.5f,  0.8f,    1.0f, 0.0f,   0.0f,  0.3f, -1.0f,  0.32f, 0.69f, 0.19f,
        0.0f, 5.8f,  0.0f,    0.5f, 1.0f,   0.0f,  0.3f, -1.0f,  0.34f, 0.73f, 0.22f,
    };
    int vertexCount = sizeof(vertices) / (11 * sizeof(float));

    unsigned int VAO, VBO, instanceVBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    int stride = 11 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);
    glBindVertexArray(0);

    TreeType treeType;
    treeType.VAO = VAO;
    treeType.VBO = VBO;
    treeType.instanceVBO = instanceVBO;
    treeType.vertexCount = vertexCount;
    treeTypes.push_back(treeType);
}

void TreeSystem::SetupGeometry() {
    CreatePineTree();
    CreateOakTree();
    CreateBirchTree();
    CreateFirTree();
}

void TreeSystem::SetupInstances(const std::vector<float>& noiseData, int hmWidth, int hmHeight,
                                int terrainWidth, int terrainHeight) {
    const int totalTreeCount = 40000;
    const float TERRAIN_MAX_HEIGHT = 150.0f;
    srand(42);

    std::vector<std::vector<glm::vec3>> typeInstances(treeTypes.size());
    for (auto& vec : typeInstances) {
        vec.reserve(totalTreeCount / treeTypes.size());
    }

    for (int i = 0; i < 100000 && GetTotalTreeCount() < totalTreeCount; ++i) {
        float x = (float)(rand() % (terrainWidth - 2)) + 1.0f;
        float z = (float)(rand() % (terrainHeight - 2)) + 1.0f;

        int px = (int)((x / terrainWidth) * hmWidth);
        int pz = (int)((z / terrainHeight) * hmHeight);
        px = glm::clamp(px, 0, hmWidth - 1);
        pz = glm::clamp(pz, 0, hmHeight - 1);

        float h = noiseData[pz * hmWidth + px];
        float worldY = h * TERRAIN_MAX_HEIGHT;

        if (worldY < 5.0f || worldY > 85.0f) continue;

        int treeType = 0;
        if (worldY < 25.0f) {
            // Lowlands: Birch, Oak, Fir (0, 1, 3)
            int choice = rand() % 3;
            if (choice == 0) treeType = 2;      // Birch
            else if (choice == 1) treeType = 1; // Oak
            else treeType = 3;                  // Fir
        } else if (worldY < 50.0f) {
            // Midlands: All types equally
            treeType = rand() % 4;
        } else {
            // Highlands: Pine and Fir (tall trees)
            treeType = (rand() % 2 == 0) ? 0 : 3;
        }

        typeInstances[treeType].push_back(glm::vec3(x, worldY, z));
    }

    // Setup instance buffers for each tree type
    for (size_t i = 0; i < treeTypes.size(); ++i) {
        glBindVertexArray(treeTypes[i].VAO);
        glBindBuffer(GL_ARRAY_BUFFER, treeTypes[i].instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, typeInstances[i].size() * sizeof(glm::vec3),
                     typeInstances[i].data(), GL_STATIC_DRAW);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribDivisor(4, 1);
        glBindVertexArray(0);

        treeTypes[i].instances = typeInstances[i];
    }
}

int TreeSystem::GetTotalTreeCount() const {
    int total = 0;
    for (const auto& type : treeTypes) {
        total += type.instances.size();
    }
    return total;
}

void TreeSystem::Render(const glm::mat4& projection, const glm::mat4& view,
                        const glm::vec3& skyColor, Shader& shader) {
    glDisable(GL_CULL_FACE);
    
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("skyColor", skyColor);

    for (const auto& type : treeTypes) {
        glBindVertexArray(type.VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, type.vertexCount, type.instances.size());
    }
    glBindVertexArray(0);

    glEnable(GL_CULL_FACE);
}

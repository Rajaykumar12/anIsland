#include "NPCSystem.h"
#include "Shader.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <vector>

NPCSystem::NPCSystem() {
    SetupGeometry();
    SetupInstances();
}

NPCSystem::~NPCSystem() {
    glDeleteVertexArrays(1, &personVAO);
    glDeleteBuffers(1, &personVBO);
    glDeleteBuffers(1, &personInstanceVBO);
}

void NPCSystem::SetupGeometry() {
    std::vector<float> personVertices;
    personVertices.reserve(9 * 36 * 6);

    auto pushVertex = [&](const glm::vec3& pos, const glm::vec3& normal, const glm::vec3& color) {
        personVertices.push_back(pos.x);
        personVertices.push_back(pos.y);
        personVertices.push_back(pos.z);
        personVertices.push_back(normal.x);
        personVertices.push_back(normal.y);
        personVertices.push_back(normal.z);
        personVertices.push_back(color.r);
        personVertices.push_back(color.g);
        personVertices.push_back(color.b);
    };

    auto addQuad = [&](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& d,
                       const glm::vec3& normal, const glm::vec3& color) {
        pushVertex(a, normal, color);
        pushVertex(b, normal, color);
        pushVertex(c, normal, color);
        pushVertex(a, normal, color);
        pushVertex(c, normal, color);
        pushVertex(d, normal, color);
    };

    auto addBox = [&](const glm::vec3& minP, const glm::vec3& maxP, const glm::vec3& color) {
        glm::vec3 v000(minP.x, minP.y, minP.z);
        glm::vec3 v001(minP.x, minP.y, maxP.z);
        glm::vec3 v010(minP.x, maxP.y, minP.z);
        glm::vec3 v011(minP.x, maxP.y, maxP.z);
        glm::vec3 v100(maxP.x, minP.y, minP.z);
        glm::vec3 v101(maxP.x, minP.y, maxP.z);
        glm::vec3 v110(maxP.x, maxP.y, minP.z);
        glm::vec3 v111(maxP.x, maxP.y, maxP.z);

        addQuad(v000, v100, v110, v010, glm::vec3(0.0f, 0.0f, -1.0f), color); // front
        addQuad(v101, v001, v011, v111, glm::vec3(0.0f, 0.0f, 1.0f), color);  // back
        addQuad(v001, v000, v010, v011, glm::vec3(-1.0f, 0.0f, 0.0f), color); // left
        addQuad(v100, v101, v111, v110, glm::vec3(1.0f, 0.0f, 0.0f), color);  // right
        addQuad(v010, v110, v111, v011, glm::vec3(0.0f, 1.0f, 0.0f), color);  // top
        addQuad(v001, v101, v100, v000, glm::vec3(0.0f, -1.0f, 0.0f), color); // bottom
    };

    const glm::vec3 skin(1.0f, 0.8f, 0.6f);
    const glm::vec3 shirt(0.82f, 0.24f, 0.24f);
    const glm::vec3 sleeves(0.25f, 0.35f, 0.85f);
    const glm::vec3 pants(0.45f, 0.34f, 0.22f);

    addBox(glm::vec3(-0.18f, 1.80f, -0.18f), glm::vec3(0.18f, 2.20f, 0.18f), skin);      // head
    addBox(glm::vec3(-0.32f, 1.00f, -0.18f), glm::vec3(0.32f, 1.80f, 0.18f), shirt);      // torso
    addBox(glm::vec3(-0.50f, 1.00f, -0.12f), glm::vec3(-0.34f, 1.70f, 0.12f), sleeves);   // left arm
    addBox(glm::vec3(0.34f, 1.00f, -0.12f), glm::vec3(0.50f, 1.70f, 0.12f), sleeves);     // right arm
    addBox(glm::vec3(-0.20f, 0.00f, -0.14f), glm::vec3(-0.02f, 1.00f, 0.14f), pants);     // left leg
    addBox(glm::vec3(0.02f, 0.00f, -0.14f), glm::vec3(0.20f, 1.00f, 0.14f), pants);       // right leg

    personVertexCount = static_cast<int>(personVertices.size() / 9);

    glGenVertexArrays(1, &personVAO);
    glGenBuffers(1, &personVBO);
    glGenBuffers(1, &personInstanceVBO);

    glBindVertexArray(personVAO);
    glBindBuffer(GL_ARRAY_BUFFER, personVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 static_cast<GLsizeiptr>(personVertices.size() * sizeof(float)),
                 personVertices.data(),
                 GL_STATIC_DRAW);

    // Position (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // Color (location 2)
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void NPCSystem::SetupInstances() {
    npcs.clear();

    // Original walking NPCs - disabled for cinematic mode
    // We can optionally add these back if needed
    srand(99);
    for (int i = 0; i < 0; i++) {  // Disabled - set to 30 to re-enable
        NPCInstance npc;
        float bx = 110.0f + (rand() % 30);
        float bz = 155.0f + (rand() % 40);
        npc.basePos = glm::vec3(bx, 10.0f, bz);
        npc.position = npc.basePos;
        npc.rotation = glm::vec3(0.0f);
        npc.pose = NPCPoseType::WALKING;
        npc.isCinematic = false;
        npc.patrolRadius = 5.0f + (rand() % 10);
        npc.patrolPhase = (float)(rand() % 628) / 100.0f;
        npc.visibleStart = 0.0f;
        npc.visibleEnd = 9999.0f;
        npc.isVisible = true;
        npcs.push_back(npc);
    }

    // Setup instance buffer
    glBindVertexArray(personVAO);
    glBindBuffer(GL_ARRAY_BUFFER, personInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, npcs.size() * sizeof(glm::vec3),
                 nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);
    glBindVertexArray(0);
}

void NPCSystem::AddCinematicNPC(const glm::vec3& pos, const glm::vec3& rot, NPCPoseType pose,
                                 float visibleStart, float visibleEnd) {
    NPCInstance npc;
    npc.position = pos;
    npc.rotation = rot;
    npc.pose = pose;
    npc.isCinematic = true;
    npc.basePos = pos;
    npc.patrolRadius = 0.0f;
    npc.patrolPhase = 0.0f;
    npc.visibleStart = visibleStart;
    npc.visibleEnd = visibleEnd;
    npc.isVisible = false;
    npcs.push_back(npc);

    UpdateInstanceBuffer();
}

void NPCSystem::ClearCinematicNPCs() {
    // Remove cinematic NPCs, keep walking ones
    npcs.erase(
        std::remove_if(npcs.begin(), npcs.end(),
            [](const NPCInstance& npc) { return npc.isCinematic; }),
        npcs.end());
    UpdateInstanceBuffer();
}

void NPCSystem::UpdateInstanceBuffer() {
    glBindBuffer(GL_ARRAY_BUFFER, personInstanceVBO);
    glBufferData(GL_ARRAY_BUFFER, npcs.size() * sizeof(glm::vec3),
                 nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void NPCSystem::Update(float currentTime) {
    std::vector<glm::vec3> positions;
    positions.reserve(npcs.size());

    for (auto& npc : npcs) {
        if (npc.isCinematic) {
            // Cinematic NPCs: check visibility window and apply pose
            npc.isVisible = (currentTime >= npc.visibleStart && currentTime < npc.visibleEnd);

            // For prone pose, adjust position to lie flat
            if (npc.pose == NPCPoseType::PRONE) {
                // Already positioned correctly via rotation
            }

            if (npc.isVisible) {
                positions.push_back(npc.position);
            }
        } else {
            // Walking NPCs: patrol animation
            float angle = currentTime * 0.25f + npc.patrolPhase;
            float r = npc.patrolRadius;
            npc.position.x = npc.basePos.x + r * std::cos(angle);
            npc.position.z = npc.basePos.z + r * std::sin(angle);
            npc.position.y = npc.basePos.y;
            positions.push_back(npc.position);
        }
    }

    // Upload updated positions to GPU
    if (!positions.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, personInstanceVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0,
                        positions.size() * sizeof(glm::vec3),
                        positions.data());
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void NPCSystem::Render(const glm::mat4& projection, const glm::mat4& view,
                       const glm::vec3& skyColor, Shader& shader, float currentTime) {
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setVec3("skyColor", skyColor);

    // Build list of visible NPCs with their data
    struct RenderNPC {
        glm::vec3 position;
        glm::vec3 rotation;
        NPCPoseType pose;
        bool isCinematic;
    };
    std::vector<RenderNPC> visibleNPCs;

    for (const auto& npc : npcs) {
        if (npc.isCinematic) {
            if (currentTime >= npc.visibleStart && currentTime < npc.visibleEnd) {
                visibleNPCs.push_back({npc.position, npc.rotation, npc.pose, npc.isCinematic});
            }
        } else {
            visibleNPCs.push_back({npc.position, npc.rotation, npc.pose, npc.isCinematic});
        }
    }

    // Render each visible NPC
    glBindVertexArray(personVAO);

    for (const auto& npc : visibleNPCs) {
        if (npc.isCinematic) {
            // Cinematic NPCs: use model matrix with custom rotation
            shader.setBool("useModelMatrix", true);
            shader.setFloat("u_Time", 0.0f);  // Static poses - no animation

            // Build model matrix with rotation based on pose
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, npc.position);

            // Apply rotations (convert from degrees to radians)
            model = glm::rotate(model, glm::radians(npc.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(npc.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(npc.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

            // Adjust scale/position based on pose
            if (npc.pose == NPCPoseType::PRONE) {
                // Flatten for prone position
                model = glm::scale(model, glm::vec3(1.0f, 0.3f, 1.0f));
            } else if (npc.pose == NPCPoseType::SITTING) {
                // Squat for sitting position
                model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
                model = glm::scale(model, glm::vec3(1.0f, 0.7f, 1.0f));
            }

            shader.setMat4("model", model);

            // Draw single instance (without instance buffer)
            glDrawArrays(GL_TRIANGLES, 0, personVertexCount);
        } else {
            // Walking NPCs: use instanced rendering
            shader.setBool("useModelMatrix", false);
            shader.setFloat("u_Time", currentTime);

            // For now, skip walking NPCs in cinematic mode
            // They would need proper instance buffer updates
        }
    }

    glBindVertexArray(0);
}

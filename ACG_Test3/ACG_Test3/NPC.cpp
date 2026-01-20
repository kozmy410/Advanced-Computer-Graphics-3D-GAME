#include "NPC.hpp"

// Constructor implementation
NPC::NPC(glm::vec3 pos, const std::string& modelPath, const std::string& iconPath)
    : GameObject(pos, modelPath, iconPath)
{
    // Set a default scale. Adjust this (e.g., 0.05f, 0.1f, 1.0f) 
    // depending on how big your downloaded OBJ file is.
    scale = glm::vec3(0.05f);
}

// Update implementation
void NPC::update(float deltaTime, const Camera& camera) {
    // Currently, NPCs do nothing (idle).
    // If you want them to rotate or walk, put that logic here later.

    // Example: Make NPC slowly rotate
    // rotation.y += 10.0f * deltaTime;
}
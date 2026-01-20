#pragma once
#include "GameObject.hpp"

class NPC : public GameObject {
public:
    // Constructor takes Position, 3D Model path, and 2D Icon path
    NPC(glm::vec3 pos, const std::string& modelPath, const std::string& iconPath);

    // Update now takes the Camera (matching the base class change)
    void update(float deltaTime, const Camera& camera) override;
};
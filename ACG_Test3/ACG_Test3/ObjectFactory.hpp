#pragma once
#include "GameObject.hpp" 
#include "CollisionManager.hpp"
#include <string>
#include <memory>
#include <vector>
#include "dependencies/glm-1.0.2/glm/glm.hpp"

// a simple settings box for collision, so we don't have to pass messy arguments later
struct ColliderConfig {
    PhysicsType type = PhysicsType::NONE;
    glm::vec3 offset = glm::vec3(0.0f);   // shifts the hitbox relative to the model
    glm::vec3 customSize = glm::vec3(0.0f);

    ColliderConfig(PhysicsType t = PhysicsType::NONE) : type(t) {}
};

class ObjectFactory {
public:
    // static factory method: creates a game object, sets up its physics, and hands us ownership (unique_ptr)
    static std::unique_ptr<GameObject> CreateObject(
        glm::vec3 position,
        glm::vec3 scale,
        const std::string& modelPath,
        const std::string& iconName,
        ColliderConfig collider,
        const std::vector<std::string>& texturePaths = {} // defaults to no extra textures
    );
};
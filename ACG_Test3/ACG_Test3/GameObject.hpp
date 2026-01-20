#pragma once
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include <string>

class GameObject {
public:
    
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    
    std::string texturePath;

    virtual ~GameObject() = default;
    virtual void update(float deltaTime) {}

protected:
    
    GameObject(glm::vec3 pos, const std::string& texPath)
        : position(pos), rotation(glm::vec3(0.0f)), scale(glm::vec3(1.0f)), texturePath(texPath) {
    }
};
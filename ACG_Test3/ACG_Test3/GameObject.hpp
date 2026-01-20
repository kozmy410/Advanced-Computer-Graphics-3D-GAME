#pragma once

#include <string>
#include <memory>
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtc/matrix_transform.hpp"

#include "Model.hpp"
#include "Shader.hpp"

// Forward declaration so GameObject knows 'Camera' exists without full include
class Camera;

class GameObject {
public:
    glm::vec3 position;
    glm::vec3 rotation; // Degrees
    glm::vec3 scale;
    std::unique_ptr<Model> model3D;
    std::string iconPath;

    GameObject(glm::vec3 pos, const std::string& modelPath, const std::string& iconPath2D)
        : position(pos), rotation(0.0f), scale(1.0f), iconPath(iconPath2D)
    {
        if (!modelPath.empty()) model3D = std::make_unique<Model>(modelPath);
    }

    GameObject(glm::vec3 pos, std::unique_ptr<Model> model, const std::string& iconPath2D)
        : position(pos), rotation(0.0f), scale(1.0f), iconPath(iconPath2D)
    {
        model3D = std::move(model);
    }

    virtual ~GameObject() = default;

    // CHANGED: Now takes Camera so objects know where "Forward" is relative to view
    virtual void update(float deltaTime, const Camera& camera) {}

    void draw3D(Shader& shader) {
        if (!model3D) return;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);

        // DEGREES directly as requested
        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0, 0, 1));

        model = glm::scale(model, scale);
        shader.setMat4("model", model);
        model3D->Draw(shader);
    }
};
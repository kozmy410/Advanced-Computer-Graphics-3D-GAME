#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Model.hpp"
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtc/matrix_transform.hpp"

// telling the compiler these classes exist so we can refer to them here
class Shader;
class Camera;

// defining the different shapes used for physics collisions
enum class PhysicsType {
    NONE,   // no collision, ghost object
    BOX,    // simple cube collider
    SPHERE  // simple ball collider
};

class GameObject {
public:
    // the 3d mesh data (loaded from a file)
    std::unique_ptr<Model> model;

    // transform properties
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    // string used for icons or identifying the object type
    std::string iconPath;

    // physics engine properties
    int physicsID = -1; // -1 means no physics
    PhysicsType physicsType = PhysicsType::NONE;
    glm::vec3 originalColliderSize = glm::vec3(0.0f); // size before rotation
    glm::vec3 colliderOffset = glm::vec3(0.0f); // offset from the center

    // pbr material settings (how surface interacts with light)
    float pbr_metallic = 0.0f;
    float pbr_roughness = 0.5f;
    float pbr_ao = 1.0f;

    // properties if this object acts as a light source
    glm::vec3 lightColor = glm::vec3(1.0f);
    float lightIntensity = 0.0f;

    // constructor: setting up the basics
    GameObject(glm::vec3 pos, std::unique_ptr<Model> m, std::string icon)
        : position(pos), model(std::move(m)), iconPath(icon), rotation(0.0f), scale(1.0f) {
    }

    // virtual destructor ensures derived classes clean up correctly
    virtual ~GameObject() = default;

    // function meant to be overridden by dynamic objects like the player or npc
    virtual void update(float dt, Camera& camera) {}

    // rendering the object to the screen
    virtual void draw3D(Shader& shader) {
        if (model) {
            // creating the transformation matrix (position + rotation + scale)
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, position);

            // applying rotations on all axes
            m = glm::rotate(m, glm::radians(rotation.y), glm::vec3(0, 1, 0));
            m = glm::rotate(m, glm::radians(rotation.x), glm::vec3(1, 0, 0));
            m = glm::rotate(m, glm::radians(rotation.z), glm::vec3(0, 0, 1));

            m = glm::scale(m, scale);

            // sending data to the shader
            shader.setMat4("model", m);
            shader.setFloat("metallic", pbr_metallic);
            shader.setFloat("roughness", pbr_roughness);
            shader.setFloat("ao", pbr_ao);

            // finally drawing the mesh
            model->Draw(shader);
        }
    }

    // calculating the bounding box size after rotation (aabb)
    // this is needed because rotating a box changes its width/height relative to the world axes
    glm::vec3 GetRotatedSize() {
        if (physicsType != PhysicsType::BOX || glm::length(originalColliderSize) < 0.001f) {
            return glm::vec3(0.0f);
        }

        // calculating the 8 corners of the unrotated box
        glm::vec3 half = originalColliderSize * 0.5f;
        std::vector<glm::vec3> corners = {
            { -half.x, -half.y, -half.z }, {  half.x, -half.y, -half.z },
            { -half.x,  half.y, -half.z }, {  half.x,  half.y, -half.z },
            { -half.x, -half.y,  half.z }, {  half.x, -half.y,  half.z },
            { -half.x,  half.y,  half.z }, {  half.x,  half.y,  half.z }
        };

        // creating a rotation matrix
        glm::mat4 rotMat = glm::mat4(1.0f);
        rotMat = glm::rotate(rotMat, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        rotMat = glm::rotate(rotMat, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        rotMat = glm::rotate(rotMat, glm::radians(rotation.z), glm::vec3(0, 0, 1));

        // finding the new min and max points after rotation
        glm::vec3 minP(99999.0f);
        glm::vec3 maxP(-99999.0f);
        for (const auto& p : corners) {
            glm::vec3 rotatedP = glm::vec3(rotMat * glm::vec4(p, 1.0f));
            minP = glm::min(minP, rotatedP);
            maxP = glm::max(maxP, rotatedP);
        }
        // returning the full size of the new bounding box
        return maxP - minP;
    }
};
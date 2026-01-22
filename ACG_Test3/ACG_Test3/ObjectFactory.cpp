#include "ObjectFactory.hpp"
#include "Primitives.hpp"
#include <iostream>

std::unique_ptr<GameObject> ObjectFactory::CreateObject(
    glm::vec3 position,
    glm::vec3 scale,
    const std::string& modelPath,
    const std::string& iconName,
    ColliderConfig collider,
    const std::vector<std::string>& texturePaths
) {
    // 1. deciding which model to use
    std::unique_ptr<Model> model = nullptr;

    // if the path is just a keyword like "CUBE", we create a shape from code
    if (modelPath == "CUBE") model = Primitives::CreateCube();
    else if (modelPath == "SPHERE") model = Primitives::CreateSphere();
    else {
        // otherwise, we try to load a real 3d file from the disk
        model = std::make_unique<Model>(modelPath);
    }

    // safety check to make sure the model actually exists before moving on
    if (!model) return nullptr;

    // 2. manually adding textures if we provided a list of them
    if (!model->GetMeshes().empty()) {
        // these are the standard names the shader expects for different layers
        std::vector<std::string> types = {
            "texture_diffuse",
            "texture_normal",
            "texture_metallic",
            "texture_roughness",
            "texture_ao",
            "texture_height"
        };

        // looping through the provided paths and matching them to the types above
        for (size_t i = 0; i < texturePaths.size() && i < types.size(); i++) {
            // skip if the path is empty or marked as "NONE"
            if (texturePaths[i].empty() || texturePaths[i] == "NONE") continue;

            // loading the image file and getting an id back from opengl
            unsigned int id = Model::TextureFromFile(texturePaths[i].c_str(), "resources", false);
            if (id != 0) {
                Texture3D t;
                t.id = id;
                t.type = types[i];
                t.path = texturePaths[i];
                // sticking the new texture onto the first part of the model
                model->GetMeshes()[0].textures.push_back(t);
            }
        }
    }

    // 3. creating the actual game object now that the model is ready
    auto obj = std::make_unique<GameObject>(position, std::move(model), iconName);
    obj->scale = scale;
    obj->rotation = glm::vec3(0.0f); // starting with no rotation

    // 4. setting up the physics hitbox
    // we combine the object's position with the specific offset for the hitbox
    glm::vec3 finalColPos = position + collider.offset;

    // use the custom size if provided, otherwise just match the visual scale of the object
    glm::vec3 finalColSize = (glm::length(collider.customSize) > 0.001f) ? collider.customSize : scale;

    // saving the physics settings into the object for later
    obj->physicsType = collider.type;
    obj->originalColliderSize = finalColSize;
    obj->colliderOffset = collider.offset;

    // 5. registering the object with the collision manager so it can be hit
    if (collider.type == PhysicsType::BOX) {
        // adding a square hitbox
        obj->physicsID = CollisionManager::getInstance().addBox(finalColPos, finalColSize);
    }
    else if (collider.type == PhysicsType::SPHERE) {
        // adding a round hitbox (radius is half the width)
        float radius = finalColSize.x * 0.5f;
        obj->physicsID = CollisionManager::getInstance().addSphere(finalColPos, radius);
    }

    // handing the finished object back to the game
    return obj;
}
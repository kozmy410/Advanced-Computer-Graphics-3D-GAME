#pragma once
// including the base class so we can inherit from it
#include "GameObject.hpp"

// creating the npc class based on the generic gameobject
// this gives us all the standard stuff like position, rotation, and drawing automatically
class NPC : public GameObject {
public:
    // constructor: setting up the npc with a starting location, a 3d model to look like something,
    // and a 2d icon (maybe for a minimap or ui)
    NPC(glm::vec3 pos, const std::string& modelPath, const std::string& iconPath);

    // update: this function runs every single frame.
    // we override it here to add specific npc logic (like walking around or looking at the player).
    // it takes the camera so the npc knows where the player is looking.
    void update(float deltaTime, Camera& camera) override;

};
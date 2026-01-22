#include "NPC.hpp"

// constructor: this runs once when the npc is created
NPC::NPC(glm::vec3 pos, const std::string& modelPath, const std::string& iconPath)
// we pass the position, the loaded 3d model, and the icon up to the base 'GameObject'
    : GameObject(pos, std::make_unique<Model>(modelPath), iconPath)
{
    // setting a starting size. 0.05 is used because some 3d models are huge by default
    scale = glm::vec3(0.05f);
}

// update: this runs every single frame of the game
void NPC::update(float deltaTime, Camera& camera) {
    // this is where you would put ai logic, like pathfinding or looking at the player

    // example: uncomment the line below to make the npc spin slowly
    // rotation.y += 10.0f * deltaTime;
}
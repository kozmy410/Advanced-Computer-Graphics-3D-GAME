#include "Player.hpp"
#include "InputManager.hpp"
#include "CollisionManager.hpp"
#include "GameConstants.hpp"
#include "Camera.hpp"
#include <iostream>
#include <cmath>

// this helper makes sure the player turns the "short way" around a circle
// so they don't spin 350 degrees just to turn 10 degrees to the left
static float InterpolateAngle2(float current, float target, float speed, float dt) {
    float difference = target - current;
    while (difference < -180.0f) difference += 360.0f;
    while (difference > 180.0f) difference -= 360.0f;
    return current + difference * speed * dt;
}

// constructor: setting up our player, their model, and their bag (inventory)
Player::Player(glm::vec3 pos, const std::string& modelPath, const std::string& iconPath)
    : GameObject(pos, nullptr, iconPath), // initializing the base object stuff
    inventory(10)                       // giving the player 10 slots to hold things
{
    // 1. try to load the 3d model file
    try {
        model = std::make_unique<Model>(modelPath);
    }
    catch (...) {
        std::cout << "ERROR: Failed to load player model: " << modelPath << std::endl;
    }

    // 2. facing the player toward the camera by default
    rotation.y = 180.0f;

    // 3. giving the player some basic items to start with
    inventory.addItem("Small fih", 1);
    inventory.addItem("Flashlight", 1);
}

void Player::update(float deltaTime, Camera& camera) {
    auto& input = InputManager::getInstance();

    // capturing movement keys (wasd)
    glm::vec3 inputDir(0.0f);
    if (input.isKeyHeld(GLFW_KEY_W)) inputDir.z += 1.0f;
    if (input.isKeyHeld(GLFW_KEY_S)) inputDir.z -= 1.0f;
    if (input.isKeyHeld(GLFW_KEY_A)) inputDir.x += 1.0f;
    if (input.isKeyHeld(GLFW_KEY_D)) inputDir.x -= 1.0f;

    // only do move math if a key is actually being pressed
    if (glm::length(inputDir) > 0.1f) {
        inputDir = glm::normalize(inputDir); // prevents moving faster diagonally

        // calculating which way the player should move based on where the camera is looking
        float camYawRad = std::atan2(camera.Front.z, camera.Front.x);
        float theta = camYawRad;

        glm::vec3 moveDir;
        moveDir.x = inputDir.x * std::sin(theta) + inputDir.z * std::cos(theta);
        moveDir.y = 0.0f;
        moveDir.z = inputDir.x * -std::cos(theta) + inputDir.z * std::sin(theta);

        glm::vec3 velocity = moveDir * m_speed * deltaTime;

        // --- COLLISION & SLIDING LOGIC ---
        // we check X and Z separately. this allows the player to "slide" along a wall
        // instead of just stopping dead when they hit it at an angle.



        float playerRadius = 0.5f;

        // 1. check horizontal movement (x-axis)
        glm::vec3 nextPosX = position;
        nextPosX.x += velocity.x;

        // don't let the player walk off the edge of the map
        nextPosX.x = glm::clamp(nextPosX.x, MAP_BOUNDS_MIN, MAP_BOUNDS_MAX);

        // ask the physics system if this new x position is hitting a wall
        if (CollisionManager::getInstance().isMovementValid(nextPosX, playerRadius)) {
            position.x = nextPosX.x;
        }

        // 2. check vertical movement (z-axis)
        glm::vec3 nextPosZ = position;
        nextPosZ.z += velocity.z;

        nextPosZ.z = glm::clamp(nextPosZ.z, MAP_BOUNDS_MIN, MAP_BOUNDS_MAX);

        // if the path is clear, update our actual position
        if (CollisionManager::getInstance().isMovementValid(nextPosZ, playerRadius)) {
            position.z = nextPosZ.z;
        }

        // --- ROTATION ---
        // turning the player's 3d model to face the direction they are walking
        float targetAngle = glm::degrees(std::atan2(moveDir.x, moveDir.z));
        float modelOffset = 0.0f;
        rotation.y = InterpolateAngle2(rotation.y, targetAngle + modelOffset, 15.0f, deltaTime);
    }
}
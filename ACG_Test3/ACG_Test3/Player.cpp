#include "Player.hpp"
#include "InputManager.hpp"
#include "CollisionManager.hpp"
#include "GameConstants.hpp"
#include "Camera.hpp" // Required to use camera.Front/Right
#include <iostream>
#include <cmath> 

// --- HELPER: Smooth Angle Interpolation (Prevents Beyblade Spinning) ---
// Calculates the shortest path between angles (e.g., 350 to 10 goes forward 20, not back 340)
float InterpolateAngle(float current, float target, float speed, float dt) {
    float difference = target - current;
    // Wrap difference to -180 to 180 range
    while (difference < -180.0f) difference += 360.0f;
    while (difference > 180.0f) difference -= 360.0f;
    return current + difference * speed * dt;
}

Player::Player(glm::vec3 pos, const std::string& modelPath, const std::string& iconPath)
    : GameObject(pos, modelPath, iconPath), m_playerName("Whiskers"), m_score(0), m_xp(0)
{
    scale = glm::vec3(0.05f); // Adjust as needed
}

void Player::update(float deltaTime, const Camera& camera) {
    auto& input = InputManager::getInstance();
    glm::vec3 moveDir(0.0f);

    // 1. FLATTEN CAMERA VECTORS
    // We want to walk on the ground, not fly into the sky
    glm::vec3 camFront = camera.Front;
    camFront.y = 0.0f;
    camFront = glm::normalize(camFront);

    glm::vec3 camRight = camera.Right;
    camRight.y = 0.0f;
    camRight = glm::normalize(camRight);

    // 2. CAMERA-RELATIVE INPUT
    // W = Move in Camera Direction
    // D = Move in Camera Right
    if (input.isKeyHeld(GLFW_KEY_W)) moveDir += camFront;
    if (input.isKeyHeld(GLFW_KEY_S)) moveDir -= camFront;
    if (input.isKeyHeld(GLFW_KEY_D)) moveDir += camRight;
    if (input.isKeyHeld(GLFW_KEY_A)) moveDir -= camRight;

    if (glm::length(moveDir) > 0.0f) {
        moveDir = glm::normalize(moveDir);

        // --- ROTATION LOGIC ---

        // A. Calculate Target Angle from movement direction
        // atan2 returns Radians.
        float targetRad = atan2(moveDir.x, moveDir.z);

        // B. Convert to Degrees
        float targetDeg = glm::degrees(targetRad);

        // C. Apply Correction (If cat faces wrong way)
        // Try 0.0f, 90.0f, 180.0f, -90.0f if model faces sideways
        float MODEL_OFFSET = 0.0f;
        targetDeg += MODEL_OFFSET;

        // D. Smooth Rotation (No Beyblade)
        // 10.0f * deltaTime gives a nice smooth turn.
        rotation.y = InterpolateAngle(rotation.y, targetDeg, 10.0f, deltaTime);

        // --- POSITION LOGIC ---
        glm::vec3 velocity = moveDir * m_speed * deltaTime;
        glm::vec3 nextPos = position + velocity;

        // Bounds & Collision
        nextPos.x = glm::clamp(nextPos.x, MAP_BOUNDS_MIN, MAP_BOUNDS_MAX);
        nextPos.z = glm::clamp(nextPos.z, MAP_BOUNDS_MIN, MAP_BOUNDS_MAX);

        if (CollisionManager::getInstance().isMovementValid(nextPos)) {
            position = nextPos;
        }
    }
}
#include "Player.hpp"
#include "GameConstants.hpp"
#include "InputManager.hpp"
#include "CollisionManager.hpp" 

Player::Player(glm::vec3 pos, const std::string& texPath)
    : GameObject(pos, texPath), m_playerName("Whiskers")
{
    scale = glm::vec3(3.0f, 1.0f, 3.0f);
}

void Player::update(float deltaTime) {
    auto& input = InputManager::getInstance();
    glm::vec3 velocity(0.0f);

    
    if (input.isKeyHeld(GLFW_KEY_W)) { velocity.z -= 1.0f; }
    if (input.isKeyHeld(GLFW_KEY_S)) { velocity.z += 1.0f; }
    if (input.isKeyHeld(GLFW_KEY_A)) { velocity.x -= 1.0f; }
    if (input.isKeyHeld(GLFW_KEY_D)) { velocity.x += 1.0f; }

    if (glm::length(velocity) > 0.0f) {
        velocity = glm::normalize(velocity) * m_speed * deltaTime;
    }

    
    glm::vec3 targetPos = position + velocity;

    
    targetPos.x = glm::clamp(targetPos.x, MAP_BOUNDS_MIN, MAP_BOUNDS_MAX);
    targetPos.z = glm::clamp(targetPos.z, MAP_BOUNDS_MIN, MAP_BOUNDS_MAX);

    
    if (CollisionManager::getInstance().isMovementValid(targetPos)) {
        position = targetPos;
    }
}
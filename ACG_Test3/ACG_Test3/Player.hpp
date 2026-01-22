#pragma once
#include "GameObject.hpp"
#include "Inventory.hpp" // bringing in the inventory system so the player can hold items
#include "Camera.hpp"    // needed so we can pass the camera into the update function
#include <string>
#include <iostream>

class Player : public GameObject {
public:
    // the player has their own inventory instance to manage items
    Inventory inventory;

    // standard constructor to set position and visuals
    Player(glm::vec3 pos, const std::string& modelPath, const std::string& iconPath);

    // runs every frame. we override the base class because the player needs custom logic
    // (like movement input) that connects to the camera
    void update(float deltaTime, Camera& camera) override;

    // --- simple stats helpers ---

    void setName(const std::string& name) { m_playerName = name; }
    const std::string& getName() const { return m_playerName; }

    void addScore(int value) { m_score += value; }
    int getScore() const { return m_score; }

    void addXP(int value) { m_xp += value; }
    int getXP() const { return m_xp; }

    // used to check if we are standing in a special area (like a door or event)
    bool isInTriggerZone() const { return m_isInTriggerZone; }
    void setInTriggerZone(bool value) { m_isInTriggerZone = value; }

private:
    float m_speed = 20.0f;
    std::string m_playerName = "Player";
    int m_score = 0;
    int m_xp = 0;
    bool m_isInTriggerZone = false;
};
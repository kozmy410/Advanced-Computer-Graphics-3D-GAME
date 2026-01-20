#pragma once

#include "GameObject.hpp"
#include <map>
#include <string> // Required for std::string

class Player : public GameObject {
public:
    Player(glm::vec3 pos, const std::string& texPath);

    // --- NEW: Function to get the player name on startup ---
    void initializePlayerName();

    void update(float deltaTime) override;

    bool isInTriggerZone() const { return m_isInTriggerZone; }
    int getScore() const { return m_score; }
    int getXP() const { return m_xp; }
    // --- NEW: Getter for the Player's Name ---
    const std::string& getName() const { return m_playerName; }

private:
    float m_speed = 10.0f;
    bool m_isInTriggerZone = false;

    // --- NEW: Player's Name ---
    std::string m_playerName;

    int m_score = 0;
    int m_xp = 0;

    std::map<int, bool> m_zonesEntered;
};
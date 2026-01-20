#pragma once
#include "GameObject.hpp"
#include <string>

class Player : public GameObject {
public:
    Player(glm::vec3 pos, const std::string& texPath);

    void update(float deltaTime) override;

    
    void setName(const std::string& name) { m_playerName = name; }
    const std::string& getName() const { return m_playerName; }

    void addScore(int value) { m_score += value; }
    void addXP(int value) { m_xp += value; }

    int getScore() const { return m_score; }
    int getXP() const { return m_xp; }

    
    bool isInTriggerZone() const { return m_isInTriggerZone; }
    void setInTriggerZone(bool value) { m_isInTriggerZone = value; }

private:
    float m_speed = 10.0f;
    std::string m_playerName;
    int m_score = 0;
    int m_xp = 0;
    bool m_isInTriggerZone = false;
};
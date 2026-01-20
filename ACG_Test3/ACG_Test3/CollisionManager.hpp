#pragma once

#include "CollisionZone.hpp" 

class CollisionManager {
public:
    static CollisionManager& getInstance();

    
    void addZone(const CollisionZone& zone);

    
    bool isMovementValid(glm::vec3 worldPosition);

    
    
    int checkTriggers(glm::vec3 worldPosition);

private:
    CollisionManager() = default;
    ~CollisionManager() = default;
    CollisionManager(const CollisionManager&) = delete;
    void operator=(const CollisionManager&) = delete;

    
    std::vector<CollisionZone> m_zones;

    glm::vec2 worldToPixel(glm::vec3 worldPos);
    bool isPointInPolygon(glm::vec2 point, const std::vector<glm::vec2>& polygon);
};
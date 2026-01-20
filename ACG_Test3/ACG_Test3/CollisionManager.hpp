#pragma once

#include "CollisionZone.hpp" // <-- Include our new header

class CollisionManager {
public:
    static CollisionManager& getInstance();

    // Replaces addCollider. Now takes a complete zone struct.
    void addZone(const CollisionZone& zone);

    // Checks if a position is inside a SOLID collider.
    bool isMovementValid(glm::vec3 worldPosition);

    // Checks if a position is inside a TRIGGER and returns its ID.
    // Returns -1 if not in any trigger.
    int checkTriggers(glm::vec3 worldPosition);

private:
    CollisionManager() = default;
    ~CollisionManager() = default;
    CollisionManager(const CollisionManager&) = delete;
    void operator=(const CollisionManager&) = delete;

    // Now stores a list of our new structs
    std::vector<CollisionZone> m_zones;

    glm::vec2 worldToPixel(glm::vec3 worldPos);
    bool isPointInPolygon(glm::vec2 point, const std::vector<glm::vec2>& polygon);
};
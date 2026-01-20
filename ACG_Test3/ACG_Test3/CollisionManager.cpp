#include "CollisionManager.hpp"
#include <iostream>


const float MAP_WORLD_SIZE = 100.0f;
const float MAP_PIXEL_SIZE = 1024.0f;

CollisionManager& CollisionManager::getInstance() {
    static CollisionManager instance;
    return instance;
}

void CollisionManager::addZone(const CollisionZone& zone) {
    m_zones.push_back(zone);
}

glm::vec2 CollisionManager::worldToPixel(glm::vec3 worldPos) {
    
    float normX = (worldPos.x + (MAP_WORLD_SIZE / 2.0f)) / MAP_WORLD_SIZE;
    float normZ = (worldPos.z + (MAP_WORLD_SIZE / 2.0f)) / MAP_WORLD_SIZE;

    
    
    
    return glm::vec2(normX * MAP_PIXEL_SIZE, (1.0f - normZ) * MAP_PIXEL_SIZE);
}


bool CollisionManager::isPointInPolygon(glm::vec2 point, const std::vector<glm::vec2>& polygon) {
    bool isInside = false;
    int n = polygon.size();
    if (n < 3) {
        return false; 
    }

    
    for (int i = 0, j = n - 1; i < n; j = i++) {
        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x)) {
            isInside = !isInside;
        }
    }
    return isInside;
}

bool CollisionManager::isMovementValid(glm::vec3 worldPosition) {
    glm::vec2 pixelPos = worldToPixel(worldPosition);
    for (const auto& zone : m_zones) {
        if (zone.type == ZoneType::COLLIDER) {
            if (isPointInPolygon(pixelPos, zone.pixelPolygon)) {
                return false;
            }
        }
    }
    return true;
}

int CollisionManager::checkTriggers(glm::vec3 worldPosition) {
    glm::vec2 pixelPos = worldToPixel(worldPosition);
    for (const auto& zone : m_zones) {
        if (zone.type == ZoneType::TRIGGER) {
            if (isPointInPolygon(pixelPos, zone.pixelPolygon)) {
                return zone.id;
            }
        }
    }
    return -1;
}
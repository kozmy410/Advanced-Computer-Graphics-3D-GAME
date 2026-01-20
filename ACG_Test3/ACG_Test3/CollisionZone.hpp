#pragma once

#include <vector>
#include "dependencies/glm-1.0.2/glm/glm.hpp"


enum class ZoneType {
    COLLIDER, 
    TRIGGER   
};


struct CollisionZone {
    int id = -1;                      
    ZoneType type = ZoneType::TRIGGER;
    std::vector<glm::vec2> pixelPolygon;
};
#pragma once

#include <vector>
#include "dependencies/glm-1.0.2/glm/glm.hpp"

// defining the different types of areas we can have
enum class ZoneType {
    COLLIDER, // a solid wall or object you bang into
    TRIGGER   // an invisible area that starts an event when you walk in
};

// the actual container for the zone data
struct CollisionZone {
    // the unique number for this zone, -1 means it's invalid or empty
    int id = -1;

    // what kind of zone is this? defaulting to trigger
    ZoneType type = ZoneType::TRIGGER;

    // a list of 2d points that outline the shape of the zone
    std::vector<glm::vec2> pixelPolygon;
};
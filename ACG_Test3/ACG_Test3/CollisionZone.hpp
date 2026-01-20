#pragma once

#include <vector>
#include <string>
#include "dependencies/glm-1.0.2/glm/glm.hpp"

// An enum to clearly define the types of zones we can have
enum class ZoneType {
    COLLIDER, // A solid wall
    TRIGGER   // An area that activates an event
};

// A struct to hold all the data for a single zone
struct CollisionZone {
    int id;                      // A unique ID for this zone (e.g., 1 for "level_1_trigger")
    ZoneType type;               // What kind of zone is this?
    std::vector<glm::vec2> pixelPolygon; // The shape in pixel coordinates
};
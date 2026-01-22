#include "CollisionManager.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>


// registers a new box collider (like a wall or crate) into our physics system
int CollisionManager::addBox(glm::vec3 position, glm::vec3 size) {
    Box b;
    glm::vec3 halfSize = size * 0.5f;

    // calculating the bottom-left-back and top-right-front corners
    // this makes the math much faster later on
    b.min = position - halfSize;
    b.max = position + halfSize;
    b.id = (int)m_boxes.size(); // using the index as a unique id
    b.isTrigger = false;        // default is solid (cannot walk through)

    m_boxes.push_back(b);
    return b.id; // returning the id so the game object knows which box belongs to it
}

// registers a new sphere collider (like a ball or just a simple round object)
int CollisionManager::addSphere(glm::vec3 position, float radius) {
    Sphere s;
    s.center = position;
    s.radius = radius;
    s.id = (int)m_spheres.size();

    m_spheres.push_back(s);
    return s.id;
}

// registers an invisible zone that triggers events (like "player entered boss room")
void CollisionManager::addZone(glm::vec3 position, glm::vec3 size, int id) {
    CollisionZone z;
    z.position = position;
    z.size = size;
    z.id = id;

    m_zones.push_back(z);
}

// objects move every frame, so we need to update their invisible hitboxes too.
// otherwise, the graphics would move but the collision would stay behind.
void CollisionManager::updateBox(int id, glm::vec3 position, glm::vec3 size) {
    // safety check to make sure we don't crash if the id is wrong
    if (id >= 0 && id < m_boxes.size()) {
        glm::vec3 halfSize = size * 0.5f;
        m_boxes[id].min = position - halfSize;
        m_boxes[id].max = position + halfSize;
    }
}

void CollisionManager::updateSphere(int id, glm::vec3 position, float radius) {
    if (id >= 0 && id < m_spheres.size()) {
        m_spheres[id].center = position;
        m_spheres[id].radius = radius;
    }
}


// wipes everything clean, usually when loading a new level
void CollisionManager::clearPrimitives() {
    m_boxes.clear();
    m_spheres.clear();
    m_zones.clear();
}


// the main function called by the player to ask: "am i hitting anything?"
bool CollisionManager::checkCollisions(glm::vec3 position, float radius) {
    // creating a temporary sphere representing the player
    Sphere playerSphere;
    playerSphere.center = position;
    playerSphere.radius = radius;

    // 1. check against all solid boxes
    for (const auto& box : m_boxes) {
        if (!box.isTrigger) { // only checking solid objects (triggers are ghosts)
            if (checkSphereBox(playerSphere, box)) {
                return true; // we hit something! stop here.
            }
        }
    }

    // 2. check against all other spheres
    for (const auto& sphere : m_spheres) {
        // simple distance check: if distance between centers < sum of radii, they touch
        float dist = glm::distance(playerSphere.center, sphere.center);
        if (dist < (playerSphere.radius + sphere.radius)) {
            return true;
        }
    }

    return false; // safe, nothing hit
}

// checks if the player is standing inside a special event zone
int CollisionManager::checkTriggers(glm::vec3 position) {
    // 1. check story zones (like level transitions)
    for (const auto& zone : m_zones) {
        if (checkPointBox(position, zone.position, zone.size)) {
            return zone.id; // return the id so the game logic knows which event to run
        }
    }

    // 2. check standard boxes that were marked as triggers
    for (const auto& box : m_boxes) {
        if (box.isTrigger) {
            // converting min/max back to center/size for the point check
            glm::vec3 size = box.max - box.min;
            glm::vec3 center = box.min + (size * 0.5f);

            if (checkPointBox(position, center, size)) {
                return box.id;
            }
        }
    }

    return -1; // -1 means no trigger was hit
}

// simple collision check between a sphere and a box
// this is the math to see if a round ball is touching a square box.
bool CollisionManager::checkSphereBox(const Sphere& s, const Box& b) {
    // clever math trick: we find the point on the box that is closest to the sphere's center.
    // we do this by "clamping" the sphere's position to the box's edges.
    float x = std::max(b.min.x, std::min(s.center.x, b.max.x));
    float y = std::max(b.min.y, std::min(s.center.y, b.max.y));
    float z = std::max(b.min.z, std::min(s.center.z, b.max.z));

    // calculating the distance between that closest point and the sphere center
    float distance = std::sqrt(
        (x - s.center.x) * (x - s.center.x) +
        (y - s.center.y) * (y - s.center.y) +
        (z - s.center.z) * (z - s.center.z)
    );

    // if that distance is less than the radius, we are touching!
    return distance < s.radius;
}

// simple check to see if a single point (xyz) is inside a box
bool CollisionManager::checkPointBox(glm::vec3 point, glm::vec3 boxPos, glm::vec3 boxSize) {
    glm::vec3 half = boxSize * 0.5f;
    glm::vec3 min = boxPos - half;
    glm::vec3 max = boxPos + half;

    // is the point inside the x, y, AND z bounds?
    return (point.x >= min.x && point.x <= max.x &&
        point.y >= min.y && point.y <= max.y &&
        point.z >= min.z && point.z <= max.z);
}
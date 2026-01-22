#pragma once
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include <vector>
#include <algorithm>

// defining a zone that triggers events when entered
struct CollisionZone {
    glm::vec3 position;
    glm::vec3 size;
    int id;
};

// simple box shape for collision
struct Box {
    glm::vec3 min;
    glm::vec3 max;
    bool isTrigger = false;
    int id = -1;
};

// simple sphere shape for collision
struct Sphere {
    glm::vec3 center;
    float radius;
    int id = -1;
};

class CollisionManager {
private:
    // keeping track of all the shapes in the world
    std::vector<Box> m_boxes;
    std::vector<Sphere> m_spheres;
    std::vector<CollisionZone> m_zones;
public:
    // getting the main instance of the collision manager
    static CollisionManager& getInstance() {
        static CollisionManager instance;
        return instance;
    }

    // adding a new box to the world
    int addBox(glm::vec3 position, glm::vec3 size);
    // adding a new sphere to the world
    int addSphere(glm::vec3 position, float radius);

    // creating a zone that triggers specific events (like cutscenes)
    void addZone(glm::vec3 position, glm::vec3 size, int id);

    // wiping all collision objects (used when reloading levels)
    void clearPrimitives();

    // moving a box to a new location
    void updateBox(int id, glm::vec3 position, glm::vec3 size);
    // moving a sphere to a new location
    void updateSphere(int id, glm::vec3 position, float radius);

    // checking if a position hits any physical object
    bool checkCollisions(glm::vec3 position, float radius);
    // checking if the player is standing inside an event zone
    int checkTriggers(glm::vec3 position);

    // simple helper to see if we can walk there or if a wall is in the way
    bool isMovementValid(glm::vec3 position, float radius) {
        return !checkCollisions(position, radius);
    }

private:
    // math to see if a sphere hits a box
    bool checkSphereBox(const Sphere& s, const Box& b);
    // math to see if a point is inside a box
    bool checkPointBox(glm::vec3 point, glm::vec3 boxPos, glm::vec3 boxSize);
};
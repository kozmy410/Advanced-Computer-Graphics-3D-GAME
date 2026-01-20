#pragma once
#include "GameObject.hpp"

class NPC : public GameObject {
public:
    NPC(glm::vec3 pos, const std::string& texPath);
};
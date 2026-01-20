#include "NPC.hpp"


#include "dependencies/glm-1.0.2/glm/glm.hpp"

/**
 * @brief The constructor for the NPC class.
 * @param pos The starting 3D position of the NPC in the world.
 * @param texPath The file path to the texture this NPC will use.
 */
NPC::NPC(glm::vec3 pos, const std::string& texPath)


    : GameObject(pos, texPath)
{
    
    
    
    scale = glm::vec3(4.0f, 1.0f, 4.0f);
}
#include "NPC.hpp"

// We need to include the GLM header to use its data types like glm::vec3.
#include "dependencies/glm-1.0.2/glm/glm.hpp"

/**
 * @brief The constructor for the NPC class.
 * @param pos The starting 3D position of the NPC in the world.
 * @param texPath The file path to the texture this NPC will use.
 */
NPC::NPC(glm::vec3 pos, const std::string& texPath)
// This line calls the constructor of the base class (GameObject),
// passing the position and texture path arguments up to it.
    : GameObject(pos, texPath)
{
    // --- SET THE NPC'S DEFAULT SIZE HERE ---
    // This makes all NPC icons yxy units in the game world.
    // We set the X and Z scale. The Y scale doesn't matter for our top-down minimap but is good to set anyway.
    scale = glm::vec3(4.0f, 1.0f, 4.0f);
}
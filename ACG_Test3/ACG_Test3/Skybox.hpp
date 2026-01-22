#pragma once
#include <vector>
#include <string>
#include <iostream>
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtc/matrix_transform.hpp"
#include "Shader.hpp"

// this class renders a large cube around the player to simulate the sky and horizon
class Skybox {
public:
    // constructor: expects a list of 6 image paths (right, left, top, bottom, front, back)
    Skybox(const std::vector<std::string>& faces);
    ~Skybox(); // clean up memory

    // renders the skybox. needs the camera's view and projection matrices to align with the world.
    void draw(const glm::mat4& view, const glm::mat4& projection);

    // helper to get the texture id. this is crucial for the pbr system so objects can reflect the sky.
    unsigned int getTextureID() const { return m_textureID; }

private:
    unsigned int m_textureID; // the id of the 'cubemap' texture (the 6 combined images)
    unsigned int m_VAO, m_VBO; // opengl buffers for the cube geometry
    Shader* m_shader; // a special shader just for the skybox

    // internal function to read the 6 images and combine them into one opengl texture
    void loadCubemap(const std::vector<std::string>& faces);

    // sets up the vertices for the cube box itself
    void setupMesh();
};
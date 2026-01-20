#pragma once

#include <string>
#include <map>

// We need GLEW for all the OpenGL function declarations.
#include "dependencies/glew-2.2.0/include/GL/glew.h"
// We need GLM for sending matrix and vector data to the shader.
#include "dependencies/glm-1.0.2/glm/glm.hpp"

class Shader {
public:
    // The public ID for the linked shader program.
    unsigned int ID;

    // Constructor: Takes paths to the vertex and fragment shader files.
    Shader(const char* vertexPath, const char* fragmentPath);

    // Activates the shader program for use in rendering.
    void use();

    // Utility functions to send data (uniforms) to the shader.
    void setVec3(const std::string& name, const glm::vec3& value);
    void setMat4(const std::string& name, const glm::mat4& mat);

private:
    // A map to store the locations of uniforms we've already found, for efficiency.
    std::map<std::string, int> m_uniformLocationCache;

    // A helper function to check for compilation or linking errors.
    void checkCompileErrors(unsigned int shader, std::string type);

    // A helper function to safely find uniform locations (and use the cache).
    int getUniformLocation(const std::string& name);
};
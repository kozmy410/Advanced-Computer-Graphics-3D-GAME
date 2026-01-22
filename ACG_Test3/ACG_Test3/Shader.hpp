#pragma once
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>

// this class handles loading, compiling, and managing the code that runs on the graphics card (gpu)
class Shader {
public:
    unsigned int ID; // the unique reference number opengl gives to this specific shader program

    // constructor: takes the file paths for the vertex and fragment code, reads them, and compiles them
    Shader(const char* vertexPath, const char* fragmentPath);

    // tells opengl to start using this shader program for any drawing we do next
    void use();

    // --- utility functions to send data to the gpu ---
    // these allow us to update variables inside the shader code (like colors, brightness, or matrices)
    // from our c++ application

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec3(const std::string& name, const glm::vec3& value);
    void setMat4(const std::string& name, const glm::mat4& mat);

private:
    // internal helper to check if the shader code failed to compile (e.g., syntax errors) and print why
    void checkCompileErrors(unsigned int shader, std::string type);

    // --- optimization system ---
    // asking the gpu where a variable lives is slow.
    // this helper function looks up the location once and saves it in a map (cache).
    // next time we need it, we grab it from the map instantly instead of asking the gpu again.
    int getUniformLocation(const std::string& name);
    std::map<std::string, int> m_uniformLocationCache;
};
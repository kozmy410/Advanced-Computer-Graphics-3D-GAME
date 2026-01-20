#include <fstream>
#include <sstream>
#include <iostream>

#include "Shader.hpp"

// Constructor implementation
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // 1. Retrieve the shader source code from the file paths.
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // Ensure ifstream objects can throw exceptions on errors.
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        ID = 0; // Set ID to 0 to indicate failure
        return;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. Compile the shaders.
    unsigned int vertex, fragment;

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // 3. Link the shaders into a Shader Program.
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // The individual shaders are now linked into the program and can be deleted.
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() {
    glUseProgram(ID);
}

// Uniform setter implementations
void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    int location = getUniformLocation(name);
    if (location != -1) { // Only set if the uniform was found
        glUniform3fv(location, 1, &value[0]);
    }
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) {
    int location = getUniformLocation(name);
    if (location != -1) { // Only set if the uniform was found
        glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
    }
}

// Helper function implementations
int Shader::getUniformLocation(const std::string& name) {
    // Check if we already have this uniform location in our cache.
    if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end()) {
        return m_uniformLocationCache[name];
    }

    // If not, get the location from OpenGL.
    int location = glGetUniformLocation(ID, name.c_str());

    // If the location is -1, the uniform was not found. Print a warning.
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader program " << ID << "!" << std::endl;
    }

    // Store the location in our cache for future use.
    m_uniformLocationCache[name] = location;
    return location;
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION of type: " << type << "\n" << infoLog << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING of type: " << type << "\n" << infoLog << std::endl;
        }
    }
}
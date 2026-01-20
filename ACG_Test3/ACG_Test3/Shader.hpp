#pragma once

#include <string>
#include <map>


#include "dependencies/glew-2.2.0/include/GL/glew.h"

#include "dependencies/glm-1.0.2/glm/glm.hpp"

class Shader {
public:
    
    unsigned int ID;

    
    Shader(const char* vertexPath, const char* fragmentPath);

    
    void use();

    
    void setVec3(const std::string& name, const glm::vec3& value);
    void setMat4(const std::string& name, const glm::mat4& mat);

    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);

private:
    
    std::map<std::string, int> m_uniformLocationCache;

    
    void checkCompileErrors(unsigned int shader, std::string type);

    
    int getUniformLocation(const std::string& name);
};
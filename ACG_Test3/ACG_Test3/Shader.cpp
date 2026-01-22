#include "Shader.hpp"

// --- CONSTRUCTOR ---
// this is the big function that reads your shader files and turns them into a gpu program
Shader::Shader(const char* vertexPath, const char* fragmentPath) {

    // 1. getting the shader text out of the files
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // telling the file readers to alert us if something goes wrong (like a missing file)
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // opening the files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        // dumping the file contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        // we are done with the files now, so we close them
        vShaderFile.close();
        fShaderFile.close();

        // storing the code as strings so opengl can read it
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        // if the file path was wrong or the file is locked, we'll see this error
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        ID = 0;
        return;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. compiling the shaders
    unsigned int vertex, fragment;

    // creating the vertex shader (handles point positions)
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX"); // making sure there are no typos in the shader code

    // creating the fragment shader (handles pixel colors)
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // 3. linking everything into a single "shader program"
    // this is like an .exe that runs on your graphics card
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // the code is now loaded onto the gpu, so we can delete the temporary "folders" we used to compile it
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

// simple command to tell opengl "use this specific shader for everything i draw next"
void Shader::use() {
    glUseProgram(ID);
}

// --- UNIFORM FUNCTIONS ---
// these send data from your c++ code to the variables inside the shader files

void Shader::setBool(const std::string& name, bool value) {
    int location = getUniformLocation(name);
    if (location != -1) glUniform1i(location, (int)value);
}

void Shader::setInt(const std::string& name, int value) {
    int location = getUniformLocation(name);
    if (location != -1) glUniform1i(location, value);
}

// using 1f because we are sending exactly one float value
void Shader::setFloat(const std::string& name, float value) {
    int location = getUniformLocation(name);
    if (location != -1) glUniform1f(location, value);
}

// sending a vec3 (usually for colors or positions)
void Shader::setVec3(const std::string& name, const glm::vec3& value) {
    int location = getUniformLocation(name);
    if (location != -1) glUniform3fv(location, 1, &value[0]);
}

// sending a 4x4 matrix (usually for camera movement or object positioning)
void Shader::setMat4(const std::string& name, const glm::mat4& mat) {
    int location = getUniformLocation(name);
    if (location != -1) glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
}

// --- PRIVATE HELPERS ---

// searching for a variable name in the shader.
// we use a cache (map) so we don't have to keep asking the gpu over and over, which is slow.
int Shader::getUniformLocation(const std::string& name) {
    if (m_uniformLocationCache.find(name) != m_uniformLocationCache.end())
        return m_uniformLocationCache[name];

    int location = glGetUniformLocation(ID, name.c_str());
    m_uniformLocationCache[name] = location;
    return location;
}

// this prints out the exact line and reason if your shader has a bug
void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
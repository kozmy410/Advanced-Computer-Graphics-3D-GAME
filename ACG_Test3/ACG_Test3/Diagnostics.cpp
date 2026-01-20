#include "Diagnostics.hpp"

// The :: operator is used to define a function that belongs to a class.
void Diagnostics::printOpenGLInfo() {
    std::cout << "----------------------------------------------------------------\n";
    std::cout << "               OpenGL Renderer Information\n";
    std::cout << "----------------------------------------------------------------\n";

    // --- Basic Information (glGetString) ---
    // The reinterpret_cast is used to convert the GLubyte* from glGetString
    // to a char* that std::cout can print as a string.
    std::cout << "Vendor:   " << reinterpret_cast<const char*>(glGetString(GL_VENDOR)) << std::endl;
    std::cout << "Renderer: " << reinterpret_cast<const char*>(glGetString(GL_RENDERER)) << std::endl;
    std::cout << "Version:  " << reinterpret_cast<const char*>(glGetString(GL_VERSION)) << std::endl;
    std::cout << "GLSL:     " << reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION)) << std::endl;

    std::cout << "----------------------------------------------------------------\n";
    std::cout << "                 OpenGL Implementation Limits\n";
    std::cout << "----------------------------------------------------------------\n";

    // --- Querying Integer Limits (glGetIntegerv) ---
    GLint queryResult;

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &queryResult);
    std::cout << "Max Texture Size: " << queryResult << "x" << queryResult << std::endl;

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &queryResult);
    std::cout << "Max Texture Image Units (Fragment Shader): " << queryResult << std::endl;

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &queryResult);
    std::cout << "Max Combined Texture Image Units (All Stages): " << queryResult << std::endl;

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &queryResult);
    std::cout << "Max Vertex Attributes: " << queryResult << std::endl;

    GLint viewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, viewportDims);
    std::cout << "Max Viewport Dimensions: " << viewportDims[0] << "x" << viewportDims[1] << std::endl;

    glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &queryResult);
    std::cout << "Max Renderbuffer Size: " << queryResult << "x" << queryResult << std::endl;

    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);
    std::cout << "Number of Supported Extensions: " << numExtensions << std::endl;
    std::cout << "----------------------------------------------------------------\n";
}
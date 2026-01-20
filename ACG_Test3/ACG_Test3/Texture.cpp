#include "Texture.hpp"
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include <iostream>

// Tell the compiler to implement the stb_image functions here in this one file.
#define STB_IMAGE_IMPLEMENTATION
#include "dependencies/stb/stb_image.h"


Texture::Texture(const std::string& path)
    : m_rendererID(0), m_filePath(path), m_width(0), m_height(0), m_bpp(0) {

    // --- THE FIX IS HERE ---
    // This single line tells stb_image to flip the image vertically when it loads it.
    // This corrects the coordinate system mismatch between image files and OpenGL.
    stbi_set_flip_vertically_on_load(0);

    // Load the image data from the file into CPU memory.
    // The '4' at the end forces stb_image to load it as RGBA (4 channels).
    unsigned char* localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 4);

    if (localBuffer) {
        // Generate and bind an OpenGL texture object.
        glGenTextures(1, &m_rendererID);
        glBindTexture(GL_TEXTURE_2D, m_rendererID);

        // Set the texture filtering and wrapping parameters.
        // GL_LINEAR provides smooth, bilinear filtering.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // GL_CLAMP_TO_EDGE prevents artifacts at the texture's borders.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Upload the texture data from the CPU (localBuffer) to the GPU.
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);

        // Unbind the texture now that we're done configuring it.
        glBindTexture(GL_TEXTURE_2D, 0);

        // The image data is now on the GPU, so we can free the CPU memory.
        stbi_image_free(localBuffer);
    }
    else {
        std::cerr << "Error: Failed to load texture: " << path << std::endl;
        std::cerr << "stb_image failure reason: " << stbi_failure_reason() << std::endl;
    }
}

Texture::~Texture() {
    glDeleteTextures(1, &m_rendererID);
}

void Texture::bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_rendererID);
}

void Texture::unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
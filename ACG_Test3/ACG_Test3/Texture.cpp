#include "Texture.hpp"
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include <iostream>

// this defines the actual code for the stb image library in this file
#define STB_IMAGE_IMPLEMENTATION
#include "dependencies/stb/stb_image.h"

Texture::Texture(const std::string& path)
    : m_rendererID(0), m_filePath(path), m_width(0), m_height(0), m_bpp(0) {

    // we keep this at 0 (off) because our engine handles uv coordinates right-side up
    stbi_set_flip_vertically_on_load(0);

    // loading the image data into a local buffer. we force it to 4 channels (rgba) 
    // so it always has transparency support even if the source is a simple jpg.
    unsigned char* localBuffer = stbi_load(path.c_str(), &m_width, &m_height, &m_bpp, 4);

    if (localBuffer) {
        // asking opengl for a unique id for our new texture
        glGenTextures(1, &m_rendererID);
        glBindTexture(GL_TEXTURE_2D, m_rendererID);

        // setting the look of the texture. linear makes it look smooth when stretched.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // clamp to edge prevents weird tiny lines at the edges of textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // sending the actual pixels from the cpu memory to the gpu memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);

        // unbinding so we don't accidentally change this texture later
        glBindTexture(GL_TEXTURE_2D, 0);

        // now that it's on the gpu, we can clear the temporary cpu data
        stbi_image_free(localBuffer);
    }
    else {
        // letting us know if the file path was wrong or the image format is broken
        std::cerr << "Error: Failed to load texture: " << path << std::endl;
        std::cerr << "stb_image failure reason: " << stbi_failure_reason() << std::endl;
    }
}

Texture::~Texture() {
    // cleaning up the texture on the graphics card when the object is destroyed
    glDeleteTextures(1, &m_rendererID);
}

void Texture::bind(unsigned int slot) const {
    // telling opengl which 'slot' to plug this texture into (0, 1, 2, etc.)
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_rendererID);
}

void Texture::unbind() const {
    // simply turning off the current texture binding
    glBindTexture(GL_TEXTURE_2D, 0);
}
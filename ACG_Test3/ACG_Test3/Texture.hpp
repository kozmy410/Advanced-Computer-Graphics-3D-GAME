#pragma once
#include <string>

class Texture {
public:
    Texture(const std::string& path);
    ~Texture();

    void bind(unsigned int slot = 0) const;
    void unbind() const;

private:
    unsigned int m_rendererID;
    std::string m_filePath;
    int m_width, m_height, m_bpp; // Bits per pixel
};
#pragma once
#include <string>

// a simple wrapper class to handle loading and using images (textures)
class Texture {
public:
    // constructor: reads the image file from the disk and sends the data to the graphics card
    Texture(const std::string& path);

    // destructor: tells the graphics card to delete the texture memory when we are done with it
    ~Texture();

    // activates this texture so opengl uses it for the next object we draw.
    // 'slot' is used because we can have multiple textures at once (slot 0 for color, slot 1 for bump maps, etc.)
    void bind(unsigned int slot = 0) const;

    // turns off the texture so it doesn't accidentally get applied to other objects
    void unbind() const;

private:
    unsigned int m_rendererID; // the unique id number opengl gives to this specific texture
    std::string m_filePath;    // keeping the file path handy just in case we need to debug
    int m_width, m_height, m_bpp; // storing the size of the image and its color depth (bits per pixel)
};
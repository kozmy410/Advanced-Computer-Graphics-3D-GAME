#pragma once

#include <vector>
#include <string>
#include <map>
#include "dependencies/glm-1.0.2/glm/glm.hpp"

// telling the compiler these classes exist so we don't have to include the heavy headers here
class Shader;
class GameObject;
class Player;
class Texture;

class Minimap {
public:
    // constructor: needs screen size to know where to draw
    Minimap(int screenWidth, int screenHeight);
    // destructor: cleaning up memory
    ~Minimap();

    // the main function that renders the map and icons every frame
    void draw(const Player* player, const std::vector<GameObject*>& gameObjects, float deltaTime);

    // switching between the small corner map and the big full-screen map
    void toggleMaximized();

    // telling the camera how close it should zoom in (smoothly)
    void setTargetZoom(float target);

    // called when the player resizes the game window so the map stays proportional
    void onWindowResize(int newWidth, int newHeight);

    // checking if we are currently looking at the big map
    bool isMaximized() const { return m_isMaximized; }

private:
    // the shader program used to draw 2d elements
    Shader* m_shader;
    Texture* m_mapTexture;

    // opengl buffers for the icons (points/quads)
    unsigned int m_vao, m_vbo, m_ibo, m_indexCount;
    // opengl buffers for the map background itself
    unsigned int m_mapVao, m_mapVbo, m_mapIbo;

    // remembering screen dimensions
    int m_screenWidth, m_screenHeight;
    // state flag for the big map mode
    bool m_isMaximized;

    // variables to handle the smooth zooming effect
    float m_currentZoom;
    float m_targetZoom;
    float m_zoomSpeed;

    // keeping loaded textures here so we don't load them from disk 60 times a second
    std::map<std::string, Texture*> m_textureCache;

    // helper to load an icon or get it from the cache if it's already there
    Texture* getTexture(const std::string& path);

    // preparing the shape (quad) for icons
    void setupIconGeometry();
    // preparing the shape for the map border/background
    void setupMapGeometry();

};
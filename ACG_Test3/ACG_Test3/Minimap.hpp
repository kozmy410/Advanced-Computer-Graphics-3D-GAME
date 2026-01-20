#pragma once

#include <vector>
#include <string>
#include <map>
#include "dependencies/glm-1.0.2/glm/glm.hpp"

// Forward declarations to reduce compile time dependencies
class Shader;
class GameObject;
class Player;
class Texture;

class Minimap {
public:
    Minimap(int screenWidth, int screenHeight);
    ~Minimap();

    // The main draw function
    void draw(const Player* player, const std::vector<GameObject*>& gameObjects, float deltaTime);

    // Interaction methods
    void toggleMaximized();
    void setTargetZoom(float target);
    void onWindowResize(int newWidth, int newHeight);

    bool isMaximized() const { return m_isMaximized; }

private:
    // Rendering Resources
    Shader* m_shader;
    Texture* m_mapTexture;

    // Geometry Data
    unsigned int m_vao, m_vbo, m_ibo, m_indexCount; // For Icons
    unsigned int m_mapVao, m_mapVbo, m_mapIbo;      // For the Map Background

    // State
    int m_screenWidth, m_screenHeight;
    bool m_isMaximized;

    // Zoom Logic
    float m_currentZoom;
    float m_targetZoom;
    float m_zoomSpeed;

    // Texture Cache (avoids reloading "cat.png" 100 times)
    std::map<std::string, Texture*> m_textureCache;

    // Helpers
    Texture* getTexture(const std::string& path);
    void setupIconGeometry();
    void setupMapGeometry();
};
#pragma once

#include <vector>
#include <string>
#include <map>

class Shader;
class GameObject;
class Player;
class Texture;

class Minimap {
public:
    Minimap(int screenWidth, int screenHeight);
    ~Minimap();
    void draw(const Player* player, const std::vector<GameObject*>& gameObjects, float deltaTime);
    void toggleMaximized();
    void onWindowResize(int newWidth, int newHeight);
    void setTargetZoom(float target);

    bool isMaximized() const { return m_isMaximized; }

private:
    Shader* m_shader;
    Texture* m_mapTexture;
    unsigned int m_vao, m_vbo, m_ibo, m_indexCount;
    unsigned int m_mapVao, m_mapVbo, m_mapIbo;
    int m_screenWidth, m_screenHeight;
    bool m_isMaximized;
    float m_currentZoom;
    float m_targetZoom;
    float m_zoomSpeed;
    std::map<std::string, Texture*> m_textureCache;
    Texture* getTexture(const std::string& path);
    void setupIconGeometry();
    void setupMapGeometry();
};
#pragma once
#include <vector>
#include <map> 
#include <memory>
#include <string>
#include "Camera.hpp"
#include "Shader.hpp"
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"

class Player;
class GameObject;
class Minimap;

class Game {
public:
    Game(const char* title, int width, int height);
    ~Game();

    void run();
    void onResize(int width, int height);

private:
    void initGLFW();
    void initOpenGL();
    void initGameObjects();

    // --- Camera Orbit Variables ---
    float m_cameraYaw = 0.0f;   // Horizontal rotation (around Y axis)
    float m_cameraPitch = 20.0f; // Vertical rotation (up/down)
    float m_cameraDistance = 14.0f; // Distance from player

    Camera camera;
    std::unique_ptr<Shader> shader3D; // The shader for 3D models
    bool isMouseCaptured = false;     // To toggle mouse cursor

    void handleInput();
    void update(float deltaTime);
    void render(float deltaTime);
    void updateWindowTitle(float deltaTime);

    void handleStoryEvent(int id);

    GLFWwindow* window;
    int windowWidth, windowHeight, windowPosX, windowPosY;
    bool isFullscreen;

    
    bool m_storyInitialized;
    std::map<int, bool> m_processedEvents; 

    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::unique_ptr<Minimap> minimap;
};
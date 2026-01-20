#pragma once

#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"

#include <array>


constexpr int MAX_KEYS = 1024;
constexpr int MAX_BUTTONS = 32;

class InputManager {
public:
    static InputManager& getInstance();

    
    bool isKeyPressed(int key);  
    bool isKeyHeld(int key);     
    bool isKeyReleased(int key); 

    
    bool isMouseButtonPressed(int button);
    bool isMouseButtonHeld(int button);
    bool isMouseButtonReleased(int button);

    glm::vec2 getMousePosition() const;
    glm::vec2 getMouseDelta() const; 
    float getScrollY() const;        

    
    void update(); 

    
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    InputManager();
    ~InputManager();
    InputManager(const InputManager&) = delete;
    void operator=(const InputManager&) = delete;

    
    std::array<bool, MAX_KEYS> m_keys;
    std::array<bool, MAX_KEYS> m_prevKeys;

    
    std::array<bool, MAX_BUTTONS> m_mouseButtons;
    std::array<bool, MAX_BUTTONS> m_prevMouseButtons;

    
    glm::vec2 m_mousePosition;
    glm::vec2 m_prevMousePosition; 

    float m_scrollX;
    float m_scrollY;
};
#pragma once

#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"

#include <array>

// defining the maximum number of keys and mouse buttons to track
constexpr int MAX_KEYS = 1024;
constexpr int MAX_BUTTONS = 32;

class InputManager {
public:
    // getting the single global instance of the input manager
    static InputManager& getInstance();


    // checking if a key was pressed *just now* (this frame)
    bool isKeyPressed(int key);
    // checking if a key is currently being held down
    bool isKeyHeld(int key);
    // checking if a key was just let go
    bool isKeyReleased(int key);


    // checking if a mouse button was clicked *just now*
    bool isMouseButtonPressed(int button);
    // checking if a mouse button is being held down
    bool isMouseButtonHeld(int button);
    // checking if a mouse button was just released
    bool isMouseButtonReleased(int button);

    // getting the cursor coordinates on the screen
    glm::vec2 getMousePosition() const;
    // getting how much the mouse moved since the last frame
    glm::vec2 getMouseDelta() const;
    // getting the scroll wheel value
    float getScrollY() const;


    // processing the input states at the end of a frame (preparing for the next one)
    void update();


    // functions that glfw calls automatically when something happens
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    // standard singleton stuff to prevent creating copies
    InputManager();
    ~InputManager();
    InputManager(const InputManager&) = delete;
    void operator=(const InputManager&) = delete;


    // storing the current state of all keys
    std::array<bool, MAX_KEYS> m_keys;
    // storing the state of keys from the previous frame (to detect changes)
    std::array<bool, MAX_KEYS> m_prevKeys;



    // storing current mouse button states
    std::array<bool, MAX_BUTTONS> m_mouseButtons;
    // storing previous mouse button states
    std::array<bool, MAX_BUTTONS> m_prevMouseButtons;


    // tracking where the mouse is and where it was
    glm::vec2 m_mousePosition;
    glm::vec2 m_prevMousePosition;

    // tracking scroll wheel movement
    float m_scrollX;
    float m_scrollY;
};
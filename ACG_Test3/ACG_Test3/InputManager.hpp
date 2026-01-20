#pragma once

#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"
#include <map>

class InputManager {
public:
    static InputManager& getInstance();

    bool isKeyPressed(int key);
    bool isKeyReleased(int key);
    bool isKeyHeld(int key);
    void update();

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    InputManager();
    ~InputManager();
    InputManager(const InputManager&) = delete;
    void operator=(const InputManager&) = delete;

    std::map<int, bool> m_keyState;
    std::map<int, bool> m_lastKeyState;

    bool wasKeyHeld(int key);
};
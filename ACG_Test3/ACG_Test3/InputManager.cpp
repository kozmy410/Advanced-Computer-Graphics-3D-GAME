#include "InputManager.hpp"

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

InputManager::InputManager() {}
InputManager::~InputManager() {}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputManager& self = getInstance();
    if (action == GLFW_PRESS) {
        self.m_keyState[key] = true;
    }
    else if (action == GLFW_RELEASE) {
        self.m_keyState[key] = false;
    }
}

void InputManager::update() {
    m_lastKeyState = m_keyState;
}

bool InputManager::isKeyHeld(int key) {
    if (m_keyState.find(key) != m_keyState.end()) {
        return m_keyState[key];
    }
    return false;
}

bool InputManager::isKeyPressed(int key) {
    return isKeyHeld(key) && !wasKeyHeld(key);
}

bool InputManager::isKeyReleased(int key) {
    return !isKeyHeld(key) && wasKeyHeld(key);
}

bool InputManager::wasKeyHeld(int key) {
    if (m_lastKeyState.find(key) != m_lastKeyState.end()) {
        return m_lastKeyState[key];
    }
    return false;
}
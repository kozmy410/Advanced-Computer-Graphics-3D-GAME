#include "InputManager.hpp"
#include <cstring> 

InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

InputManager::InputManager()
    : m_mousePosition(0.0f), m_prevMousePosition(0.0f), m_scrollX(0.0f), m_scrollY(0.0f)
{
    
    m_keys.fill(false);
    m_prevKeys.fill(false);
    m_mouseButtons.fill(false);
    m_prevMouseButtons.fill(false);
}

InputManager::~InputManager() {}


void InputManager::update() {
    
    
    m_prevKeys = m_keys;
    m_prevMouseButtons = m_mouseButtons;

    
    m_prevMousePosition = m_mousePosition;

    
    m_scrollX = 0.0f;
    m_scrollY = 0.0f;
}


bool InputManager::isKeyHeld(int key) {
    if (key >= 0 && key < MAX_KEYS) {
        return m_keys[key];
    }
    return false;
}

bool InputManager::isKeyPressed(int key) {
    
    return isKeyHeld(key) && !m_prevKeys[key];
}

bool InputManager::isKeyReleased(int key) {
    
    return !isKeyHeld(key) && m_prevKeys[key];
}


bool InputManager::isMouseButtonHeld(int button) {
    if (button >= 0 && button < MAX_BUTTONS) {
        return m_mouseButtons[button];
    }
    return false;
}

bool InputManager::isMouseButtonPressed(int button) {
    return isMouseButtonHeld(button) && !m_prevMouseButtons[button];
}

bool InputManager::isMouseButtonReleased(int button) {
    return !isMouseButtonHeld(button) && m_prevMouseButtons[button];
}


glm::vec2 InputManager::getMousePosition() const {
    return m_mousePosition;
}

glm::vec2 InputManager::getMouseDelta() const {
    return m_mousePosition - m_prevMousePosition;
}

float InputManager::getScrollY() const {
    return m_scrollY;
}



void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    
    if (key < 0 || key >= MAX_KEYS) return;

    InputManager& self = getInstance();
    if (action == GLFW_PRESS) {
        self.m_keys[key] = true;
    }
    else if (action == GLFW_RELEASE) {
        self.m_keys[key] = false;
    }
    
}

void InputManager::mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button < 0 || button >= MAX_BUTTONS) return;

    InputManager& self = getInstance();
    if (action == GLFW_PRESS) {
        self.m_mouseButtons[button] = true;
    }
    else if (action == GLFW_RELEASE) {
        self.m_mouseButtons[button] = false;
    }
}

void InputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    InputManager& self = getInstance();
    self.m_mousePosition.x = static_cast<float>(xpos);
    self.m_mousePosition.y = static_cast<float>(ypos);
}

void InputManager::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    InputManager& self = getInstance();
    self.m_scrollX = static_cast<float>(xoffset);
    self.m_scrollY = static_cast<float>(yoffset);
}
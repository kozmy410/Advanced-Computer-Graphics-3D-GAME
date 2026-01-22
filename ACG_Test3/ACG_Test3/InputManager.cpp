#include "InputManager.hpp"
#include <cstring> 

// accessing the single global instance of this class (singleton pattern)
// this ensures the whole game talks to the same input manager.
InputManager& InputManager::getInstance() {
    static InputManager instance;
    return instance;
}

// constructor: setting everything to a clean "zero" state so inputs don't glitch at startup
InputManager::InputManager()
    : m_mousePosition(0.0f), m_prevMousePosition(0.0f), m_scrollX(0.0f), m_scrollY(0.0f)
{
    // using .fill to set all keys and buttons to false (not pressed)
    m_keys.fill(false);
    m_prevKeys.fill(false);
    m_mouseButtons.fill(false);
    m_prevMouseButtons.fill(false);
}

InputManager::~InputManager() {}

// this function must be called exactly once per frame (usually at the start or end of the game loop).
// it prepares the data for the next frame's calculations.
void InputManager::update() {

    // snapshotting the current state into the "previous" state variables.
    // this allows us to compare "now" vs "last frame" to detect clicks vs holds.
    m_prevKeys = m_keys;
    m_prevMouseButtons = m_mouseButtons;

    // saving the old mouse position so we can calculate movement (delta)
    m_prevMousePosition = m_mousePosition;

    // resetting scroll because scroll wheels don't "hold" a position, they just fire once
    m_scrollX = 0.0f;
    m_scrollY = 0.0f;
}

// checks if a key is currently down (good for movement like holding 'W')
bool InputManager::isKeyHeld(int key) {
    if (key >= 0 && key < MAX_KEYS) {
        return m_keys[key];
    }
    return false;
}

// checks if a key was *just* pushed down this exact frame (good for jumping or shooting)
bool InputManager::isKeyPressed(int key) {
    // logic: it is down now, but it wasn't down last frame
    return isKeyHeld(key) && !m_prevKeys[key];
}

// checks if a key was *just* let go this frame
bool InputManager::isKeyReleased(int key) {
    // logic: it is not down now, but it was down last frame
    return !isKeyHeld(key) && m_prevKeys[key];
}

// same logic as keys, but for mouse clicks (left, right, middle)
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

// returns exactly where the mouse cursor is on the window
glm::vec2 InputManager::getMousePosition() const {
    return m_mousePosition;
}

// returns how much the mouse moved since the last frame.
// very useful for camera controls (looking around).
glm::vec2 InputManager::getMouseDelta() const {
    return m_mousePosition - m_prevMousePosition;
}

float InputManager::getScrollY() const {
    return m_scrollY;
}

// --- CALLBACKS ---
// these functions are called automatically by glfw whenever hardware events happen.
// we just use them to update our internal boolean arrays.

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key < 0 || key >= MAX_KEYS) return; // safety check

    InputManager& self = getInstance();
    if (action == GLFW_PRESS) {
        self.m_keys[key] = true;
    }
    else if (action == GLFW_RELEASE) {
        self.m_keys[key] = false;
    }
    // we ignore GLFW_REPEAT because we handle repeats internally with our update() logic
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
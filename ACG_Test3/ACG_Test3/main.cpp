#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
#include <iomanip>
#include <random>

// Custom classes
#include "GameConstants.hpp" 
#include "InputManager.hpp"
#include "Diagnostics.hpp"
#include "Minimap.hpp"
#include "Player.hpp"
#include "NPC.hpp"

// Library includes
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtc/matrix_transform.hpp"

// --- No cleanup in the code, had no time :/ ---

// --- Global pointer and updated callback ---
Minimap* minimap = nullptr;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    if (minimap) {
        minimap->onWindowResize(width, height);
    }
}

// --- Global Variables for window state ---
int lastWindowWidth = 800;
int lastWindowHeight = 600;
int lastWindowPosX = 100;
int lastWindowPosY = 100;
bool isFullscreen = false;



int main() {
    // 1. Initialize libraries and create window
    if (!glfwInit()) { std::cerr << "Failed to init GLFW\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(lastWindowWidth, lastWindowHeight, "Minimap Project", NULL, NULL);
    if (!window) { glfwTerminate(); std::cerr << "Failed to create window\n"; return -1; }
    glfwSetWindowPos(window, lastWindowPosX, lastWindowPosY);
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, InputManager::keyCallback);
    if (glewInit() != GLEW_OK) { std::cerr << "Failed to init GLEW\n"; return -1; }

    // Enable OpenGL features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Diagnostics::printOpenGLInfo();

    // 2. Create GameObjects
    Player* player = new Player(glm::vec3(7.0f, 0.0f, 35.0f), "resources/BrownHead.png");
    assert(player != nullptr);
    std::vector<GameObject*> gameObjects;
    gameObjects.push_back(player);

    std::vector<std::string> npcTexturePaths = { "resources/TuxedoCat.png", "resources/GreyCat.png" };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, npcTexturePaths.size() - 1);
    gameObjects.push_back(new NPC(glm::vec3(10.0f, 0.0f, 40.0f), npcTexturePaths[distrib(gen)]));
    gameObjects.push_back(new NPC(glm::vec3(-3.0f, 0.0f, 18.0f), npcTexturePaths[distrib(gen)]));
    gameObjects.push_back(new NPC(glm::vec3(-1.0f, 0.0f, -26.0f), npcTexturePaths[distrib(gen)]));

    // 3. Create the minimap instance
    minimap = new Minimap(lastWindowWidth, lastWindowHeight);

    // 4. Set up timing variables
    double lastTime = glfwGetTime();
    float deltaTime = 0.0f;

    // --- The Game Loop ---
    while (!glfwWindowShouldClose(window)) {
        // --- Calculate Delta Time ---
        double currentTime = glfwGetTime();
        deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // --- FPS and Player Position Title Update ---
        static double titleUpdateTime = 0.0;
        titleUpdateTime += deltaTime;
        if (titleUpdateTime >= 1.0) {
            std::stringstream ss;
            ss << " Project WHISKERS: THE CAT HOOD aka. THE HOOD CAT | FPS: " << static_cast<int>(1.0 / deltaTime)
                << " | Player Pos: ("
                << std::fixed << std::setprecision(1)
                << player->position.x << ", " << player->position.y << ", " << player->position.z << ")";
            glfwSetWindowTitle(window, ss.str().c_str());
            titleUpdateTime = 0.0;
        }

        // --- Process Input ---
        if (InputManager::getInstance().isKeyPressed(GLFW_KEY_F11)) {
            isFullscreen = !isFullscreen;
            if (isFullscreen) {
                glfwGetWindowPos(window, &lastWindowPosX, &lastWindowPosY);
                glfwGetWindowSize(window, &lastWindowWidth, &lastWindowHeight);
                GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
                const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
                glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            }
            else {
                glfwSetWindowMonitor(window, NULL, lastWindowPosX, lastWindowPosY, lastWindowWidth, lastWindowHeight, 0);
            }
        }
        if (InputManager::getInstance().isKeyPressed(GLFW_KEY_M)) {
            minimap->toggleMaximized();
        }
       
        // --- Update Game Logic ---
        // This updates the player's position and their m_isInTriggerZone state.
        for (auto& obj : gameObjects) {
            obj->update(deltaTime);
        }

        // --- NEW: Game State Orchestration ---
        // The main loop acts as the "brain", telling the minimap how to behave
        // based on the player's state.
        if (player->isInTriggerZone()) {
            // If the player is in a zone, set the target zoom for a 2x zoom-in.
            // (10.0f is a 2x zoom on the default 20.0f view).
            minimap->setTargetZoom(10.0f);
        }
        else {
            // If the player is not in a zone, set the target zoom back to the default.
            minimap->setTargetZoom(20.0f);
        }

        // --- Rendering ---
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // The draw call now requires deltaTime for the smooth zoom animation.
        minimap->draw(player, gameObjects, deltaTime);

        // --- Finalize Frame ---
        glfwSwapBuffers(window);
        InputManager::getInstance().update();
        glfwPollEvents();
        if (InputManager::getInstance().isKeyPressed(GLFW_KEY_ESCAPE)) {
            glfwDestroyWindow(window);
        }
    }

    // --- Cleanup ---
    for (auto& obj : gameObjects) {
        delete obj;
    }
    delete minimap;

    glfwTerminate();
    return 0;
}
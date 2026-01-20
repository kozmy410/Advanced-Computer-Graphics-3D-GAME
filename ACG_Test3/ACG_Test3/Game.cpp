#include "Game.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <vector>

#include "GameConstants.hpp"
#include "InputManager.hpp"
#include "AudioManager.hpp"
#include "CollisionManager.hpp"
#include "Diagnostics.hpp"
#include "Minimap.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "Primitives.hpp"

// Callback wrapper
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) game->onResize(width, height);
}

Game::Game(const char* title, int width, int height)
    : windowWidth(width), windowHeight(height),
    windowPosX(100), windowPosY(100),
    isFullscreen(false), m_storyInitialized(false)
{
    initGLFW();
    window = glfwCreateWindow(windowWidth, windowHeight, title, NULL, NULL);
    if (!window) { glfwTerminate(); throw std::runtime_error("Failed to create window"); }

    glfwSetWindowUserPointer(window, this);
    glfwSetWindowPos(window, windowPosX, windowPosY);
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, InputManager::keyCallback);
    glfwSetMouseButtonCallback(window, InputManager::mouseCallback);
    glfwSetCursorPosCallback(window, InputManager::cursorPosCallback);
    glfwSetScrollCallback(window, InputManager::scrollCallback);

    initOpenGL();
    AudioManager::getInstance().init();
    initGameObjects();

    std::cout << "Game Initialized." << std::endl;
}

Game::~Game() {
    AudioManager::getInstance().cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Game::initGLFW() {
    if (!glfwInit()) throw std::runtime_error("Failed to init GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

void Game::initOpenGL() {
    if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to init GLEW");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Game::initGameObjects() {
    shader3D = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
    camera = Camera(glm::vec3(7.0f, 5.0f, 45.0f));

    // Primitives
    auto floorModel = Primitives::CreatePlane("resources/map.bmp", 1.0f);
    auto floor = std::make_unique<GameObject>(glm::vec3(0, -0.05f, 0), std::move(floorModel), "");
    floor->scale = glm::vec3(100.0f, 1.0f, 100.0f);
    gameObjects.push_back(std::move(floor));

    auto wallModel = Primitives::CreateCube("resources/wall.jpg");
    auto wall = std::make_unique<GameObject>(glm::vec3(5.0f, 2.0f, 15.0f), std::move(wallModel), "resources/icon_wall.png");
    wall->scale = glm::vec3(10.0f, 4.0f, 1.0f);
    gameObjects.push_back(std::move(wall));

    // Player & NPCs
    player = std::make_unique<Player>(glm::vec3(7.0f, 0.0f, 35.0f), "resources/models/cat.obj", "resources/BrownHead.png");

    std::vector<std::string> npcIcons = { "resources/TuxedoCat.png", "resources/GreyCat.png" };
    std::string npcModel = "resources/models/cat.obj";
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, static_cast<int>(npcIcons.size()) - 1);

    gameObjects.push_back(std::make_unique<NPC>(glm::vec3(10.0f, 0.0f, 40.0f), npcModel, npcIcons[distrib(gen)]));
    gameObjects.push_back(std::make_unique<NPC>(glm::vec3(-3.0f, 0.0f, 18.0f), npcModel, npcIcons[distrib(gen)]));
    gameObjects.push_back(std::make_unique<NPC>(glm::vec3(-1.0f, 0.0f, -26.0f), npcModel, npcIcons[distrib(gen)]));

    minimap = std::make_unique<Minimap>(windowWidth, windowHeight);

    // Collisions
    CollisionManager& colMgr = CollisionManager::getInstance();
    auto createZone = [](int id, float minX, float maxX, float minZ, float maxZ) {
        CollisionZone zone; zone.id = id; zone.type = ZoneType::TRIGGER;
        const float MAP_WORLD_SIZE = 100.0f; const float MAP_PIXEL_SIZE = 1024.0f;
        auto toPix = [&](float x, float z) {
            float normX = (x + (MAP_WORLD_SIZE / 2.0f)) / MAP_WORLD_SIZE;
            float normZ = (z + (MAP_WORLD_SIZE / 2.0f)) / MAP_WORLD_SIZE;
            return glm::vec2(normX * MAP_PIXEL_SIZE, (1.0f - normZ) * MAP_PIXEL_SIZE);
            };
        zone.pixelPolygon.push_back(toPix(minX, minZ)); zone.pixelPolygon.push_back(toPix(maxX, minZ));
        zone.pixelPolygon.push_back(toPix(maxX, maxZ)); zone.pixelPolygon.push_back(toPix(minX, maxZ));
        return zone;
        };
    colMgr.addZone(createZone(0, -8.0f, -5.0f, 14.0f, 23.0f));
    colMgr.addZone(createZone(1, 7.0f, 12.0f, 38.0f, 49.0f));
    colMgr.addZone(createZone(2, 42.0f, 47.0f, 42.0f, 47.0f));
}

void Game::run() {
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        handleInput();
        update(deltaTime);
        render(deltaTime);
        updateWindowTitle(deltaTime);

        glfwSwapBuffers(window);
        InputManager::getInstance().update();
        glfwPollEvents();
    }
}

void Game::handleInput() {
    InputManager& input = InputManager::getInstance();

    if (input.isKeyPressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, true);
    }

    // 1. Mouse Capture Toggle (Press C)
    if (input.isKeyPressed(GLFW_KEY_C)) {
        isMouseCaptured = !isMouseCaptured;
        if (isMouseCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Hide cursor
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);   // Show cursor
        }
    }

    // 2. ORBIT CAMERA LOGIC
    // Only move camera if mouse is captured
    if (isMouseCaptured) {
        glm::vec2 delta = input.getMouseDelta();
        float sensitivity = 0.1f;

        m_cameraYaw += delta.x * sensitivity;
        m_cameraPitch -= delta.y * sensitivity; // Subtract to invert Y (Standard FPS feel)

        // Clamp Pitch (Prevent camera flipping upside down or going under ground)
        if (m_cameraPitch > 89.0f) m_cameraPitch = 89.0f;
        if (m_cameraPitch < -10.0f) m_cameraPitch = -10.0f; // Don't go too low
    }

    // 3. Zoom (Scroll Wheel)
    float scroll = input.getScrollY();
    if (scroll != 0.0f) {
        m_cameraDistance -= scroll;
        if (m_cameraDistance < 2.0f) m_cameraDistance = 2.0f;
        if (m_cameraDistance > 30.0f) m_cameraDistance = 30.0f;
    }

    if (input.isKeyPressed(GLFW_KEY_M)) {
        minimap->toggleMaximized();
    }
}

void Game::update(float deltaTime) {
    // 1. UPDATE PLAYER
    player->update(deltaTime, camera);
    for (auto& obj : gameObjects) obj->update(deltaTime, camera);

    // 2. ORBIT CAMERA MATH

    // Center point: The Player's Head
    glm::vec3 targetCenter = player->position + glm::vec3(0.0f, 2.0f, 0.0f);

    float yawRad = glm::radians(m_cameraYaw);
    float pitchRad = glm::radians(m_cameraPitch);

    float hDist = m_cameraDistance * cos(pitchRad);
    float vDist = m_cameraDistance * sin(pitchRad);

    float offsetX = hDist * sin(yawRad);
    float offsetZ = hDist * cos(yawRad);

    glm::vec3 cameraPos;
    cameraPos.x = targetCenter.x - offsetX;
    cameraPos.y = targetCenter.y + vDist;
    cameraPos.z = targetCenter.z - offsetZ;

    // --- FIX: FLOOE CLAMP ---
    // If the calculated Y is lower than 0.2f, force it to 0.2f.
    // This makes the camera "slide" along the floor instead of going under it.
    if (cameraPos.y < 0.2f) {
        cameraPos.y = 0.2f;
    }

    // Apply smooth damping
    float smoothSpeed = 15.0f;
    camera.Position = glm::mix(camera.Position, cameraPos, smoothSpeed * deltaTime);

    // Always look at the player's head
    camera.Front = glm::normalize(targetCenter - camera.Position);
    camera.Right = glm::normalize(glm::cross(camera.Front, glm::vec3(0.0f, 1.0f, 0.0f)));
    camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));

    // 3. TRIGGERS
    int triggerID = CollisionManager::getInstance().checkTriggers(player->position);
    if (triggerID != -1) {
        player->setInTriggerZone(true);
        minimap->setTargetZoom(10.0f);
        handleStoryEvent(triggerID);
    }
    else {
        player->setInTriggerZone(false);
        minimap->setTargetZoom(20.0f);
    }
}

void Game::render(float deltaTime) {
    if (windowWidth == 0 || windowHeight == 0) return;

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- PASS 1: 3D WORLD ---
    shader3D->use();

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();

    shader3D->setMat4("projection", projection);
    shader3D->setMat4("view", view);

    // Global Illumination
    shader3D->setVec3("viewPos", camera.Position);
    shader3D->setVec3("lightDir", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader3D->setVec3("lightColor", glm::vec3(2.0f, 2.0f, 2.0f));

    player->draw3D(*shader3D);
    for (auto& obj : gameObjects) {
        obj->draw3D(*shader3D);
    }

    // --- PASS 2: MINIMAP ---
    std::vector<GameObject*> renderList;
    for (auto& obj : gameObjects) renderList.push_back(obj.get());
    minimap->draw(player.get(), renderList, deltaTime);
}

void Game::handleStoryEvent(int id) {
    if (m_processedEvents[id]) return;

    if (id == 1) { // Intro
        AudioManager::getInstance().playExclusive("resources/intro.wav");
        if (!m_storyInitialized) {
            std::cout << "You see a pair of skinny kittens..." << std::endl;
            std::cout << "Enter Name: ";
            std::string name;
            std::getline(std::cin, name);
            if (name.empty()) name = "Whiskers";
            player->setName(name);
            m_storyInitialized = true;
        }
    }
    else if (id == 0) { // Mission 1
        AudioManager::getInstance().playExclusive("resources/lvl1.mp3");
        player->addScore(1000);
        std::cout << "[STORY] Mission Complete!" << std::endl;
    }
    else if (id == 2) { // Easter Egg
        AudioManager::getInstance().playExclusive("resources/easteregg.mp3");
        std::cout << "[STORY] You found the secret!" << std::endl;
    }

    m_processedEvents[id] = true;
}

void Game::onResize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    if (minimap) minimap->onWindowResize(width, height);
}

void Game::updateWindowTitle(float deltaTime) {
    static double titleUpdateTime = 0.0;
    titleUpdateTime += deltaTime;
    if (titleUpdateTime >= 1.0) {
        std::stringstream ss;
        ss << " Project WHISKERS | FPS: " << static_cast<int>(1.0 / deltaTime)
            << " | Pos: (" << std::fixed << std::setprecision(1)
            << player->position.x << ", " << player->position.z << ")"
            << " | Score: " << player->getScore();
        glfwSetWindowTitle(window, ss.str().c_str());
        titleUpdateTime = 0.0;
    }
}
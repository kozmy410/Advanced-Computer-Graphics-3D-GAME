#include "Game.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>

#include "GameConstants.hpp"
#include "InputManager.hpp"
#include "AudioManager.hpp"
#include "CollisionManager.hpp"
#include "Diagnostics.hpp"
#include "Minimap.hpp"
#include "Player.hpp"
#include "NPC.hpp"


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

    
    std::cout << "Game Initialized. Player ready at (7, 0, 35)." << std::endl;
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
    
    player = std::make_unique<Player>(glm::vec3(7.0f, 0.0f, 35.0f), "resources/BrownHead.png");

    
    std::vector<std::string> npcTextures = { "resources/TuxedoCat.png", "resources/GreyCat.png" };
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, static_cast<int>(npcTextures.size()) - 1);

    gameObjects.push_back(std::make_unique<NPC>(glm::vec3(10.0f, 0.0f, 40.0f), npcTextures[distrib(gen)]));
    gameObjects.push_back(std::make_unique<NPC>(glm::vec3(-3.0f, 0.0f, 18.0f), npcTextures[distrib(gen)]));
    gameObjects.push_back(std::make_unique<NPC>(glm::vec3(-1.0f, 0.0f, -26.0f), npcTextures[distrib(gen)]));

    
    minimap = std::make_unique<Minimap>(windowWidth, windowHeight);

    
    CollisionManager& colMgr = CollisionManager::getInstance();

    auto createZone = [](int id, float minX, float maxX, float minZ, float maxZ) {
        CollisionZone zone;
        zone.id = id;
        zone.type = ZoneType::TRIGGER;
        const float MAP_WORLD_SIZE = 100.0f;
        const float MAP_PIXEL_SIZE = 1024.0f;
        auto toPix = [&](float x, float z) {
            float normX = (x + (MAP_WORLD_SIZE / 2.0f)) / MAP_WORLD_SIZE;
            float normZ = (z + (MAP_WORLD_SIZE / 2.0f)) / MAP_WORLD_SIZE;
            return glm::vec2(normX * MAP_PIXEL_SIZE, (1.0f - normZ) * MAP_PIXEL_SIZE);
            };
        zone.pixelPolygon.push_back(toPix(minX, minZ));
        zone.pixelPolygon.push_back(toPix(maxX, minZ));
        zone.pixelPolygon.push_back(toPix(maxX, maxZ));
        zone.pixelPolygon.push_back(toPix(minX, maxZ));
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
    if (InputManager::getInstance().isKeyPressed(GLFW_KEY_ESCAPE))
        glfwSetWindowShouldClose(window, true);
    if (InputManager::getInstance().isKeyPressed(GLFW_KEY_M))
        minimap->toggleMaximized();
}

void Game::update(float deltaTime) {
    player->update(deltaTime);
    for (auto& obj : gameObjects) obj->update(deltaTime);

    
    int triggerID = CollisionManager::getInstance().checkTriggers(player->position);

    
    
    if (triggerID != -1) {
        
        if (!m_processedEvents[triggerID]) {
            std::cout << "[DEBUG] Entered Trigger Zone ID: " << triggerID << std::endl;
        }

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
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::vector<GameObject*> renderList;
    for (auto& obj : gameObjects) renderList.push_back(obj.get());
    minimap->draw(player.get(), renderList, deltaTime);
}

void Game::handleStoryEvent(int id) {
    
    if (m_processedEvents[id]) return;

    if (id == 1) { 
        std::cout << "[AUDIO] Starting Story Intro..." << std::endl;

        
        AudioManager::getInstance().playExclusive("resources/intro.wav");

        std::cout << "\n------------------------------------------------------\n";
        std::cout << "** The Turning Point **\n";
        std::cout << "You see a pair of skinny kittens..." << std::endl;

        if (!m_storyInitialized) {
            std::cout << "What is your name, hero? ";
            std::string name;
            std::getline(std::cin, name);
            if (name.empty()) name = "Whiskers";
            player->setName(name);
            std::cout << "Welcome, " << player->getName() << "!" << std::endl;
            m_storyInitialized = true;
        }
    }
    else if (id == 0) { 
        std::cout << "[AUDIO] Mission Complete - Playing Meow/Theme..." << std::endl;

        
        AudioManager::getInstance().playExclusive("resources/lvl1.mp3");
        
        

        std::cout << "\n------------------------------------------------------\n";
        std::cout << "** Mission Complete! **\n";
        player->addScore(1000);
    }
    else if (id == 2) { 
        std::cout << "[AUDIO] Found Easter Egg by kozmy410!" << std::endl;

        
        AudioManager::getInstance().playExclusive("resources/easteregg.mp3");
    }

    
    m_processedEvents[id] = true;
}

void Game::onResize(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
    if (minimap) minimap->onWindowResize(width, height);
}

void Game::updateWindowTitle(float deltaTime) {
    static double titleUpdateTime = 0.0;
    titleUpdateTime += deltaTime;
    if (titleUpdateTime >= 1.0) {
        std::stringstream ss;
        ss << " Project WHISKERS | FPS: " << static_cast<int>(1.0 / deltaTime)
            << " | Pos: (" << std::fixed << std::setprecision(1)
            << player->position.x << ", " << player->position.z << ")";
        glfwSetWindowTitle(window, ss.str().c_str());
        titleUpdateTime = 0.0;
    }
}
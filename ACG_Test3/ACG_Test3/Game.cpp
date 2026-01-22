#include "Game.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <random>
#include <vector>
#include <fstream> 

// including all the necessary headers for the engine parts
#include "GameConstants.hpp"
#include "InputManager.hpp"
#include "AudioManager.hpp"
#include "CollisionManager.hpp"
#include "Diagnostics.hpp"
#include "Minimap.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "Primitives.hpp"
#include "Skybox.hpp"
#include "ObjectFactory.hpp"

// simple structure to keep track of quest progress
struct GameTask {
    std::string description;
    bool isCompleted = false;
};

// list of current quests
std::vector<GameTask> m_tasks;
bool m_tasksInitialized = false;

// this function gets called whenever the user resizes the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // get our game instance and tell it to adjust the screen size
    Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));
    if (game) game->onResize(width, height);
}

// helper function to slap a texture onto an object
void AddTextureToObject(GameObject* obj, const std::string& filename, const std::string& type) {
    // checking if the object actually exists and has a mesh to paint on
    if (!obj || !obj->model || obj->model->GetMeshes().empty()) {
        std::cout << "Error: Object has no mesh to apply texture to!" << std::endl;
        return;
    }

    // try to load the image file from the disk
    unsigned int id = Model::TextureFromFile(filename.c_str(), "resources", false);

    if (id != 0) {
        Texture3D t;
        t.id = id;
        t.type = type;
        t.path = filename;

        // add the texture to the object's first mesh
        obj->model->GetMeshes()[0].textures.push_back(t);

        std::cout << "SUCCESS: Loaded " << type << " (" << filename << ") to object." << std::endl;
    }
    else {
        // oops, couldn't find the picture
        std::cout << "FAILED: Could not find texture file: resources/" << filename << std::endl;
    }
}

// setting up the initial to-do list for the player
void InitTasks() {
    if (m_tasksInitialized) return;

    m_tasks.clear();
    // adding the tutorial missions
    m_tasks.push_back({ "Move around using W, A, S, D" });
    m_tasks.push_back({ "Turn on Flashlight (Press F)" });
    m_tasks.push_back({ "Find and pickup an Apple" });
    m_tasks.push_back({ "Enter the House" });

    m_tasksInitialized = true;
}


// constructor: this is where the game starts up
Game::Game(const char* title, int width, int height)
    : windowWidth(width), windowHeight(height),
    windowPosX(100), windowPosY(100),
    isFullscreen(false), m_storyInitialized(false)
{
    // firing up glfw libraries
    initGLFW();

    // create the actual window on the screen
    window = glfwCreateWindow(windowWidth, windowHeight, title, NULL, NULL);
    if (!window) { glfwTerminate(); throw std::runtime_error("Failed to create window"); }

    // linking the window to this game class
    glfwSetWindowUserPointer(window, this);
    glfwSetWindowPos(window, windowPosX, windowPosY);
    glfwMakeContextCurrent(window);

    // hooking up all the inputs (keyboard, mouse, scrolling)
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, InputManager::keyCallback);
    glfwSetMouseButtonCallback(window, InputManager::mouseCallback);
    glfwSetCursorPosCallback(window, InputManager::cursorPosCallback);
    glfwSetScrollCallback(window, InputManager::scrollCallback);

    // starting up opengl
    initOpenGL();

    // starting up the imgui interface
    initImGui();

    // getting the sound system ready
    AudioManager::getInstance().init();

    // creating the world and objects
    initGameObjects();

    std::cout << "Game Initialized." << std::endl;

    // setting time speed and starting at noon
    m_timeSpeed = 0.067f;
    m_dayTime = 12.0f;
    InitTasks();
}

// destructor: cleaning up mess when game closes
Game::~Game() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    AudioManager::getInstance().cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}

// basic glfw setup settings
void Game::initGLFW() {
    if (!glfwInit()) throw std::runtime_error("Failed to init GLFW");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
}

// enabling depth testing so objects don't draw on top of each other weirdly
void Game::initOpenGL() {
    if (glewInit() != GLEW_OK) throw std::runtime_error("Failed to init GLEW");
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

// setting up the ui library
void Game::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

// drawing the quest log window
void DrawTaskUI() {
    // setting flags to make the window transparent and non-interactive
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    // making the background dark but see-through
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.4f));
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::Begin("QuestLog", nullptr, flags);

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f), "CURRENT OBJECTIVES:");
    ImGui::Separator();

    bool hasFoundActiveTask = false;

    // looping through tasks to see what's done and what's next
    for (const auto& task : m_tasks) {
        if (task.isCompleted) {
            // green text for done stuff
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 0.5f), "[X] %s", task.description.c_str());
        }
        else {
            if (!hasFoundActiveTask) {
                // white text for the current active task
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "[ ] %s", task.description.c_str());
                hasFoundActiveTask = true;
            }
            else {
                // grey out future tasks so players don't get spoiled
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 0.5f), "[ ] ???");
            }
        }
    }

    ImGui::End();
    ImGui::PopStyleColor();
}

// preparing the shadow mapping system
void Game::initShadowMap() {
    depthShader = std::make_unique<Shader>("shaders/depth.vert", "shaders/depth.frag");

    // loading the skybox images (cubemap)
    std::vector<std::string> faces = {
        "resources/skybox/right.png", "resources/skybox/left.png",
        "resources/skybox/top.png", "resources/skybox/bottom.png",
        "resources/skybox/front.png", "resources/skybox/back.png"
    };
    m_skybox = std::make_unique<Skybox>(faces);

    // creating the framebuffer that holds shadow data
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    // making sure borders of shadow map are white so things outside don't shadow
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// defining invisible boxes that trigger story events
void Game::initTriggers() {
    CollisionManager& colMgr = CollisionManager::getInstance();

    // zone 0: near the starting npc
    colMgr.addZone(glm::vec3(-6.5f, 2.0f, 18.5f), glm::vec3(4.0f, 4.0f, 10.0f), 0);

    // zone 1: near the second npc
    colMgr.addZone(glm::vec3(9.5f, 2.0f, 43.5f), glm::vec3(5.0f, 4.0f, 12.0f), 1);

    // zone 99: right at the door to enter the house
    colMgr.addZone(glm::vec3(10.0f, 2.0f, 10.0f), glm::vec3(4.0f, 4.0f, 4.0f), 99);
}


// wiping physics and reloading (used when loading levels)
void Game::rebuildPhysics() {
    CollisionManager& colMgr = CollisionManager::getInstance();

    colMgr.clearPrimitives();

    // loop through all objects and add their colliders back
    for (const auto& obj : gameObjects) {
        if (obj->iconPath == "CUBE") {
            colMgr.addBox(obj->position, obj->scale);
        }
        else if (obj->iconPath == "SPHERE") {
            float radius = obj->scale.x * 0.5f;
            colMgr.addSphere(obj->position, radius);
        }
        else if (obj->physicsType == PhysicsType::BOX) {
            colMgr.addBox(obj->position + obj->colliderOffset, obj->GetRotatedSize());
        }
    }

    // don't forget the event triggers
    initTriggers();

    std::cout << "Physics World Rebuilt (Triggers Restored)." << std::endl;
}

// this is the big setup function where we spawn everything
void Game::initGameObjects() {
    std::cout << "\n>>> Initializing Game World..." << std::endl;

    // loading shaders
    shader3D = std::make_unique<Shader>("shaders/vert.glsl", "shaders/frag.glsl");
    sunShader = std::make_unique<Shader>("shaders/sun.vert", "shaders/sun.frag");
    camera = Camera(glm::vec3(7.0f, 5.0f, 45.0f));

    initShadowMap();
    initTriggers();
    minimap = std::make_unique<Minimap>(windowWidth, windowHeight);

    // creating the sun object
    auto sunModel = Primitives::CreateSphere("resources/ball.jpg");
    sun = std::make_unique<GameObject>(glm::vec3(-200.0f, 80.0f, -200.0f), std::move(sunModel), "SUN");
    sun->scale = glm::vec3(5.0f);
    sun->lightColor = sunColor;
    sun->lightIntensity = sunIntensity;

    // creating the ground
    auto floorModel = Primitives::CreatePlane("resources/4c.jpg", 100.0f);
    if (!floorModel->GetMeshes().empty()) {
        unsigned int normID = Model::TextureFromFile("4n.jpg", "resources", false);
        if (normID) floorModel->GetMeshes()[0].textures.push_back({ normID, "texture_normal", "4n.jpg" });
    }
    auto floor = std::make_unique<GameObject>(glm::vec3(0, -0.05f, 0), std::move(floorModel), "");
    floor->scale = glm::vec3(100.0f, 1.0f, 100.0f);
    gameObjects.push_back(std::move(floor));

    // spawning some test objects and the apple
    internalSpawn("CUBE", glm::vec3(5.0f, 2.0f, 15.0f), glm::vec3(10.0f, 4.0f, 1.0f), -1);
    internalSpawn("SPHERE", glm::vec3(8.0f, 1.0f, 32.0f), glm::vec3(2.0f), -1);
    internalSpawn("ITEM_Apple", glm::vec3(5.0f, 1.0f, 38.0f), glm::vec3(0.5f), -1);

    // configuring the door physics
    ColliderConfig usaCol;
    usaCol.type = PhysicsType::BOX;
    usaCol.customSize = glm::vec3(0.5f, 4.0f, 0.5f);

    // creating the door object
    auto usa = ObjectFactory::CreateObject(
        glm::vec3(10.0f, 2.0f, 10.0f),
        glm::vec3(0.05f),
        "resources/models/door.obj",
        "resources/models/door.obj",
        usaCol,
        { "dc.png", "dn.png", "", "dr.png", "dao.png","dh.png" }
    );
    gameObjects.push_back(std::move(usa));

    // creating the roof
    ColliderConfig roofCol;
    roofCol.type = PhysicsType::BOX;
    roofCol.customSize = glm::vec3(2.0f, 7.0f, 2.0f);
    auto roof = ObjectFactory::CreateObject(
        glm::vec3(10.0f, 0.0f, 10.0f), glm::vec3(0.05f),
        "resources/models/roof.obj", "resources/models/roof.obj",
        roofCol, { "pc.jpg", "pn.jpg", "pd.jpg", "pr.jpg", "pao.jpg", "gh.jpg" }
    );
    gameObjects.push_back(std::move(roof));

    // creating the fire place (reusing roof model? weird but ok)
    ColliderConfig focCol;
    focCol.type = PhysicsType::BOX;
    focCol.customSize = glm::vec3(2.0f, 7.0f, 2.0f);
    auto foc = ObjectFactory::CreateObject(
        glm::vec3(10.0f, 0.0f, 10.0f), glm::vec3(0.05f),
        "resources/models/roof.obj", "resources/models/roof.obj",
        roofCol, { "fc.png", "fn.png", "", "fr.png", "", "fh.png" }
    );
    gameObjects.push_back(std::move(foc));

    // creating the concrete fence
    ColliderConfig gardCol;
    gardCol.type = PhysicsType::BOX;
    gardCol.customSize = glm::vec3(2.0f, 7.0f, 2.0f);
    auto gard = ObjectFactory::CreateObject(
        glm::vec3(10.0f, 0.0f, 10.0f), glm::vec3(3.0f),
        "resources/models/garddebeton.obj", "resources/models/garddebeton.obj",
        gardCol, { "gc.png", "gn.png", "", "gr.png", "", "gh.png" }
    );
    gameObjects.push_back(std::move(gard));

    // creating the window
    ColliderConfig geamCol;
    geamCol.type = PhysicsType::BOX;
    geamCol.customSize = glm::vec3(0.5f, 4.0f, 0.5f);
    auto geam = ObjectFactory::CreateObject(
        glm::vec3(10.0f, 2.0f, 10.0f), glm::vec3(3.0f),
        "resources/models/window.obj", "resources/models/window.obj",
        geamCol, { "wc.png", "wn.png", "", "wr.png", "","wh.png" }
    );
    gameObjects.push_back(std::move(geam));

    // creating the player (the cat)
    player = std::make_unique<Player>(glm::vec3(7.0f, 0.0f, 35.0f), "resources/models/cat.obj", "resources/BrownHead.png");
    player->scale = glm::vec3(0.05f);

    // creating random npc cats
    std::vector<std::string> npcIcons = { "resources/TuxedoCat.png", "resources/GreyCat.png" };
    std::random_device rd; std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, (int)npcIcons.size() - 1);

    gameObjects.push_back(std::make_unique<NPC>(glm::vec3(10.0f, 0.0f, 40.0f), "resources/models/npc.obj", npcIcons[distrib(gen)]));
    gameObjects.push_back(std::make_unique<NPC>(glm::vec3(-3.0f, 0.0f, 18.0f), "resources/models/npc.obj", npcIcons[distrib(gen)]));

    // finalizing physics
    rebuildPhysics();
}

struct ItemDef {
    std::string modelPath;
    std::vector<std::string> textures;
    glm::vec3 defaultScale;
    PhysicsType physics;
};

// helper to define properties for different object types
ItemDef GetItemDefinition(const std::string& type) {
    ItemDef def;

    def.modelPath = type;
    def.defaultScale = glm::vec3(1.0f);
    def.physics = PhysicsType::BOX;

    // defining the door properties
    if (type == "DOOR_USA") {
        def.modelPath = "resources/models/door.obj";
        def.defaultScale = glm::vec3(0.05f);
        def.physics = PhysicsType::BOX;
        def.textures = { "dc.png", "dn.png", "", "dr.png", "dao.png","dh.png" };
    }
    // simple cube
    else if (type == "CUBE") {
        def.modelPath = "CUBE";
        def.physics = PhysicsType::BOX;
    }
    // simple sphere
    else if (type == "SPHERE") {
        def.modelPath = "SPHERE";
        def.physics = PhysicsType::SPHERE;
    }
    // invisible light source
    else if (type == "LIGHT") {
        def.modelPath = "CUBE";
        def.defaultScale = glm::vec3(0.2f);
        def.physics = PhysicsType::NONE;
    }
    // the collectable apple
    else if (type == "ITEM_Apple") {
        def.modelPath = "resources/models/apple.obj";
        def.defaultScale = glm::vec3(0.5f);
        def.physics = PhysicsType::SPHERE;
        def.textures = { "resources/textures/apple_diff.png", "resources/textures/apple_norm.png" };
    }

    return def;
}

// core spawning logic
void Game::internalSpawn(const std::string& type, glm::vec3 pos, glm::vec3 scale, int index, const std::vector<std::string>& texPaths) {

    ItemDef def = GetItemDefinition(type);
    ColliderConfig config(def.physics);

    // special collider for the door
    if (type == "DOOR_USA") {
        config.customSize = glm::vec3(0.5f, 4.0f, 0.5f);
    }

    std::vector<std::string> finalTextures = texPaths;
    if (finalTextures.empty()) {
        finalTextures = def.textures;
    }

    // if scale is basically zero, use the default
    glm::vec3 finalScale = (glm::length(scale) < 0.01f) ? def.defaultScale : scale;

    auto obj = ObjectFactory::CreateObject(pos, finalScale, def.modelPath, type, config, finalTextures);

    // if it's a light, give it high intensity
    if (type == "LIGHT") {
        obj->lightIntensity = 50.0f;
        obj->lightColor = glm::vec3(1.0f, 0.9f, 0.8f);
    }
    // if it's a door, add the trigger zone
    else if (type == "DOOR_USA") {
        CollisionManager::getInstance().addZone(pos, glm::vec3(4.0f, 4.0f, 4.0f), 99);
        std::cout << "Restored Door Trigger Zone 99 at " << pos.x << ", " << pos.z << std::endl;
    }

    // add to the list
    if (obj) {
        if (index >= 0 && index < gameObjects.size()) {
            gameObjects.insert(gameObjects.begin() + index, std::move(obj));
        }
        else {
            gameObjects.push_back(std::move(obj));
        }
    }
}

// deleting objects from the list
void Game::internalDelete(int index) {
    if (index >= 0 && index < gameObjects.size()) {
        if (m_selectedIndex == index) {
            m_selectedObject = nullptr;
            m_selectedIndex = -1;
        }
        gameObjects.erase(gameObjects.begin() + index);
        rebuildPhysics();
    }
}

// handling editor actions (spawn, delete, move) so we can undo them later
void Game::executeAction(const EditorAction& action) {
    if (action.type == EditorAction::ACT_ADD) {
        // spawn object
        internalSpawn(action.objectType, action.position, action.scale, -1, action.texturePaths);
        if (!gameObjects.empty()) gameObjects.back()->rotation = action.rotation;
    }
    else if (action.type == EditorAction::ACT_DELETE) {
        // delete object
        internalDelete(action.index);
    }
    else if (action.type == EditorAction::ACT_TRANSFORM) {
        // move object
        if (action.index >= 0 && action.index < gameObjects.size()) {
            gameObjects[action.index]->position = action.position;
        }
    }
    // add to undo history and clear redo history
    m_undoStack.push(action);
    while (!m_redoStack.empty()) m_redoStack.pop();
}

// undo logic: reversing the last action
void Game::undo() {
    if (m_undoStack.empty()) return;
    EditorAction last = m_undoStack.top(); m_undoStack.pop();

    if (last.type == EditorAction::ACT_ADD) {
        // if we added it, delete it now
        internalDelete(gameObjects.size() - 1);
    }
    else if (last.type == EditorAction::ACT_DELETE) {
        // if we deleted it, bring it back
        internalSpawn(last.objectType, last.position, last.scale, last.index, last.texturePaths);
        if (last.index < gameObjects.size()) gameObjects[last.index]->rotation = last.rotation;
    }
    else if (last.type == EditorAction::ACT_TRANSFORM) {
        // if we moved it, move it back
        if (last.index >= 0 && last.index < gameObjects.size()) {
            gameObjects[last.index]->position = last.oldPosition;
            if (m_selectedIndex == last.index) m_selectedObject = gameObjects[last.index].get();
        }
    }
    m_redoStack.push(last);
}

// redo logic: doing the thing we just undid
void Game::redo() {
    if (m_redoStack.empty()) return;
    EditorAction redo = m_redoStack.top(); m_redoStack.pop();

    if (redo.type == EditorAction::ACT_ADD) {
        internalSpawn(redo.objectType, redo.position, redo.scale, -1, redo.texturePaths);
        if (!gameObjects.empty()) gameObjects.back()->rotation = redo.rotation;
    }
    else if (redo.type == EditorAction::ACT_DELETE) {
        internalDelete(redo.index);
    }
    else if (redo.type == EditorAction::ACT_TRANSFORM) {
        if (redo.index >= 0 && redo.index < gameObjects.size()) {
            gameObjects[redo.index]->position = redo.position;
        }
    }
    m_undoStack.push(redo);
}


// the main game loop!
void Game::run() {
    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        // calculating delta time (how long the last frame took)
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // starting new ui frames
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // core loop steps
        handleInput();
        update(deltaTime);
        render(deltaTime);

        // deciding which ui to draw
        if (m_editorMode) {
            renderUI();
        }
        else {
            drawInventoryUI();
            DrawTaskUI();
        }

        // finishing up rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        updateWindowTitle(deltaTime);

        glfwSwapBuffers(window);
        InputManager::getInstance().update();
        glfwPollEvents();
    }
}

// nuking textures from an object
void ClearObjectTextures(GameObject* obj) {
    if (!obj || !obj->model) return;
    for (auto& mesh : obj->model->GetMeshes()) {
        mesh.textures.clear();
    }
}

// master ui render function
void Game::renderUI() {
    drawHierarchy();
    drawInspector();
    drawGlobalSettings();
}

// drawing the list of objects in the scene
void Game::drawHierarchy() {
    ImGui::Begin("Scene Hierarchy");
    for (int i = 0; i < gameObjects.size(); i++) {
        std::string label = gameObjects[i]->iconPath;
        if (label.empty()) label = "Object";
        label += " [" + std::to_string(i) + "]";

        // checking if this specific object is selected
        bool isSelected = false;
        for (int idx : m_selectedIndices) {
            if (idx == i) { isSelected = true; break; }
        }

        if (ImGui::Selectable(label.c_str(), isSelected)) {
            // handle multi-selection with shift key
            if (ImGui::GetIO().KeyShift) {
                if (!isSelected) {
                    m_selectedObjects.push_back(gameObjects[i].get());
                    m_selectedIndices.push_back(i);
                }
            }
            else {
                // single selection
                clearSelection();
                m_selectedObjects.push_back(gameObjects[i].get());
                m_selectedIndices.push_back(i);
            }

            m_selectedObject = gameObjects[i].get();
            m_selectedIndex = i;
        }
    }
    ImGui::End();
}

// drawing the hotbar at the bottom of the screen
void Game::drawInventoryUI() {
    if (!player) return;

    // setting up window flags for the hud
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 0.3f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    // centering the hotbar
    float estimatedWidth = player->inventory.slots.size() * 68.0f;
    ImGui::SetNextWindowPos(ImVec2((windowWidth - estimatedWidth) / 2.0f, windowHeight - 90.0f));

    ImGui::Begin("Hotbar", nullptr, flags);

    for (int i = 0; i < player->inventory.slots.size(); i++) {
        InventoryItem& item = player->inventory.slots[i];
        bool isSelected = (i == player->inventory.selectedSlot);

        // highlight the selected slot with gold color
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.8f, 0.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 3.0f);
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        }

        // create the label (item name and count)
        std::string label;
        if (item.isEmpty()) {
            label = "\n";
        }
        else {
            std::string display = item.name.substr(0, 8);
            label = display + "\n x" + std::to_string(item.quantity);
        }

        std::string btnId = "##InvSlot" + std::to_string(i);

        // the button itself
        if (ImGui::Button((label + btnId).c_str(), ImVec2(60, 60))) {
            player->inventory.selectedSlot = i;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        // draw buttons in a row
        if (i < player->inventory.slots.size() - 1) {
            ImGui::SameLine();
        }
    }

    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

// details panel for editing objects
void Game::drawInspector() {
    ImGui::Begin("Inspector");

    if (ImGui::Button("UNDO")) undo();
    ImGui::SameLine();
    if (ImGui::Button("REDO")) redo();

    ImGui::Separator();

    // if something is selected, show options
    if (!m_selectedObjects.empty()) {

        if (m_selectedObjects.size() > 1) {
            ImGui::Text("Selected Group: %d Objects", (int)m_selectedObjects.size());
        }
        else if (m_selectedObject) {
            ImGui::Text("Selected: %s", m_selectedObject->iconPath.c_str());
        }


        std::string dupLabel = (m_selectedObjects.size() > 1) ? "DUPLICATE GROUP" : "DUPLICATE OBJECT";

        // duplicating objects
        if (ImGui::Button(dupLabel.c_str())) {
            std::vector<EditorAction> actionsToExecute;
            glm::vec3 groupOffset(2.0f, 0.0f, 2.0f);
            for (GameObject* obj : m_selectedObjects) {
                EditorAction act;
                act.type = EditorAction::ACT_ADD;
                act.objectType = obj->iconPath;
                act.position = obj->position + groupOffset;
                act.rotation = obj->rotation;
                act.scale = obj->scale;
                act.index = -1;
                act.texturePaths = GetObjectTextures(obj);
                actionsToExecute.push_back(act);
            }

            // executing the copy
            for (const auto& act : actionsToExecute) {
                executeAction(act);
            }

            clearSelection();
        }

        ImGui::SameLine();

        // drag and drop button styling
        bool stylePushed = m_isDragging;
        if (stylePushed) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.8f, 0, 1));

        std::string dragLabel = m_isDragging ? "DROP (G)" : "CARRY (G)";
        if (ImGui::Button(dragLabel.c_str())) {
            if (!m_isDragging) {
                // start carrying
                m_isDragging = true;
                m_dragOffsets.clear();
                GameObject* leader = m_selectedObjects.back();
                for (auto* obj : m_selectedObjects) {
                    m_dragOffsets.push_back(obj->position - leader->position);
                }
            }
            else {
                // stop carrying
                m_isDragging = false;
                m_dragOffsets.clear();
            }
        }
        if (stylePushed) ImGui::PopStyleColor();

        ImGui::Separator();
    }

    // specific object details
    if (m_selectedObject) {

        if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::DragFloat3("Position", &m_selectedObject->position.x, 0.1f);
            ImGui::DragFloat3("Rotation", &m_selectedObject->rotation.x, 1.0f);
            ImGui::DragFloat3("Scale", &m_selectedObject->scale.x, 0.1f);

            ImGui::Spacing();
            if (ImGui::Button("Snap to Camera View (G)", ImVec2(-1, 0))) {
                m_selectedObject->position = camera.Position + (camera.Front * 5.0f);
            }
        }

        // material editing (shiny, rough, etc)
        if (ImGui::CollapsingHeader("Material Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Base Properties:");
            ImGui::SliderFloat("Metallic", &m_selectedObject->pbr_metallic, 0.0f, 1.0f);
            ImGui::SliderFloat("Roughness", &m_selectedObject->pbr_roughness, 0.0f, 1.0f);

            ImGui::Separator();
            if (ImGui::Button("Clear All Textures")) ClearObjectTextures(m_selectedObject);

            static char texBuf[64] = "";
            ImGui::InputText("Filename", texBuf, 64);

            // buttons to load specific textures
            if (ImGui::Button("Add Diffuse")) AddTextureToObject(m_selectedObject, texBuf, "texture_diffuse");
            ImGui::SameLine();
            if (ImGui::Button("Add Normal")) AddTextureToObject(m_selectedObject, texBuf, "texture_normal");

            if (ImGui::Button("Add Metal")) AddTextureToObject(m_selectedObject, texBuf, "texture_metallic");
            ImGui::SameLine();
            if (ImGui::Button("Add Rough")) AddTextureToObject(m_selectedObject, texBuf, "texture_roughness");

            if (ImGui::Button("Add AO")) AddTextureToObject(m_selectedObject, texBuf, "texture_ao");
            ImGui::SameLine();
            if (ImGui::Button("Add Height")) AddTextureToObject(m_selectedObject, texBuf, "texture_height");
        }

        // light settings if it's a light source
        if (m_selectedObject->iconPath == "LIGHT") {
            if (ImGui::CollapsingHeader("Light Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::ColorEdit3("Light Color", &m_selectedObject->lightColor.x);
                ImGui::DragFloat("Intensity", &m_selectedObject->lightIntensity, 0.5f, 0.0f, 1000.0f);
            }
        }

        ImGui::Separator();

        // button to delete the selected object
        if (ImGui::Button("Delete Selected", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
            EditorAction act;
            act.type = EditorAction::ACT_DELETE;
            act.objectType = m_selectedObject->iconPath;
            act.position = m_selectedObject->position;
            act.scale = m_selectedObject->scale;
            act.index = m_selectedIndex;
            act.rotation = m_selectedObject->rotation;
            act.texturePaths = GetObjectTextures(m_selectedObject);

            executeAction(act);

            clearSelection();
        }
    }
    ImGui::End();
}
void Game::drawGlobalSettings() {
    ImGui::Begin("Global Settings");

    // pbr settings that affect everything that doesn't have a texture
    if (ImGui::CollapsingHeader("Global PBR Values", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Fallback values if no texture is loaded:");
        ImGui::SliderFloat("Metallic", &m_metallic, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness", &m_roughness, 0.01f, 1.0f);
        ImGui::SliderFloat("AO", &m_ao, 0.0f, 1.0f);
        ImGui::DragFloat("Light Intensity", &m_lightIntensity, 0.5f, 0.0f, 500.0f);
    }
    ImGui::Separator();

    // editing the sun properties
    if (ImGui::CollapsingHeader("Sun Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Main Directional Light Source");

        if (sun) {
            ImGui::DragFloat3("Sun Position", &sun->position.x, 0.5f);
            ImGui::ColorEdit3("Sun Color", &sun->lightColor.x);
            ImGui::DragFloat("Sun Intensity", &sun->lightIntensity, 0.5f, 0.0f, 100000.0f);
            ImGui::DragFloat3("Sun Scale", &sun->scale.x, 0.1f, 0.1f, 20.0f);

            ImGui::Separator();
            if (ImGui::CollapsingHeader("Day/Night Cycle", ImGuiTreeNodeFlags_DefaultOpen)) {

                // showing current in-game time
                int h = (int)m_dayTime;
                int m = (int)((m_dayTime - h) * 60);
                ImGui::Text("Current Time: %02d:%02d", h, m);
                ImGui::SliderFloat("##TimeSlider", &m_dayTime, 0.0f, 24.0f, "%.2f Hrs");

                // pause/resume button
                if (ImGui::Button(m_isTimePaused ? "RESUME" : "PAUSE")) {
                    m_isTimePaused = !m_isTimePaused;
                }
                ImGui::SameLine();
                ImGui::SliderFloat("Speed", &m_timeSpeed, 0.0f, 2.0f, "%.3f");
                ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), "0.067 = 6 min/day");

                ImGui::Separator();

                // shortcuts to change time quickly
                ImGui::Text("Jump To:");
                if (ImGui::Button("Sunrise (6:00)")) m_dayTime = 6.0f;
                ImGui::SameLine();
                if (ImGui::Button("Noon (12:00)")) m_dayTime = 12.0f;
                ImGui::SameLine();
                if (ImGui::Button("Sunset (18:00)")) m_dayTime = 18.0f;
                ImGui::SameLine();
                if (ImGui::Button("Midnight (0:00)")) m_dayTime = 0.0f;
            }
        }
    }
    ImGui::Separator();

    // spawn buttons
    ImGui::Text("Tools");
    if (ImGui::Button("Spawn Cube")) {
        EditorAction act; act.type = EditorAction::ACT_ADD; act.objectType = "CUBE";
        act.position = camera.Position + camera.Front * 5.0f; act.scale = glm::vec3(2.0f); act.index = -1;
        executeAction(act);
    }
    if (ImGui::Button("Spawn Sphere")) {
        EditorAction act; act.type = EditorAction::ACT_ADD; act.objectType = "SPHERE";
        act.position = camera.Position + camera.Front * 5.0f; act.scale = glm::vec3(2.0f); act.index = -1;
        executeAction(act);
    }
    if (ImGui::Button("Spawn Light")) {
        EditorAction act; act.type = EditorAction::ACT_ADD; act.objectType = "LIGHT";
        act.position = camera.Position + camera.Front * 5.0f; act.scale = glm::vec3(0.5f); act.index = -1;
        executeAction(act);
    }

    ImGui::Separator();
    if (ImGui::Button("Load Level", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
        loadLevel("level_data.txt");
    }

    ImGui::End();
}


// function to drop an item from the inventory onto the ground
void Game::dropCurrentItem() {
    if (!player) return;

    int slotIndex = player->inventory.selectedSlot;
    InventoryItem& item = player->inventory.slots[slotIndex];

    if (item.isEmpty()) return;

    // calculating where to spawn the dropped item
    glm::vec3 spawnPos = player->position + (camera.Front * 1.5f);
    spawnPos.y += 1.0f;

    std::string spawnName = "ITEM_" + item.name;

    // spawning it
    internalSpawn(spawnName, spawnPos, glm::vec3(0), -1);

    // decreasing the count in inventory
    item.quantity--;
    if (item.quantity <= 0) {
        item.name = "";
        item.quantity = 0;
    }

    std::cout << "Dropped: " << spawnName << std::endl;
}

// handling all user input
void Game::handleInput() {
    InputManager& input = InputManager::getInstance();
    ImGuiIO& io = ImGui::GetIO();

    // close app on escape
    if (input.isKeyPressed(GLFW_KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);

    // toggling between editor and game mode
    if (input.isKeyPressed(GLFW_KEY_TAB)) {
        m_editorMode = !m_editorMode;
        if (m_editorMode) {
            isMouseCaptured = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            camera.Zoom = 90.0f;
        }
        else {
            isMouseCaptured = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            camera.Zoom = 45.0f;
            m_timeSinceMouseInput = 0.0f;
        }
    }

    if (m_editorMode) {
        // releasing mouse cursor with alt
        if (input.isKeyPressed(GLFW_KEY_LEFT_ALT)) {
            isMouseCaptured = !isMouseCaptured;
            glfwSetInputMode(window, GLFW_CURSOR, isMouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }

        // carrying objects logic
        if (!m_selectedObjects.empty() && input.isKeyPressed(GLFW_KEY_G)) {
            if (!m_isDragging) {
                // start dragging
                m_isDragging = true;
                m_dragOffsets.clear();

                GameObject* leader = m_selectedObjects.back();

                for (auto* obj : m_selectedObjects) {
                    m_dragOffsets.push_back(obj->position - leader->position);
                }
            }
            else {
                // stop dragging
                m_isDragging = false;
                m_dragOffsets.clear();
            }
        }

        // hotkeys for spawning and saving
        glm::vec3 spawnPos = camera.Position + (camera.Front * 5.0f);
        if (input.isKeyPressed(GLFW_KEY_1)) internalSpawn("CUBE", spawnPos, glm::vec3(2.0f), -1);
        if (input.isKeyPressed(GLFW_KEY_2)) internalSpawn("SPHERE", spawnPos, glm::vec3(2.0f), -1);
        if (input.isKeyPressed(GLFW_KEY_3)) internalSpawn("LIGHT", spawnPos, glm::vec3(0.5f), -1);
        if (input.isKeyPressed(GLFW_KEY_K)) saveLevel("level_data.txt");
        if (input.isKeyPressed(GLFW_KEY_L)) loadLevel("level_data.txt");

        // free camera movement (flying mode)
        if (isMouseCaptured) {
            float currentSpeed = m_flySpeed;
            if (input.isKeyHeld(GLFW_KEY_LEFT_SHIFT)) currentSpeed *= 3.0f;
            float moveAmount = currentSpeed * 0.016f;
            if (input.isKeyHeld(GLFW_KEY_W)) camera.Position += camera.Front * moveAmount;
            if (input.isKeyHeld(GLFW_KEY_S)) camera.Position -= camera.Front * moveAmount;
            if (input.isKeyHeld(GLFW_KEY_A)) camera.Position -= camera.Right * moveAmount;
            if (input.isKeyHeld(GLFW_KEY_D)) camera.Position += camera.Right * moveAmount;
            if (input.isKeyHeld(GLFW_KEY_Q)) camera.Position -= camera.Up * moveAmount;
            if (input.isKeyHeld(GLFW_KEY_E)) camera.Position += camera.Up * moveAmount;
            camera.ProcessMouseMovement(input.getMouseDelta().x, -input.getMouseDelta().y);
        }

        // raycasting to select objects with right click
        if (!isMouseCaptured && !io.WantCaptureMouse) {
            static bool lastRightState = false;
            bool currentRightState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

            if (currentRightState && !lastRightState) {
                // finding the closest object under the mouse cursor
                int closestIndex = -1; float closestDist = 9999.0f;
                for (int i = 0; i < gameObjects.size(); i++) {
                    glm::vec3 toObj = gameObjects[i]->position - camera.Position;
                    float t = glm::dot(toObj, camera.Front);
                    if (t > 0.0f) {
                        float distToRay = glm::length((camera.Position + (camera.Front * t)) - gameObjects[i]->position);
                        float radius = std::max(gameObjects[i]->scale.x, gameObjects[i]->scale.y) * 0.7f;
                        if (distToRay < radius) {
                            if (t < closestDist) { closestDist = t; closestIndex = i; }
                        }
                    }
                }

                if (closestIndex != -1) {
                    GameObject* clickedObj = gameObjects[closestIndex].get();
                    bool isShift = input.isKeyHeld(GLFW_KEY_LEFT_SHIFT);

                    if (isShift) {
                        // adding to selection
                        if (!isObjectSelected(clickedObj)) {
                            m_selectedObjects.push_back(clickedObj);
                            m_selectedIndices.push_back(closestIndex);
                        }
                    }
                    else {
                        // selecting just one
                        clearSelection();
                        m_selectedObjects.push_back(clickedObj);
                        m_selectedIndices.push_back(closestIndex);
                    }
                    m_selectedObject = m_selectedObjects.back();
                    m_selectedIndex = m_selectedIndices.back();
                }
                else {
                    // clicked on nothing, so clear selection
                    if (!input.isKeyHeld(GLFW_KEY_LEFT_SHIFT)) clearSelection();
                }
            }
            lastRightState = currentRightState;
        }
    }
    else {
        // game mode inputs
        if (input.isKeyPressed(GLFW_KEY_C)) {
            isMouseCaptured = !isMouseCaptured;
            glfwSetInputMode(window, GLFW_CURSOR, isMouseCaptured ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }

        // camera control for the player
        if (isMouseCaptured) {
            glm::vec2 delta = input.getMouseDelta();
            if (glm::length(delta) > 0.1f) {
                if (m_timeSinceMouseInput > 5.0f) delta = glm::vec2(0.0f);
                m_timeSinceMouseInput = 0.0f;
            }
            m_cameraYaw -= delta.x * 0.1f;
            m_cameraPitch += delta.y * 0.1f;
            m_cameraPitch = glm::clamp(m_cameraPitch, -10.0f, 89.0f);
        }

        // scrolling to change inventory slot
        float scroll = input.getScrollY();
        if (scroll != 0) {
            int& slot = player->inventory.selectedSlot;
            int max = (int)player->inventory.slots.size();
            if (scroll > 0) slot--; else slot++;
            if (slot < 0) slot = max - 1;
            if (slot >= max) slot = 0;
        }

        // direct inventory slot selection
        if (input.isKeyPressed(GLFW_KEY_1)) player->inventory.selectedSlot = 0;
        if (input.isKeyPressed(GLFW_KEY_2)) player->inventory.selectedSlot = 1;
        if (input.isKeyPressed(GLFW_KEY_3)) player->inventory.selectedSlot = 2;
        if (input.isKeyPressed(GLFW_KEY_4)) player->inventory.selectedSlot = 3;
        if (input.isKeyPressed(GLFW_KEY_5)) player->inventory.selectedSlot = 4;
        if (input.isKeyPressed(GLFW_KEY_6)) player->inventory.selectedSlot = 5;
        if (input.isKeyPressed(GLFW_KEY_7)) player->inventory.selectedSlot = 6;
        if (input.isKeyPressed(GLFW_KEY_8)) player->inventory.selectedSlot = 7;
        if (input.isKeyPressed(GLFW_KEY_9)) player->inventory.selectedSlot = 8;
        if (input.isKeyPressed(GLFW_KEY_0)) player->inventory.selectedSlot = 9;

        // drop item
        if (input.isKeyPressed(GLFW_KEY_Q)) {
            dropCurrentItem();
        }
        if (input.isKeyPressed(GLFW_KEY_M)) minimap->toggleMaximized();
        if (input.isKeyPressed(GLFW_KEY_F)) m_flashLightOn = !m_flashLightOn;
    }
}

// updating the sun position based on time of day
void Game::updateDayCycle(float deltaTime) {
    m_dayTime += deltaTime * m_timeSpeed;
    if (m_dayTime >= 24.0f) m_dayTime -= 24.0f;

    // keyframes for where the sun should be at different hours
    SunKeyframe keyframes[] = {
        { 0.0f,   glm::vec3(0, -50, 0),    glm::vec3(0.2f, 0.2f, 0.5f), 15000.0f },
        { 5.0f,   glm::vec3(200, -10, 200),glm::vec3(0.2f, 0.2f, 0.4f), 10000.0f },
        { 6.0f,   glm::vec3(200, 20, 200), glm::vec3(1.0f, 0.6f, 0.3f), 80000.0f },
        { 12.0f,  glm::vec3(0, 150, 0),    glm::vec3(1.0f, 1.0f, 0.95f),100000.0f },
        { 18.0f,  glm::vec3(-200, 20, -200),glm::vec3(1.0f, 0.6f, 0.3f),80000.0f },
        { 19.0f,  glm::vec3(-200, -10, -200),glm::vec3(0.2f, 0.2f, 0.4f),10000.0f },
        { 24.0f,  glm::vec3(0, -50, 0),    glm::vec3(0.2f, 0.2f, 0.5f), 15000.0f }
    };

    // finding which two keyframes we are between
    int currentKey = 0;
    for (int i = 0; i < 6; i++) {
        if (m_dayTime >= keyframes[i].time && m_dayTime < keyframes[i + 1].time) {
            currentKey = i; break;
        }
    }
    SunKeyframe start = keyframes[currentKey];
    SunKeyframe end = keyframes[currentKey + 1];

    // calculating interpolation factor
    float range = end.time - start.time;
    float offset = m_dayTime - start.time;
    float t = offset / range;
    t = t * t * (3.0f - 2.0f * t); // smoothing the transition

    if (sun) {
        // actually moving the sun
        sun->position = glm::mix(start.position, end.position, t);
        sun->lightColor = glm::mix(start.color, end.color, t);
        sun->lightIntensity = glm::mix(start.intensity, end.intensity, t);
    }
}


// main update loop for game logic
void Game::update(float deltaTime) {
    if (!m_editorMode) {

        // handling quest progress
        if (m_tasks.size() >= 4) {

            // task 1: moving away from start
            if (!m_tasks[0].isCompleted) {
                if (glm::distance(player->position, glm::vec3(7.0f, 0.0f, 35.0f)) > 5.0f) {
                    m_tasks[0].isCompleted = true;
                }
            }

            // task 2: turning on flashlight
            if (!m_tasks[1].isCompleted && m_flashLightOn) {
                m_tasks[1].isCompleted = true;
            }

            // task 3: finding the apple in inventory
            if (!m_tasks[2].isCompleted) {
                for (const auto& slot : player->inventory.slots) {
                    if (!slot.isEmpty() && slot.name == "Apple") {
                        m_tasks[2].isCompleted = true;
                        break;
                    }
                }
            }

            // task 4: finding the door zone (id 99)
            if (!m_tasks[3].isCompleted) {
                int tID = CollisionManager::getInstance().checkTriggers(player->position);
                if (tID == 99) {
                    m_tasks[3].isCompleted = true;
                }
            }
        }

        // picking up items logic
        for (int i = gameObjects.size() - 1; i >= 0; i--) {
            GameObject* obj = gameObjects[i].get();
            if (obj->iconPath.find("ITEM_") == 0) {
                float dist = glm::distance(player->position, obj->position);
                if (dist < 2.0f) {
                    std::string itemName = obj->iconPath.substr(5);
                    if (player->inventory.addItem(itemName, 1)) {
                        std::cout << "Collected: " << itemName << std::endl;
                        internalDelete(i);
                    }
                }
            }
        }

        // checking triggers for doors
        int triggerID = CollisionManager::getInstance().checkTriggers(player->position);
        bool shouldOpen = (triggerID == 99);

        // door animation logic
        for (auto& obj : gameObjects) {
            if (obj->iconPath == "resources/models/door.obj") {

                float doorSpeed = 120.0f;
                float targetY = shouldOpen ? 90.0f : 0.0f;
                bool needsUpdate = false;

                if (shouldOpen) {
                    if (obj->rotation.y < targetY) {
                        obj->rotation.y += doorSpeed * deltaTime;
                        if (obj->rotation.y > targetY) obj->rotation.y = targetY;
                        needsUpdate = true;
                    }
                }
                else {
                    if (obj->rotation.y > targetY) {
                        obj->rotation.y -= doorSpeed * deltaTime;
                        if (obj->rotation.y < targetY) obj->rotation.y = targetY;
                        needsUpdate = true;
                    }
                }

                // updating physics box for the moving door
                if (needsUpdate && obj->physicsID != -1) {
                    glm::vec3 newSize = obj->GetRotatedSize();
                    CollisionManager::getInstance().updateBox(obj->physicsID, obj->position, newSize);
                }
                break;
            }
        }

        // updating time
        if (!m_isTimePaused) {
            updateDayCycle(deltaTime);
        }

        player->update(deltaTime, camera);

        // updating physics for all other moving objects
        for (auto& obj : gameObjects) {
            obj->update(deltaTime, camera);

            if (obj->physicsID != -1 && obj->iconPath != "resources/models/door.obj") {
                if (obj->physicsType == PhysicsType::BOX) {
                    glm::vec3 center = obj->position + obj->colliderOffset;
                    glm::vec3 newSize = obj->GetRotatedSize();
                    CollisionManager::getInstance().updateBox(obj->physicsID, center, newSize);
                }
                else if (obj->physicsType == PhysicsType::SPHERE) {
                    CollisionManager::getInstance().updateSphere(obj->physicsID, obj->position, obj->originalColliderSize.x * 0.5f);
                }
            }
        }

        // automatic camera rotation if idle
        m_timeSinceMouseInput += deltaTime;
        if (m_timeSinceMouseInput > 5.0f) {
            float targetYaw = player->rotation.y;
            float diff = targetYaw - m_cameraYaw;
            while (diff < -180.0f) diff += 360.0f;
            while (diff > 180.0f) diff -= 360.0f;
            m_cameraYaw += diff * 3.0f * deltaTime;
            m_cameraPitch = glm::mix(m_cameraPitch, 20.0f, 2.0f * deltaTime);
        }

        // calculating camera position (third person follow)
        glm::vec3 target = player->position + glm::vec3(0, 2.0f, 0);
        float hDistance = m_cameraDistance * cos(glm::radians(m_cameraPitch));
        float vDistance = m_cameraDistance * sin(glm::radians(m_cameraPitch));
        float offX = hDistance * sin(glm::radians(m_cameraYaw));
        float offZ = hDistance * cos(glm::radians(m_cameraYaw));

        glm::vec3 desiredPos;
        desiredPos.x = target.x - offX;
        desiredPos.y = target.y + vDistance;
        desiredPos.z = target.z - offZ;
        if (desiredPos.y < 0.5f) desiredPos.y = 0.5f;

        // smoothing the camera movement
        camera.Position = glm::mix(camera.Position, desiredPos, 10.0f * deltaTime);
        camera.Front = glm::normalize(target - camera.Position);
        camera.Right = glm::normalize(glm::cross(camera.Front, glm::vec3(0, 1, 0)));
        camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));

        // checking if we entered a story trigger zone
        int tID = CollisionManager::getInstance().checkTriggers(player->position);
        if (tID != -1 && tID != 99) {
            player->setInTriggerZone(true);
            minimap->setTargetZoom(10.0f);
            handleStoryEvent(tID);
        }
        else {
            player->setInTriggerZone(false);
            minimap->setTargetZoom(20.0f);
        }
    }
    else {
        // editor mode updates (dragging objects)
        if (m_isDragging && !m_selectedObjects.empty()) {
            float distance = 5.0f;
            glm::vec3 leaderTargetPos = camera.Position + (camera.Front * distance);
            for (size_t i = 0; i < m_selectedObjects.size(); i++) {
                if (i < m_dragOffsets.size()) {
                    m_selectedObjects[i]->position = leaderTargetPos + m_dragOffsets[i];
                }
            }
        }
    }
}

// drawing everything to the screen
void Game::render(float deltaTime) {
    if (windowWidth == 0) return;

    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)windowWidth / (float)windowHeight, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    // preparing shadow pass
    glm::vec3 lightDir = glm::normalize(sun->position);

    glm::vec3 shadowTarget = player->position;

    glm::vec3 shadowPos = shadowTarget + (lightDir * 50.0f);
    glm::mat4 lightProjection = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, 1.0f, 150.0f);

    glm::mat4 lightView = glm::lookAt(shadowPos, shadowTarget, glm::vec3(0.0, 1.0, 0.0));

    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    // render pass 1: render to shadow map (depth only)
    depthShader->use();
    depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    drawSceneToShader(*depthShader);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glCullFace(GL_BACK);

    // render pass 2: normal scene rendering
    glViewport(0, 0, windowWidth, windowHeight);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // drawing the skybox
    if (m_skybox) {
        glm::mat4 skyboxView = glm::mat4(glm::mat3(view));

        glDepthFunc(GL_LEQUAL);
        m_skybox->draw(skyboxView, projection);
        glDepthFunc(GL_LESS);

        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox->getTextureID());
    }

    // drawing the sun sphere
    if (sun && sunShader) {
        sunShader->use();
        sunShader->setMat4("projection", projection);
        sunShader->setMat4("view", view);
        sunShader->setVec3("sunColor", sun->lightColor);
        sunShader->setFloat("sunIntensity", sun->lightIntensity);

        glm::mat4 sunModel = glm::mat4(1.0f);
        sunModel = glm::translate(sunModel, sun->position);
        sunModel = glm::scale(sunModel, sun->scale);
        sunShader->setMat4("model", sunModel);

        glDepthMask(GL_FALSE);
        sun->draw3D(*sunShader);
        glDepthMask(GL_TRUE);
    }

    // setting up main shader for objects
    shader3D->use();
    shader3D->setMat4("projection", projection);
    shader3D->setMat4("view", view);
    shader3D->setVec3("viewPos", camera.Position);
    shader3D->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shader3D->setInt("skybox", 15);

    // binding the shadow map
    shader3D->setInt("shadowMap", 10);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, depthMap);

    // collecting all light sources
    std::vector<glm::vec3> lightPositions;
    std::vector<glm::vec3> lightColors;

    if (sun) {
        lightPositions.push_back(sun->position);
        lightColors.push_back(sun->lightColor * sun->lightIntensity);
    }
    else {
        // fallback light if sun is missing
        lightPositions.push_back(glm::vec3(-10.0f, 30.0f, -10.0f));
        lightColors.push_back(glm::vec3(20.0f, 18.0f, 15.0f));
    }

    // flashlight logic
    if (m_flashLightOn) {
        glm::vec3 lightPos = player->position + glm::vec3(0.0f, 1.0f, 0.0f);
        lightPositions.push_back(lightPos);
        lightColors.push_back(glm::vec3(10.0f, 10.0f, 8.0f));
    }

    // other scene lights
    for (const auto& obj : gameObjects) {
        if (obj->iconPath == "LIGHT") {
            lightPositions.push_back(obj->position);
            lightColors.push_back(obj->lightColor * obj->lightIntensity);
        }
    }

    // sending lights to shader
    int activeLights = 0;
    for (int i = 0; i < 64 && i < (int)lightPositions.size(); i++) {
        shader3D->setVec3("lightPos[" + std::to_string(i) + "]", lightPositions[i]);
        shader3D->setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        activeLights++;
    }
    shader3D->setInt("nrLights", activeLights);

    // setting global material properties
    shader3D->setFloat("metallicVal", m_metallic);
    shader3D->setFloat("roughnessVal", m_roughness);
    shader3D->setFloat("aoVal", m_ao);
    shader3D->setBool("hasDispMap", m_debugMode != 4);

    // drawing all objects
    drawSceneToShader(*shader3D);

    // updating minimap
    std::vector<GameObject*> list;
    for (auto& obj : gameObjects) list.push_back(obj.get());
    minimap->draw(player.get(), list, deltaTime);
}

// triggering specific story moments
void Game::handleStoryEvent(int id) {
    if (m_processedEvents[id]) return;
    if (id == 1) {
        // play intro sound
        AudioManager::getInstance().playExclusive("resources/intro.wav");
        if (!m_storyInitialized) { player->setName("Whiskers"); m_storyInitialized = true; }
    }
    m_processedEvents[id] = true;
}

// window resize handler
void Game::onResize(int w, int h) { windowWidth = w; windowHeight = h; if (minimap) minimap->onWindowResize(w, h); }

// changing the window title to show fps
void Game::updateWindowTitle(float dt) {
    static double t = 0.0; t += dt;
    if (t >= 1.0) {
        std::stringstream ss;
        ss << " Project WHISKERS | FPS: " << int(1.0 / dt) << " | " << (m_editorMode ? "EDITOR" : "GAME");
        glfwSetWindowTitle(window, ss.str().c_str());
        t = 0.0;
    }
}

// helper to get texture path from an object
std::string getTexturePath(GameObject* obj, const std::string& textureType) {
    if (!obj || !obj->model) return "NONE";
    auto& meshes = obj->model->GetMeshes();
    if (meshes.empty()) return "NONE";

    for (const auto& tex : meshes[0].textures) {
        if (tex.type == textureType) {
            return tex.path;
        }
    }
    return "NONE";
}

// writing level data to a text file
void Game::saveLevel(const std::string& filename) {
    std::ofstream f(filename);
    if (!f.is_open()) return;

    std::cout << "Saving Level..." << std::endl;
    for (const auto& obj : gameObjects) {
        std::string type = obj->iconPath;
        if (type.empty() || type == "SUN") continue;

        // writing position, rotation, scale
        f << type << " ";
        f << obj->position.x << " " << obj->position.y << " " << obj->position.z << " ";
        f << obj->rotation.x << " " << obj->rotation.y << " " << obj->rotation.z << " ";
        f << obj->scale.x << " " << obj->scale.y << " " << obj->scale.z << " ";

        // writing textures
        std::vector<std::string> texs = GetObjectTextures(obj.get());
        for (const std::string& t : texs) f << (t.empty() ? "NONE" : t) << " ";
        f << "\n";
    }
    f.close();
    std::cout << "Level saved!" << std::endl;
}

// reading level data and spawning objects
void Game::loadLevel(const std::string& filename) {
    std::ifstream f(filename);
    if (!f.is_open()) {
        std::cerr << "Failed to open level file: " << filename << std::endl;
        return;
    }

    // clearing current world
    gameObjects.clear();
    clearSelection();
    while (!m_undoStack.empty()) m_undoStack.pop();
    while (!m_redoStack.empty()) m_redoStack.pop();

    // resetting the floor
    auto floorModel = Primitives::CreatePlane("resources/4c.jpg", 100.0f);
    if (!floorModel->GetMeshes().empty()) {
        unsigned int normID = Model::TextureFromFile("1n.jpg", "resources", false);
        if (normID) floorModel->GetMeshes()[0].textures.push_back({ normID, "texture_normal", "4n.jpg" });
    }
    auto floor = std::make_unique<GameObject>(glm::vec3(0, -0.05f, 0), std::move(floorModel), "");
    floor->scale = glm::vec3(300.0f, 1.0f, 300.0f);
    gameObjects.push_back(std::move(floor));

    // reading file line by line
    std::string type;
    float px, py, pz, rx, ry, rz, sx, sy, sz;
    std::string tDiff, tNorm, tMet, tRough, tAO, tHeight;

    while (f >> type >> px >> py >> pz >> rx >> ry >> rz >> sx >> sy >> sz >> tDiff >> tNorm >> tMet >> tRough >> tAO >> tHeight) {
        std::vector<std::string> textures = { tDiff, tNorm, tMet, tRough, tAO, tHeight };
        internalSpawn(type, glm::vec3(px, py, pz), glm::vec3(sx, sy, sz), -1, textures);
        if (!gameObjects.empty()) gameObjects.back()->rotation = glm::vec3(rx, ry, rz);
    }

    rebuildPhysics();
    f.close();
    std::cout << "Level loaded successfully from " << filename << std::endl;
}

// sending objects to the gpu to be drawn
void Game::drawSceneToShader(Shader& shader) {
    shader.setFloat("u_Metallic", 0.0f);
    shader.setFloat("u_Roughness", 0.8f);
    shader.setFloat("u_AO", 1.0f);
    player->draw3D(shader);

    for (auto& obj : gameObjects) {
        // don't draw light bulbs in game mode
        if (!m_editorMode && obj->iconPath == "LIGHT") {
            continue;
        }
        shader.setFloat("u_Metallic", obj->pbr_metallic);
        shader.setFloat("u_Roughness", obj->pbr_roughness);
        shader.setFloat("u_AO", obj->pbr_ao);

        obj->draw3D(shader);
    }
}

// helper to get all textures from an object as a list
std::vector<std::string> Game::GetObjectTextures(GameObject* obj) {
    std::vector<std::string> paths(6, "");

    if (!obj || !obj->model || obj->model->GetMeshes().empty()) return paths;

    for (const auto& t : obj->model->GetMeshes()[0].textures) {
        if (t.type == "texture_diffuse")        paths[0] = t.path;
        else if (t.type == "texture_normal")    paths[1] = t.path;
        else if (t.type == "texture_metallic")  paths[2] = t.path;
        else if (t.type == "texture_roughness") paths[3] = t.path;
        else if (t.type == "texture_ao")        paths[4] = t.path;
        else if (t.type == "texture_height")    paths[5] = t.path;
    }
    return paths;
}
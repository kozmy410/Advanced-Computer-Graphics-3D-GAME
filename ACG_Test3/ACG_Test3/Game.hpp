#pragma once
#include <vector>
#include <map> 
#include <memory>
#include <string>
#include <stack>

// including the gui library
#include "dependencies/imgui/imgui.h"
#include "dependencies/imgui/imgui_impl_glfw.h"
#include "dependencies/imgui/imgui_impl_opengl3.h"

// including our own engine classes
#include "Camera.hpp"
#include "Shader.hpp"
#include "Skybox.hpp"
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glfw-3.4/include/GLFW/glfw3.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"

// letting the compiler know these classes exist
class Player;
class GameObject;
class Minimap;

// this struct helps us remember what we did so we can undo it later
struct EditorAction {
    enum Type { ACT_ADD, ACT_DELETE, ACT_TRANSFORM } type;

    std::string objectType;
    glm::vec3 position;
    glm::vec3 oldPosition; // where it was before we moved it
    glm::vec3 scale;
    glm::vec3 rotation;
    int index; // which object in the list was modified

    // storing texture paths so we don't lose them when undoing
    std::vector<std::string> texturePaths;
};


// this defines what the sun looks like at a specific time of day
struct SunKeyframe {
    float time;             // 0.0 to 24.0 (hours)
    glm::vec3 position;     // where the sun is in the sky
    glm::vec3 color;        // the color of the light
    float intensity;        // how bright it is
};

class Game {
public:
    // setting up the game window
    Game(const char* title, int width, int height);
    // cleaning up memory when the game closes
    ~Game();

    // the main loop that runs the game
    void run();
    // handling window resizing
    void onResize(int width, int height);

private:
    // starting up the backend libraries
    void initGLFW();
    void initOpenGL();
    void initImGui();
    // creating the initial world state
    void initGameObjects();

    // saving the current world to a file
    void saveLevel(const std::string& filename);
    // loading a world from a file
    void loadLevel(const std::string& filename);

    // drawing the editor windows
    void renderUI();
    // dropping an item from inventory to the ground
    void dropCurrentItem();

    // editor tools logic
    void executeAction(const EditorAction& action);
    void undo(); // go back one step
    void redo(); // go forward one step

    // helper to get texture info from an object
    std::vector<std::string> GetObjectTextures(GameObject* obj);

    // the heavy lifting for spawning an object
    void internalSpawn(const std::string& type, glm::vec3 pos, glm::vec3 scale, int index, const std::vector<std::string>& texPaths = {});
    // removing an object from the game
    void internalDelete(int index);
    // refreshing collisions after spawning/deleting
    void rebuildPhysics();

    // history of actions for undo/redo
    std::stack<EditorAction> m_undoStack;
    std::stack<EditorAction> m_redoStack;

    // which object is currently clicked in the editor
    GameObject* m_selectedObject = nullptr;
    int m_selectedIndex = -1;

    // handling multiple selected objects
    std::vector<GameObject*> m_selectedObjects;
    std::vector<int> m_selectedIndices;
    // keeping the group shape when dragging multiple items
    std::vector<glm::vec3> m_dragOffsets;

    // helper: checking if an object is in the current selection
    bool isObjectSelected(GameObject* obj) {
        for (auto* s : m_selectedObjects) {
            if (s == obj) return true;
        }
        return false;
    }

    // helper: unselecting everything
    void clearSelection() {
        m_selectedObjects.clear();
        m_selectedIndices.clear();
        m_selectedObject = nullptr;
        m_selectedIndex = -1;
        m_dragOffsets.clear();
    }

    // default material settings (how shiny/rough things look)
    float m_metallic = 0.1f;
    float m_roughness = 0.5f;
    float m_ao = 1.0f;
    float m_lightIntensity = 30.0f;

    // camera settings for the 3rd person view
    float m_cameraYaw = 0.0f;
    float m_cameraPitch = 20.0f;
    float m_cameraDistance = 14.0f;

    // are we playing or building?
    bool m_editorMode = false;
    float m_flySpeed = 2.0f;
    int m_debugMode = 0;

    Camera camera;
    std::unique_ptr<Shader> shader3D;

    // preventing the camera from spinning too fast
    float m_timeSinceMouseInput = 0.0f;

    // is the mouse locked to the screen?
    bool isMouseCaptured = false;

    // time of day system
    float m_dayTime = 12.0f;       // starts at noon
    float m_timeSpeed = 4.0f;      // how fast time passes
    bool m_isTimePaused = false;   // pausing the sun

    // handling object dragging
    bool m_isDragging = false;
    glm::vec3 m_dragStartPos;

    // math to move the sun across the sky
    void updateDayCycle(float deltaTime);

    // sun rendering stuff
    std::unique_ptr<Shader> sunShader;
    std::unique_ptr<GameObject> sun;
    glm::vec3 sunColor = glm::vec3(1.0f, 0.9f, 0.7f);
    float sunIntensity = 20.0f;

    // shadow mapping variables
    unsigned int depthMapFBO;
    unsigned int depthMap;
    const unsigned int SHADOW_WIDTH = 4096, SHADOW_HEIGHT = 4096;
    std::unique_ptr<Shader> depthShader;

    // drawing the objects for the shadow pass
    void drawSceneToShader(Shader& shader);

    bool m_flashLightOn = false;

    // the skybox (background environment)
    std::unique_ptr<Skybox> m_skybox;

    // main loop functions
    void handleInput();
    void update(float deltaTime);
    void render(float deltaTime);
    void updateWindowTitle(float deltaTime);

    // story and event system
    void handleStoryEvent(int id);
    void initShadowMap();
    void initTriggers();

    // ui drawing functions
    void drawInventoryUI();
    void drawHierarchy();
    void drawInspector();
    void drawGlobalSettings();

    // window data
    GLFWwindow* window;
    int windowWidth, windowHeight, windowPosX, windowPosY;
    bool isFullscreen;

    // story flags
    bool m_storyInitialized;
    std::map<int, bool> m_processedEvents;

    // the player and the world objects
    std::unique_ptr<Player> player;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::unique_ptr<Minimap> minimap;
};
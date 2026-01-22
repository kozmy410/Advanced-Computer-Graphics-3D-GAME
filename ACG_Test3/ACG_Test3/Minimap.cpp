#include "Minimap.hpp"
#include "GameConstants.hpp" 
#include "Shader.hpp"
#include "GameObject.hpp"
#include "Player.hpp"
#include "Texture.hpp"
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtc/matrix_transform.hpp"

// simple math helper to move a value toward a target smoothly over time
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

Minimap::Minimap(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight),
    m_shader(nullptr), m_isMaximized(false)
{
    // setting how close the camera starts to the ground
    m_currentZoom = 20.0f;
    m_targetZoom = 20.0f;
    m_zoomSpeed = 4.0f;

    // setting up the special shaders used for drawing the 2d overlay
    m_shader = new Shader("shaders/minimap.vert", "shaders/minimap.frag");

    // loading the large image that represents the entire world map background
    m_mapTexture = new Texture("resources/map.bmp");

    setupIconGeometry(); // creates the square for icons (player/npcs)
    setupMapGeometry();  // creates the giant square for the background
}

Minimap::~Minimap() {
    delete m_shader;
    delete m_mapTexture;
    // cleaning up any icons we stored in memory
    for (auto const& [key, val] : m_textureCache) {
        delete val;
    }
    // clearing opengl buffers to prevent memory leaks
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
    glDeleteVertexArrays(1, &m_mapVao);
}

void Minimap::setTargetZoom(float target) {
    m_targetZoom = target;
}

void Minimap::toggleMaximized() {
    m_isMaximized = !m_isMaximized; // switches between corner view and full-screen view
}

void Minimap::onWindowResize(int newWidth, int newHeight) {
    m_screenWidth = newWidth;
    m_screenHeight = newHeight;
}

// this helper ensures we don't load the same icon image twice
Texture* Minimap::getTexture(const std::string& path) {
    if (m_textureCache.find(path) != m_textureCache.end()) {
        return m_textureCache[path];
    }
    Texture* newTexture = new Texture(path);
    m_textureCache[path] = newTexture;
    return newTexture;
}

void Minimap::setupIconGeometry() {
    // defining a small 1x1 flat square centered at 0,0
    float vertices[] = {
        -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
         0.5f, 0.0f, -0.5f,   1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,   1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,   0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    m_indexCount = 6;

    // standard opengl buffer setup
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ibo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Minimap::setupMapGeometry() {
    // creating a giant square that covers the entire playable area
    float vertices[] = {
        MAP_BOUNDS_MIN, 0.0f, MAP_BOUNDS_MIN,  0.0f, 0.0f,
        MAP_BOUNDS_MAX, 0.0f, MAP_BOUNDS_MIN,  1.0f, 0.0f,
        MAP_BOUNDS_MAX, 0.0f, MAP_BOUNDS_MAX,  1.0f, 1.0f,
        MAP_BOUNDS_MIN, 0.0f, MAP_BOUNDS_MAX,  0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

    glGenVertexArrays(1, &m_mapVao);
    glGenBuffers(1, &m_mapVbo);
    glGenBuffers(1, &m_mapIbo);

    glBindVertexArray(m_mapVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_mapVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_mapIbo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Minimap::draw(const Player* player, const std::vector<GameObject*>& gameObjects, float deltaTime) {
    // 1. we disable depth testing so the map draws on top of the world, not inside it
    glDisable(GL_DEPTH_TEST);

    float viewportWidth, viewportHeight;
    if (m_isMaximized) {
        // use the whole screen
        viewportWidth = static_cast<float>(m_screenWidth);
        viewportHeight = static_cast<float>(m_screenHeight);
        glViewport(0, 0, m_screenWidth, m_screenHeight);
    }
    else {
        // draw a small box in the corner
        int minimapSize = 250;
        int padding = 15;
        viewportWidth = static_cast<float>(minimapSize);
        viewportHeight = static_cast<float>(minimapSize);

        glViewport(padding, padding, minimapSize, minimapSize);

        // "scissor" prevents opengl from drawing anything outside this specific square
        glEnable(GL_SCISSOR_TEST);
        glScissor(padding, padding, minimapSize, minimapSize);
    }
    float aspectRatio = viewportWidth / viewportHeight;

    // clear the minimap background color to dark grey
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_shader->use();
    glUniform1i(glGetUniformLocation(m_shader->ID, "u_Texture"), 0);

    // --- camera logic ---
    // smoothing the zoom movement so it doesn't snap instantly
    m_currentZoom = lerp(m_currentZoom, m_targetZoom, m_zoomSpeed * deltaTime);

    float halfViewWidth, halfViewHeight;
    if (aspectRatio >= 1.0f) {
        halfViewHeight = m_currentZoom;
        halfViewWidth = m_currentZoom * aspectRatio;
    }
    else {
        halfViewWidth = m_currentZoom;
        halfViewHeight = m_currentZoom / aspectRatio;
    }

    // the minimap camera always stays directly above the player
    glm::vec3 cameraPos = player->position;

    // clamping prevents the camera from showing the "void" outside the map edges
    float minCamX = MAP_BOUNDS_MIN + halfViewWidth;
    float maxCamX = MAP_BOUNDS_MAX - halfViewWidth;
    float minCamZ = MAP_BOUNDS_MIN + halfViewHeight;
    float maxCamZ = MAP_BOUNDS_MAX - halfViewHeight;

    if (halfViewWidth < (MAP_BOUNDS_MAX - MAP_BOUNDS_MIN) / 2.0f) {
        cameraPos.x = glm::clamp(cameraPos.x, minCamX, maxCamX);
        cameraPos.z = glm::clamp(cameraPos.z, minCamZ, maxCamZ);
    }

    // setting up the view from high above looking straight down
    glm::mat4 view = glm::lookAt(
        glm::vec3(cameraPos.x, 50.0f, cameraPos.z),
        glm::vec3(cameraPos.x, 0.0f, cameraPos.z),
        glm::vec3(0.0f, 0.0f, -1.0f) // z is 'up' in our top-down view
    );
    // orthographic projection removes perspective (objects far away don't look smaller)
    glm::mat4 projection = glm::ortho(-halfViewWidth, halfViewWidth, -halfViewHeight, halfViewHeight, -1.0f, 100.0f);

    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);

    // draw pass 1: map background
    m_mapTexture->bind(0);
    m_shader->setVec3("objectColor", glm::vec3(1.0f));
    m_shader->setMat4("model", glm::mat4(1.0f));
    glBindVertexArray(m_mapVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // draw pass 2: player icon
    glBindVertexArray(m_vao);
    {
        Texture* playerTex = getTexture(player->iconPath);
        if (playerTex) playerTex->bind(0);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, player->position);
        model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f)); // sizing the player icon

        m_shader->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    }

    // draw pass 3: all other object icons (npcs, items, etc)
    for (const auto& obj : gameObjects) {
        if (obj->iconPath.empty()) continue; // don't draw things that don't have icons

        Texture* objectTexture = getTexture(obj->iconPath);
        if (objectTexture) {
            objectTexture->bind(0);
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, obj->position);
            model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f));

            m_shader->setMat4("model", model);
            glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
        }
    }

    // cleaning up state so the main 3d world renders correctly after this
    if (!m_isMaximized) {
        glDisable(GL_SCISSOR_TEST);
    }
    glViewport(0, 0, m_screenWidth, m_screenHeight);
    glEnable(GL_DEPTH_TEST);
}
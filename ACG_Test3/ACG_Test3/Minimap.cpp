#include "Minimap.hpp"
#include "GameConstants.hpp" 
#include "Shader.hpp"
#include "GameObject.hpp"
#include "Player.hpp"
#include "Texture.hpp"
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtc/matrix_transform.hpp"



// A simple Linear Interpolation (Lerp) helper function for smooth animation.
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}


Minimap::Minimap(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight),
    m_shader(nullptr), m_isMaximized(false) {

    // --- SET INITIAL ZOOM STATE ---
    m_currentZoom = 20.0f;
    m_targetZoom = 20.0f;
    m_zoomSpeed = 4.0f; // Higher is faster

    m_shader = new Shader("shaders/minimap.vert", "shaders/minimap.frag");
    m_mapTexture = new Texture("resources/map.bmp");

    setupIconGeometry();
    setupMapGeometry();
}

Minimap::~Minimap() {
    delete m_shader;
    delete m_mapTexture;
    for (auto const& [key, val] : m_textureCache) {
        delete val;
    }
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
    glDeleteVertexArrays(1, &m_mapVao);
    glDeleteBuffers(1, &m_mapVbo);
    glDeleteBuffers(1, &m_mapIbo);
}

// This is a new public method for external control
void Minimap::setTargetZoom(float target) {
    m_targetZoom = target;
}

Texture* Minimap::getTexture(const std::string& path) {
    if (m_textureCache.find(path) != m_textureCache.end()) {
        return m_textureCache[path];
    }
    Texture* newTexture = new Texture(path);
    m_textureCache[path] = newTexture;
    return newTexture;
}

void Minimap::toggleMaximized() { m_isMaximized = !m_isMaximized; }
void Minimap::onWindowResize(int newWidth, int newHeight) { m_screenWidth = newWidth; m_screenHeight = newHeight; }


void Minimap::setupIconGeometry() {
    float vertices[] = {
        -0.5f, 0.0f, -0.5f,   0.0f, 0.0f,
         0.5f, 0.0f, -0.5f,   1.0f, 0.0f,
         0.5f, 0.0f,  0.5f,   1.0f, 1.0f,
        -0.5f, 0.0f,  0.5f,   0.0f, 1.0f
    };
    unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };
    m_indexCount = 6;
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
    glBindVertexArray(0);
}

void Minimap::setupMapGeometry() {
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
    glBindVertexArray(0);
}


void Minimap::draw(const Player* player, const std::vector<GameObject*>& gameObjects, float deltaTime) {
    // 1. Set OpenGL state for 2D UI rendering
    glDisable(GL_DEPTH_TEST);

    // 2. Set up the viewport and calculate its aspect ratio
    float viewportWidth, viewportHeight;
    if (m_isMaximized) {
        viewportWidth = m_screenWidth;
        viewportHeight = m_screenHeight;
        glViewport(0, 0, static_cast<int>(viewportWidth), static_cast<int>(viewportHeight));
    }
    else {
        int minimapSize = 200;
        int padding = 10;
        viewportWidth = static_cast<float>(minimapSize);
        viewportHeight = static_cast<float>(minimapSize);
        glViewport(padding, padding, minimapSize, minimapSize);
        glEnable(GL_SCISSOR_TEST);
        glScissor(padding, padding, minimapSize, minimapSize);
    }
    float aspectRatio = viewportWidth / viewportHeight;

    // 3. Clear the minimap background
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 4. Prepare the shader and common uniforms
    m_shader->use();
    glUniform1i(glGetUniformLocation(m_shader->ID, "u_Texture"), 0);

    // 5. --- SMOOTH ZOOM AND CAMERA LOGIC ---

    // Animate the current zoom level towards the target zoom set by main.cpp
    m_currentZoom = lerp(m_currentZoom, m_targetZoom, m_zoomSpeed * deltaTime);

    // All rendering will use this single, animated zoom value.
    float zoomForRender = m_currentZoom;

    // Calculate the dimensions of our camera's view in world space
    float halfViewWidth, halfViewHeight;
    if (aspectRatio >= 1.0f) { // Wider than tall
        halfViewHeight = zoomForRender;
        halfViewWidth = zoomForRender * aspectRatio;
    }
    else { // Taller than wide
        halfViewWidth = zoomForRender;
        halfViewHeight = zoomForRender / aspectRatio;
    }

    // Clamp the camera position so it never shows area outside the map
    glm::vec3 cameraPos = player->position;
    float minCamX = MAP_BOUNDS_MIN + halfViewWidth;
    float maxCamX = MAP_BOUNDS_MAX - halfViewWidth;
    float minCamZ = MAP_BOUNDS_MIN + halfViewHeight;
    float maxCamZ = MAP_BOUNDS_MAX - halfViewHeight;
    cameraPos.x = glm::clamp(cameraPos.x, minCamX, maxCamX);
    cameraPos.z = glm::clamp(cameraPos.z, minCamZ, maxCamZ);

    // Create the final view and projection matrices
    glm::mat4 view = glm::lookAt(
        glm::vec3(cameraPos.x, 50.0f, cameraPos.z),
        glm::vec3(cameraPos.x, 0.0f, cameraPos.z),
        glm::vec3(0.0f, 0.0f, -1.0f)
    );
    glm::mat4 projection = glm::ortho(-halfViewWidth, halfViewWidth, -halfViewHeight, halfViewHeight, -1.0f, 100.0f);

    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);

    // 6. --- DRAW PASS 1: THE MAP BACKGROUND ---
    m_mapTexture->bind(0);
    m_shader->setVec3("objectColor", glm::vec3(1.0f)); // White tint (no color change)
    m_shader->setMat4("model", glm::mat4(1.0f)); // Map is already world-sized
    glBindVertexArray(m_mapVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // 7. --- DRAW PASS 2: THE ICONS ---
    glBindVertexArray(m_vao);
    for (const auto& obj : gameObjects) {
        Texture* objectTexture = getTexture(obj->texturePath);
        if (objectTexture) {
            objectTexture->bind(0);
        }

        m_shader->setVec3("objectColor", glm::vec3(1.0f)); // White tint for icons

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj->position);
        model = glm::scale(model, obj->scale); // Apply the object's unique scale

        m_shader->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    }

    // 8. --- RESET OPENGL STATE ---
    if (!m_isMaximized) {
        glDisable(GL_SCISSOR_TEST);
    }
    glViewport(0, 0, m_screenWidth, m_screenHeight); // Reset viewport to full screen
    glEnable(GL_DEPTH_TEST); // Re-enable depth test for the 3D scene
}
#include "Minimap.hpp"
#include "GameConstants.hpp" 
#include "Shader.hpp"
#include "GameObject.hpp"
#include "Player.hpp"
#include "Texture.hpp"

#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glm-1.0.2/glm/gtc/matrix_transform.hpp"

// Helper for smooth zooming
float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

Minimap::Minimap(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth), m_screenHeight(screenHeight),
    m_shader(nullptr), m_isMaximized(false)
{
    // --- SET INITIAL ZOOM STATE ---
    m_currentZoom = 20.0f;
    m_targetZoom = 20.0f;
    m_zoomSpeed = 4.0f;

    // Load Minimap Specific Shaders
    m_shader = new Shader("shaders/minimap.vert", "shaders/minimap.frag");

    // Load the Map Background Image
    m_mapTexture = new Texture("resources/map.bmp");

    setupIconGeometry();
    setupMapGeometry();
}

Minimap::~Minimap() {
    delete m_shader;
    delete m_mapTexture;
    // Clean up cached icon textures
    for (auto const& [key, val] : m_textureCache) {
        delete val;
    }
    // Clean up OpenGL buffers
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);
    glDeleteVertexArrays(1, &m_mapVao);
    glDeleteBuffers(1, &m_mapVbo);
    glDeleteBuffers(1, &m_mapIbo);
}

void Minimap::setTargetZoom(float target) {
    m_targetZoom = target;
}

void Minimap::toggleMaximized() {
    m_isMaximized = !m_isMaximized;
}

void Minimap::onWindowResize(int newWidth, int newHeight) {
    m_screenWidth = newWidth;
    m_screenHeight = newHeight;
}

Texture* Minimap::getTexture(const std::string& path) {
    // Check if texture is already loaded
    if (m_textureCache.find(path) != m_textureCache.end()) {
        return m_textureCache[path];
    }
    // If not, load it and cache it
    Texture* newTexture = new Texture(path);
    m_textureCache[path] = newTexture;
    return newTexture;
}

void Minimap::setupIconGeometry() {
    // A simple 1x1 flat square for icons centered at 0,0
    float vertices[] = {
        // Pos(X, Y, Z)    TexCoord(U, V)
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

    // Attrib 0: Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Attrib 1: TexCoords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Minimap::setupMapGeometry() {
    // A giant square covering the entire world bounds
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
    // 1. DISABLE DEPTH TEST (So 2D map draws on top of 3D world)
    glDisable(GL_DEPTH_TEST);

    // 2. Set Viewport (Mini vs Maxi)
    float viewportWidth, viewportHeight;
    if (m_isMaximized) {
        viewportWidth = static_cast<float>(m_screenWidth);
        viewportHeight = static_cast<float>(m_screenHeight);
        glViewport(0, 0, m_screenWidth, m_screenHeight);
    }
    else {
        int minimapSize = 250; // Increased size slightly for better visibility
        int padding = 15;

        viewportWidth = static_cast<float>(minimapSize);
        viewportHeight = static_cast<float>(minimapSize);

        // Place in top-left or bottom-left depending on your preference
        glViewport(padding, padding, minimapSize, minimapSize);

        // Clip rendering to this box
        glEnable(GL_SCISSOR_TEST);
        glScissor(padding, padding, minimapSize, minimapSize);
    }
    float aspectRatio = viewportWidth / viewportHeight;

    // 3. Clear Background (Dark Grey)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 4. Activate Shader
    m_shader->use();
    glUniform1i(glGetUniformLocation(m_shader->ID, "u_Texture"), 0);

    // 5. --- CAMERA LOGIC ---
    m_currentZoom = lerp(m_currentZoom, m_targetZoom, m_zoomSpeed * deltaTime);

    // Calculate View/Projection
    float halfViewWidth, halfViewHeight;
    if (aspectRatio >= 1.0f) {
        halfViewHeight = m_currentZoom;
        halfViewWidth = m_currentZoom * aspectRatio;
    }
    else {
        halfViewWidth = m_currentZoom;
        halfViewHeight = m_currentZoom / aspectRatio;
    }

    // Camera center follows player
    glm::vec3 cameraPos = player->position;

    // Clamp Camera so we don't see outside the map bounds
    float minCamX = MAP_BOUNDS_MIN + halfViewWidth;
    float maxCamX = MAP_BOUNDS_MAX - halfViewWidth;
    float minCamZ = MAP_BOUNDS_MIN + halfViewHeight;
    float maxCamZ = MAP_BOUNDS_MAX - halfViewHeight;

    // Only clamp if the zoom is smaller than the map itself
    if (halfViewWidth < (MAP_BOUNDS_MAX - MAP_BOUNDS_MIN) / 2.0f) {
        cameraPos.x = glm::clamp(cameraPos.x, minCamX, maxCamX);
        cameraPos.z = glm::clamp(cameraPos.z, minCamZ, maxCamZ);
    }

    // Top-Down Orthographic View
    glm::mat4 view = glm::lookAt(
        glm::vec3(cameraPos.x, 50.0f, cameraPos.z), // Eye
        glm::vec3(cameraPos.x, 0.0f, cameraPos.z),  // Center
        glm::vec3(0.0f, 0.0f, -1.0f)                // Up (Z-axis is "up" in 2D top-down logic here)
    );
    glm::mat4 projection = glm::ortho(-halfViewWidth, halfViewWidth, -halfViewHeight, halfViewHeight, -1.0f, 100.0f);

    m_shader->setMat4("view", view);
    m_shader->setMat4("projection", projection);

    // 6. --- DRAW PASS 1: MAP BACKGROUND ---
    m_mapTexture->bind(0);
    m_shader->setVec3("objectColor", glm::vec3(1.0f)); // White tint (original color)
    m_shader->setMat4("model", glm::mat4(1.0f)); // Identity (Map is already world size)
    glBindVertexArray(m_mapVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

    // 7. --- DRAW PASS 2: PLAYER ICON ---
    glBindVertexArray(m_vao); // Switch to Icon geometry
    {
        // Use 'iconPath' from the 3D refactor
        Texture* playerTex = getTexture(player->iconPath);
        if (playerTex) playerTex->bind(0);

        m_shader->setVec3("objectColor", glm::vec3(1.0f)); // No tint (or use Green if you want)

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, player->position);
        // Player scale on map: 3.0f usually looks good
        model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f));

        m_shader->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    }

    // 8. --- DRAW PASS 3: NPC ICONS ---
    for (const auto& obj : gameObjects) {
        // Use 'iconPath' here too
        Texture* objectTexture = getTexture(obj->iconPath);
        if (objectTexture) objectTexture->bind(0);

        m_shader->setVec3("objectColor", glm::vec3(1.0f));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, obj->position);
        model = glm::scale(model, glm::vec3(3.0f, 1.0f, 3.0f)); // Fixed size for map icons

        m_shader->setMat4("model", model);
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, nullptr);
    }

    // 9. --- CLEANUP ---
    if (!m_isMaximized) {
        glDisable(GL_SCISSOR_TEST);
    }
    // Restore Viewport for the next frame's 3D render
    glViewport(0, 0, m_screenWidth, m_screenHeight);

    // Re-enable Depth Test for 3D
    glEnable(GL_DEPTH_TEST);
}
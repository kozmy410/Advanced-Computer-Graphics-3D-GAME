#include "Primitives.hpp"
#include "GameConstants.hpp"

const float PI = 3.14159265359f;

// --- 1. PLANE ---
// creating a flat 1x1 floor or wall
std::unique_ptr<Model> Primitives::CreatePlane(const std::string& texturePath, float uvScale) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // defining the 4 corners of a flat square sitting on the xz-plane
    // bottom left
    vertices.push_back({
        glm::vec3(-0.5f, 0.0f,  0.5f),
        glm::vec3(0.0f, 1.0f, 0.0f),  // pointing straight up
        glm::vec2(0.0f, uvScale),      // uv mapping
        glm::vec3(1.0f, 0.0f, 0.0f),  // side vector for bump mapping
        glm::vec3(0.0f, 0.0f, -1.0f)
        });

    // bottom right
    vertices.push_back({
        glm::vec3(0.5f, 0.0f,  0.5f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec2(uvScale, uvScale),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)
        });

    // top right
    vertices.push_back({
        glm::vec3(0.5f, 0.0f, -0.5f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec2(uvScale, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)
        });

    // top left
    vertices.push_back({
        glm::vec3(-0.5f, 0.0f, -0.5f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec2(0.0f, 0.0f),
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)
        });

    // telling opengl to make two triangles out of these four points
    indices = { 0, 1, 2, 2, 3, 0 };

    return std::make_unique<Model>(vertices, indices, texturePath);
}

// --- 2. CUBE ---
// building a 6-sided box
std::unique_ptr<Model> Primitives::CreateCube(const std::string& texturePath) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // a little helper function to build one side of the box at a time
    auto addFace = [&](glm::vec3 n, glm::vec3 t, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4) {
        int base = (int)vertices.size();
        vertices.push_back({ p1, n, glm::vec2(0.0f, 1.0f), t });
        vertices.push_back({ p2, n, glm::vec2(1.0f, 1.0f), t });
        vertices.push_back({ p3, n, glm::vec2(1.0f, 0.0f), t });
        vertices.push_back({ p4, n, glm::vec2(0.0f, 0.0f), t });
        // connecting the four points into two triangles
        indices.push_back(base); indices.push_back(base + 1); indices.push_back(base + 2);
        indices.push_back(base + 2); indices.push_back(base + 3); indices.push_back(base);
        };

    // adding each of the 6 sides
    addFace({ 0,0,1 }, { 1,0,0 }, { -0.5,-0.5,0.5 }, { 0.5,-0.5,0.5 }, { 0.5,0.5,0.5 }, { -0.5,0.5,0.5 }); // front
    addFace({ 0,0,-1 }, { -1,0,0 }, { 0.5,-0.5,-0.5 }, { -0.5,-0.5,-0.5 }, { -0.5,0.5,-0.5 }, { 0.5,0.5,-0.5 }); // back
    addFace({ 0,1,0 }, { 1,0,0 }, { -0.5,0.5,0.5 }, { 0.5,0.5,0.5 }, { 0.5,0.5,-0.5 }, { -0.5,0.5,-0.5 }); // top
    addFace({ 0,-1,0 }, { 1,0,0 }, { -0.5,-0.5,-0.5 }, { 0.5,-0.5,-0.5 }, { 0.5,-0.5,0.5 }, { -0.5,-0.5,0.5 }); // bottom
    addFace({ 1,0,0 }, { 0,0,-1 }, { 0.5,-0.5,0.5 }, { 0.5,-0.5,-0.5 }, { 0.5,0.5,-0.5 }, { 0.5,0.5,0.5 }); // right
    addFace({ -1,0,0 }, { 0,0,1 }, { -0.5,-0.5,-0.5 }, { -0.5,-0.5,0.5 }, { -0.5,0.5,0.5 }, { -0.5,0.5,-0.5 }); // left

    return std::make_unique<Model>(vertices, indices, texturePath);
}

// --- 3. SPHERE ---
// uses math to wrap a grid of points into a ball
std::unique_ptr<Model> Primitives::CreateSphere(const std::string& texturePath, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    // looping through latitude and longitude to create the shell of the ball
    for (int y = 0; y <= segments; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float xSegment = (float)x / (float)segments;
            float ySegment = (float)y / (float)segments;

            // trigonometry to find the xyz position on the surface of the ball
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            Vertex v;
            v.Position = glm::vec3(xPos, yPos, zPos) * 0.5f;
            v.Normal = glm::normalize(v.Position); // on a sphere, normal is just the position direction
            v.TexCoords = glm::vec2(xSegment, ySegment);
            v.Tangent = glm::cross(glm::vec3(0, 1, 0), v.Normal);
            vertices.push_back(v);
        }
    }

    // sewing the grid of points together into triangles
    for (int y = 0; y < segments; ++y) {
        for (int x = 0; x < segments; ++x) {
            indices.push_back((y + 1) * (segments + 1) + x);
            indices.push_back(y * (segments + 1) + x);
            indices.push_back(y * (segments + 1) + x + 1);

            indices.push_back((y + 1) * (segments + 1) + x);
            indices.push_back(y * (segments + 1) + x + 1);
            indices.push_back((y + 1) * (segments + 1) + x + 1);
        }
    }

    return std::make_unique<Model>(vertices, indices, texturePath);
}

// --- 4. CYLINDER ---
// creates a round pillar or tube
std::unique_ptr<Model> Primitives::CreateCylinder(const std::string& texturePath, int segments) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    float step = 2.0f * PI / (float)segments;

    // walking in a circle and creating points at the top and bottom for each step
    for (int i = 0; i <= segments; i++) {
        float angle = i * step;
        float x = cos(angle) * 0.5f;
        float z = sin(angle) * 0.5f;

        glm::vec3 n = glm::normalize(glm::vec3(x, 0, z));

        // creating a point at the top of the circle
        Vertex vTop;
        vTop.Position = glm::vec3(x, 0.5f, z);
        vTop.Normal = n;
        vTop.TexCoords = glm::vec2((float)i / segments, 1.0f);
        vertices.push_back(vTop);

        // creating a point directly below it at the bottom
        Vertex vBot;
        vBot.Position = glm::vec3(x, -0.5f, z);
        vBot.Normal = n;
        vBot.TexCoords = glm::vec2((float)i / segments, 0.0f);
        vertices.push_back(vBot);
    }

    // connecting the top and bottom circles with a "wall" of triangles
    for (int i = 0; i < segments; i++) {
        int top1 = i * 2;
        int bot1 = i * 2 + 1;
        int top2 = (i * 2 + 2);
        int bot2 = (i * 2 + 3);

        indices.push_back(top1); indices.push_back(bot1); indices.push_back(top2);
        indices.push_back(bot1); indices.push_back(bot2); indices.push_back(top2);
    }

    return std::make_unique<Model>(vertices, indices, texturePath);
}
#pragma once
#include <vector>
#include <memory>
#include <cmath>
#include "Model.hpp"
#include "dependencies/glm-1.0.2/glm/glm.hpp"

// this class is a factory that generates basic 3d shapes from code
// so we don't need to load external .obj files for simple things like floors or boxes
class Primitives {
public:
    // creates a flat 1x1 surface. 'uvScale' allows us to tile the texture (e.g. repeat a brick pattern)
    static std::unique_ptr<Model> CreatePlane(const std::string& texturePath = "", float uvScale = 1.0f);

    // creates a standard cube. useful for crates or stretching to make walls
    static std::unique_ptr<Model> CreateCube(const std::string& texturePath = "");

    // creates a round ball. 'segments' determines how smooth the curve is (higher = smoother but more expensive)
    static std::unique_ptr<Model> CreateSphere(const std::string& texturePath = "", int segments = 32);

    // creates a pillar shape
    static std::unique_ptr<Model> CreateCylinder(const std::string& texturePath = "", int segments = 32);

private:
    // internal math helper to calculate lighting vectors (tangents) so normal maps work correctly on these shapes
    static void CalculateTangents(std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
};
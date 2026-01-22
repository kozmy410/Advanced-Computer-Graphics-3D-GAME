#pragma once
#include <string>
#include <vector>
#include "dependencies/glm-1.0.2/glm/glm.hpp"
#include "dependencies/glew-2.2.0/include/GL/glew.h"
#include "Shader.hpp"

// defining what a single point in 3d space looks like
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

// structure to hold info about an image we loaded
struct Texture3D {
    unsigned int id = 0;
    std::string type; // e.g., "texture_diffuse" or "texture_normal"
    std::string path;
};

class Mesh {
public:
    // the raw data for this part of the model
    std::vector<Vertex>       vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture3D>    textures;

    // constructor: copying the data and sending it to the gpu
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture3D> textures) {
        this->vertices = vertices;
        this->indices = indices;
        this->textures = textures;
        setupMesh();
    }

    // the main function to render this mesh to the screen
    void Draw(Shader& shader) {
        // keeping track of how many textures of each type we have
        unsigned int diffuseNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;
        unsigned int metallicNr = 1;
        unsigned int roughnessNr = 1;
        unsigned int aoNr = 1;

        // flags to tell the shader what data to expect
        bool hasDiffuse = false;
        bool hasNormal = false;
        bool hasDisp = false;
        bool hasMetallic = false;
        bool hasRoughness = false;
        bool hasAO = false;

        // looping through all the textures attached to this mesh
        for (unsigned int i = 0; i < textures.size(); i++) {
            // activating the proper texture unit on the gpu
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = textures[i].type;

            // checking the type and incrementing the counter
            if (name == "texture_diffuse") { number = std::to_string(diffuseNr++); hasDiffuse = true; }
            else if (name == "texture_normal") { number = std::to_string(normalNr++); hasNormal = true; }
            else if (name == "texture_height") { number = std::to_string(heightNr++); hasDisp = true; }
            // checking for pbr specific textures
            else if (name == "texture_metallic") { number = std::to_string(metallicNr++); hasMetallic = true; }
            else if (name == "texture_roughness") { number = std::to_string(roughnessNr++); hasRoughness = true; }
            else if (name == "texture_ao") { number = std::to_string(aoNr++); hasAO = true; }

            // telling the shader which texture unit corresponds to this name (e.g., texture_diffuse1)
            shader.setInt((name + number).c_str(), i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // telling the shader which maps were actually found
        shader.setBool("hasTexture", hasDiffuse);
        shader.setBool("hasNormalMap", hasNormal);
        shader.setBool("hasDispMap", hasDisp);
        shader.setBool("hasMetallicMap", hasMetallic);
        shader.setBool("hasRoughnessMap", hasRoughness);
        shader.setBool("hasAOMap", hasAO);

        // binding the mesh data and drawing the triangles
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);

        // cleaning up
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // opengl buffer ids
    unsigned int VAO, VBO, EBO;

    // sending the vertex data to the graphics card
    void setupMesh() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        // uploading the list of vertices
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // uploading the list of indices (which vertices make a triangle)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // telling opengl how to interpret the vertex data
        // 1. position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // 2. normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // 3. texture coordinates
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // 4. tangent vectors (for normal mapping)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

        glBindVertexArray(0);
    }
};
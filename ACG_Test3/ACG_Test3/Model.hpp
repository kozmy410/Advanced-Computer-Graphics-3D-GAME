#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <map>

// standard libraries needed for lists, text and maps

// including the assimp library, which does the heavy lifting of reading 3d model files
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.hpp"
#include "Shader.hpp"

// including stb_image to handle loading image files (like jpg or png)
#include "dependencies/stb/stb_image.h"

class Model {
public:
    // --- CONSTRUCTOR 1: Load from File (Assimp) ---
    // this is the main way we create a model. we just give it a file path
    // and it triggers the loading process immediately.
    Model(std::string const& path, bool gamma = false) : gammaCorrection(gamma) {
        loadModel(path);
    }

    // --- CONSTRUCTOR 2: Load from Raw Data (Primitives) ---
    // this allows us to create a 'model' manually by passing in a list of points (vertices)
    // and the order to connect them (indices), useful for simple shapes like a box or floor.
    Model(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::string& texturePath)
        : gammaCorrection(false)
    {
        std::vector<Texture3D> textures;

        // if we provided a path for a texture image, try to load it
        if (!texturePath.empty()) {
            // using the helper function to load the image file
            unsigned int id = TextureFromFile(texturePath.c_str(), ".", false);

            // if the texture loaded successfully (id isn't 0)
            if (id != 0) {
                Texture3D tex;
                tex.id = id;
                tex.type = "texture_diffuse"; // assuming it's a standard color texture
                tex.path = texturePath;
                textures.push_back(tex);
            }
            else {
                // just letting us know if the image file wasn't found
                std::cout << "Warning: Primitive texture failed [" << texturePath << "]. Defaulting to White." << std::endl;
            }
        }
        // creating a mesh object from the raw data we just processed
        meshes.push_back(Mesh(vertices, indices, textures));
    }

    // this function is called inside the game loop to render the model
    void Draw(Shader& shader) {
        // a model is made of many meshes (parts), so we loop through all of them
        // and tell each one to draw itself
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
    }

    // helper to get the list of meshes, in case we need to modify them from outside
    std::vector<Mesh>& GetMeshes() { return meshes; }

    // --- NEW: Public Static Helper ---
    // this function reads an image file from the disk and turns it into an opengl texture.
    // it's static so we can use it without needing a model instance.
    static unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false) {
        std::string filename = std::string(path);

        // checking if we need to combine the folder path with the filename
        if (directory != ".") {
            filename = directory + '/' + filename;
        }

        int width, height, nrComponents;
        // using stb_image to load the actual pixel data from the file
        unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

        if (data) {
            unsigned int textureID;
            glGenTextures(1, &textureID); // asking opengl to give us an id for a new texture

            // figuring out if the image has an alpha channel (transparency) or just colors
            GLenum format = GL_RGB;
            if (nrComponents == 1) format = GL_RED;
            else if (nrComponents == 3) format = GL_RGB;
            else if (nrComponents == 4) format = GL_RGBA;

            // binding the texture so future commands affect this specific one
            glBindTexture(GL_TEXTURE_2D, textureID);
            // sending the pixel data to the graphics card
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D); // automatically creating smaller versions for distance rendering

            // setting how the texture wraps if the coordinates go outside 0 to 1
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            // setting how the texture looks when we zoom in or out (filtering)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // freeing the cpu memory since the gpu has the data now
            stbi_image_free(data);
            return textureID;
        }
        else {
            // letting us know if the file path was wrong or the image was broken
            std::cout << "Texture failed to load at path: " << filename << std::endl;
            stbi_image_free(data);
            return 0;
        }
    }

private:
    std::vector<Mesh> meshes; // a list of all the individual parts of the model
    std::string directory; // where the model file is located on the computer
    bool gammaCorrection;
    std::vector<Texture3D> textures_loaded; // keeping track of loaded textures to avoid duplicates

    // this is the starting point for loading a model via assimp
    void loadModel(std::string const& path) {
        Assimp::Importer importer;
        // reading the file and setting flags to clean up the data (like flipping uvs for opengl)
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        // checking if assimp failed to read the file
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            return;
        }
        // saving the folder path so we can find texture files later
        directory = path.substr(0, path.find_last_of('/'));

        // starting the recursive processing of the nodes
        processNode(scene->mRootNode, scene);
    }

    // this function looks at a node, processes its meshes, and then checks its children
    void processNode(aiNode* node, const aiScene* scene) {
        // processing all the meshes located at this specific node
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // checking if this node has children nodes and processing them too (recursion)
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    // this converts assimp's mesh data into our custom mesh class
    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture3D> textures;

        // looping through every vertex (point) in the mesh
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex vertex;
            // getting the 3d position
            vertex.Position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

            // getting the normal vector (used for lighting)
            if (mesh->HasNormals())
                vertex.Normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

            // handling texture coordinates (uv maps)
            if (mesh->mTextureCoords[0]) {
                vertex.TexCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
                // tangents are needed for advanced lighting maps (normal mapping)
                if (mesh->HasTangentsAndBitangents()) {
                    vertex.Tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
                    vertex.Bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
                }
            }
            else vertex.TexCoords = glm::vec2(0.0f, 0.0f); // default to zero if no texture coords exist

            vertices.push_back(vertex);
        }

        // gathering the indices (which points connect to make triangles)
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // processing materials (textures)
        if (mesh->mMaterialIndex >= 0) {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

            // loading diffuse maps (base color textures)
            std::vector<Texture3D> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

            // loading normal maps (bump details)
            std::vector<Texture3D> normalMaps = loadMaterialTextures(material, aiTextureType_NORMALS, "texture_normal");
            textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        }
        // returning the fully constructed mesh
        return Mesh(vertices, indices, textures);
    }

    // checks all textures in a material and loads them
    std::vector<Texture3D> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
        std::vector<Texture3D> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
            aiString str;
            mat->GetTexture(type, i, &str); // getting the filename of the texture

            bool skip = false;
            // checking if we already loaded this texture before to save memory
            for (unsigned int j = 0; j < textures_loaded.size(); j++) {
                if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // we found it, so we don't need to reload it from disk
                    break;
                }
            }

            // if it wasn't loaded yet, we load it now
            if (!skip) {
                unsigned int id = TextureFromFile(str.C_Str(), this->directory);
                if (id != 0) {
                    Texture3D texture;
                    texture.id = id;
                    texture.type = typeName;
                    texture.path = str.C_Str();
                    textures.push_back(texture);
                    textures_loaded.push_back(texture); // add to cache so we don't load it again
                }
            }
        }
        return textures;
    }
};
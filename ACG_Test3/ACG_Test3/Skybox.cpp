#include "Skybox.hpp"
#include "dependencies/stb/stb_image.h" // including the image loader for the 6 sky faces

Skybox::Skybox(const std::vector<std::string>& faces) {
    // creating a special shader just for the sky
    m_shader = new Shader("skybox.vert", "skybox.frag");
    setupMesh();       // creates the 3d box geometry
    loadCubemap(faces); // loads the 6 textures onto that box
}

Skybox::~Skybox() {
    delete m_shader;
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteTextures(1, &m_textureID);
}

// this function takes 6 individual images and stitches them into one cube texture
void Skybox::loadCubemap(const std::vector<std::string>& faces) {
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        // loading each face (right, left, top, bottom, front, back)
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            // GL_TEXTURE_CUBE_MAP_POSITIVE_X + i is a clever way to loop through all 6 sides
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // setting how the sky looks. we use CLAMP_TO_EDGE so you don't see seams at the corners.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

// creating a simple 3d cube that will hold the sky texture
void Skybox::setupMesh() {

    float skyboxVertices[] = {
        // these are the 36 points needed to make the 12 triangles of a cube
        -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, // back
         1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, // left
        -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // right
         1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, // front
         1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f, // top
         1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f, // bottom
         1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Skybox::draw(const glm::mat4& view, const glm::mat4& projection) {
    // we change the depth function to "less than or equal" 
    // this ensures the skybox is drawn behind everything else in the scene
    glDepthFunc(GL_LEQUAL);

    m_shader->use();

    // this is a trick: we turn the 4x4 view matrix into a 3x3 then back to 4x4.
    // this removes the movement (translation) but keeps the rotation.
    // it makes the skybox follow the camera so you can never "reach" the edge of the sky.
    glm::mat4 viewNoTrans = glm::mat4(glm::mat3(view));

    m_shader->setMat4("view", viewNoTrans);
    m_shader->setMat4("projection", projection);
    m_shader->setInt("skybox", 0);

    glBindVertexArray(m_VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_textureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    // putting the depth function back to normal for the rest of the objects
    glDepthFunc(GL_LESS);
}
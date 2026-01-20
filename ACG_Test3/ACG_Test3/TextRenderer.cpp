//#include "TextRenderer.hpp"
//#include <fstream>
//#include <vector>
//
//// --- STB Setup ---
//// Define implementation macro in ONE C/C++ file
//#define STB_TRUETYPE_IMPLEMENTATION
//#include "dependencies/stb/stb_truetype.h"
//// --- End STB Setup ---
//
//
//// --- GLSL Shader Sources (Unchanged from FreeType version) ---
//const char* textVertexShaderSource = R"glsl(
//#version 330 core
//layout (location = 0) in vec4 vertex; // x, y, tex_x, tex_y
//out vec2 TexCoords;
//
//uniform mat4 projection;
//
//void main()
//{
//    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
//    TexCoords = vertex.zw;
//}
//)glsl";
//
//const char* textFragmentShaderSource = R"glsl(
//#version 330 core
//in vec2 TexCoords;
//out vec4 color;
//
//uniform sampler2D textTexture;
//uniform vec3 textColor;
//
//void main()
//{    
//    vec4 sampled = texture(textTexture, TexCoords);
//    float alpha = sampled.r; 
//    
//    color = vec4(textColor, alpha);
//}
//)glsl";
//
//
//// --- Helper Functions (Shader Compilation - Unchanged) ---
//
//GLuint TextRenderer::compileShader(GLenum type, const char* source) {
//    // ... (Implementation remains the same as previous TextRenderer.cpp) ...
//    GLuint shader = glCreateShader(type);
//    glShaderSource(shader, 1, &source, NULL);
//    glCompileShader(shader);
//
//    int success;
//    char infoLog[512];
//    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
//    if (!success) {
//        glGetShaderInfoLog(shader, 512, NULL, infoLog);
//        std::cerr << "ERROR::SHADER::COMPILATION_FAILED of type " << type << "\n" << infoLog << std::endl;
//    }
//    return shader;
//}
//
//GLuint TextRenderer::createBasicTextShader() {
//    // ... (Implementation remains the same as previous TextRenderer.cpp) ...
//    GLuint vertex = compileShader(GL_VERTEX_SHADER, textVertexShaderSource);
//    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, textFragmentShaderSource);
//
//    GLuint program = glCreateProgram();
//    glAttachShader(program, vertex);
//    glAttachShader(program, fragment);
//    glLinkProgram(program);
//
//    int success;
//    char infoLog[512];
//    glGetProgramiv(program, GL_LINK_STATUS, &success);
//    if (!success) {
//        glGetProgramInfoLog(program, 512, NULL, infoLog);
//        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
//    }
//
//    glDeleteShader(vertex);
//    glDeleteShader(fragment);
//
//    return program;
//}
//
//
//// --- TextRenderer Class Methods (Rewritten for STB) ---
//
//TextRenderer::TextRenderer(int windowWidth, int windowHeight, const std::string& fontPath) {
//    // 1. Setup Shader and Projection
//    shaderProgram = createBasicTextShader();
//    setProjection(windowWidth, windowHeight);
//
//    // 2. Setup VBO and VAO for the text quads
//    glGenVertexArrays(1, &VAO);
//    glGenBuffers(1, &VBO);
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
//
//    glEnableVertexAttribArray(0);
//    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//
//    // --- STB FONT ATLAS GENERATION ---
//
//    // 3. Load font file into memory
//    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
//    if (!file.is_open()) {
//        std::cerr << "ERROR::STB_TRUETYPE: Failed to open font file: " << fontPath << std::endl;
//        return;
//    }
//    std::streamsize size = file.tellg();
//    file.seekg(0, std::ios::beg);
//
//    std::vector<unsigned char> fontBuffer(size);
//    if (!file.read((char*)fontBuffer.data(), size)) {
//        std::cerr << "ERROR::STB_TRUETYPE: Failed to read font file: " << fontPath << std::endl;
//        return;
//    }
//
//    // 4. Allocate memory for the baked character data
//    bakedChars = new stbtt_bakedchar[STB_CHAR_COUNT];
//
//    // 5. Create Atlas Bitmap (CPU)
//    std::vector<unsigned char> temp_bitmap(STB_ATLAS_SIZE * STB_ATLAS_SIZE);
//
//    int result = stbtt_BakeFontBitmap(
//        fontBuffer.data(), 0, STB_FONT_HEIGHT,      // font data, offset, pixel height
//        temp_bitmap.data(), ATLAS_SIZE, ATLAS_SIZE, // output bitmap, size
//        STB_FIRST_CHAR, STB_CHAR_COUNT,             // first character, number of chars
//        bakedChars                                  // baked metrics output
//    );
//
//    if (result <= 0) {
//        std::cerr << "ERROR::STB_TRUETYPE: Failed to bake font bitmap. Result: " << result << std::endl;
//        delete[] bakedChars;
//        bakedChars = nullptr;
//        return;
//    }
//
//    // 6. Create Atlas Texture (GPU)
//    glGenTextures(1, &atlasTextureID);
//    glBindTexture(GL_TEXTURE_2D, atlasTextureID);
//
//    // Upload the bitmap data
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-row alignment
//    glTexImage2D(
//        GL_TEXTURE_2D, 0, GL_RED, ATLAS_SIZE, ATLAS_SIZE, 0,
//        GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data()
//    );
//    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // Restore default alignment
//
//    // Set texture parameters
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//}
//
//TextRenderer::~TextRenderer() {
//    glDeleteVertexArrays(1, &VAO);
//    glDeleteBuffers(1, &VBO);
//    glDeleteProgram(shaderProgram);
//    if (atlasTextureID != 0) {
//        glDeleteTextures(1, &atlasTextureID);
//    }
//    if (bakedChars) {
//        delete[] bakedChars;
//    }
//}
//
//void TextRenderer::setProjection(int width, int height) {
//    projection = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
//
//    glUseProgram(shaderProgram);
//    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
//    if (projLoc != -1) {
//        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
//    }
//    glUseProgram(0);
//}
//
//
//void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color) {
//    if (!bakedChars || atlasTextureID == 0) return;
//
//    glUseProgram(shaderProgram);
//    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color.x, color.y, color.z);
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, atlasTextureID); // Bind the single atlas texture
//    glBindVertexArray(VAO);
//    glBindBuffer(GL_ARRAY_BUFFER, VBO);
//
//    // STB renders text from bottom-left (OpenGL's origin) up, 
//    // but the baked metrics expect Y to be the baseline.
//    // Adjust y to match a more common UI style (top-down coordinates with scale).
//    float adjusted_y = y - STB_FONT_HEIGHT * scale;
//
//    // Current cursor position
//    float currentX = x;
//    float currentY = adjusted_y;
//
//    for (char c : text) {
//        if (c < STB_FIRST_CHAR || c >= STB_FIRST_CHAR + STB_CHAR_COUNT) continue;
//
//        stbtt_aligned_quad q;
//
//        // GetBakedQuad calculates the vertices and UVs for the character
//        stbtt_GetBakedQuad(
//            bakedChars, ATLAS_SIZE, ATLAS_SIZE,
//            c - STB_FIRST_CHAR,             // Index into the bakedChars array
//            &currentX, &currentY,           // Current cursor position (updated by STB)
//            &q, 1                           // quad output, opengl-style coordinate system
//        );
//
//        // STB's baked metrics are based on the FONT_HEIGHT used in baking. 
//        // We need to scale the resulting quad vertices by the rendering scale factor.
//        float scale_ratio = scale / (1.0f); // Scale factor from baked size (1.0) to current size (scale)
//
//        float vertices[6][4] = {
//            //   x      y        u        v
//            { q.x0 * scale_ratio, q.y0 * scale_ratio, q.s0, q.t0 }, // Top-left
//            { q.x0 * scale_ratio, q.y1 * scale_ratio, q.s0, q.t1 }, // Bottom-left
//            { q.x1 * scale_ratio, q.y1 * scale_ratio, q.s1, q.t1 }, // Bottom-right
//
//            { q.x0 * scale_ratio, q.y0 * scale_ratio, q.s0, q.t0 }, // Top-left
//            { q.x1 * scale_ratio, q.y1 * scale_ratio, q.s1, q.t1 }, // Bottom-right
//            { q.x1 * scale_ratio, q.y0 * scale_ratio, q.s1, q.t0 }  // Top-right
//        };
//
//        // Apply the pre-calculated position offset from STB
//        for (int i = 0; i < 6; ++i) {
//            vertices[i][0] += currentX; // This is a bit tricky, stbtt_GetBakedQuad modifies the x/y in a complex way.
//            vertices[i][1] += currentY; // For a proper setup, you typically transform the quad directly.
//        }
//
//        // --- Simpler Vertex Update (Focus on the scaling and offset) ---
//        // A much cleaner way to integrate with the previous FreeType approach:
//
//        // Calculate the actual position using STB metrics (q.x0/q.y0 are offsets from currentX/Y)
//        float x_offset = x + q.x0 * scale;
//        float y_offset = y + q.y0 * scale;
//        float w = (q.x1 - q.x0) * scale;
//        float h = (q.y1 - q.y0) * scale;
//
//        // Reset the quad vertices using the scaled and positioned data.
//        float quad_vertices[6][4] = {
//            { x_offset,     y_offset + h, q.s0, q.t0 },
//            { x_offset,     y_offset,     q.s0, q.t1 },
//            { x_offset + w, y_offset,     q.s1, q.t1 },
//
//            { x_offset,     y_offset + h, q.s0, q.t0 },
//            { x_offset + w, y_offset,     q.s1, q.t1 },
//            { x_offset + w, y_offset + h, q.s1, q.t0 }
//        };
//
//        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_vertices), quad_vertices);
//        glDrawArrays(GL_TRIANGLES, 0, 6);
//
//        // Advance cursor
//        x += q.x1 - q.x0; // STB sets q.x1 to the new advance position, q.x0 is the current position
//    }
//
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
//    glBindTexture(GL_TEXTURE_2D, 0);
//    glUseProgram(0);
//}
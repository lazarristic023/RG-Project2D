// TextRenderer.h
#ifndef TEXTRENDERER_H
#define TEXTRENDERER_H

#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <map>
#include <string>
#include <GL/glew.h>

struct Character {
    GLuint TextureID;   // Texture ID
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
    GLuint Advance;     // Offset to advance to next glyph
};

class TextRenderer {
public:
    // Constructor
    TextRenderer(unsigned int width, unsigned int height);

    // Destructor
    ~TextRenderer();

    // Initialize FreeType and load the font
    bool Load(const std::string& fontPath, unsigned int fontSize);

    // Render text
    void RenderText(const std::string& text, float x, float y, float scale, glm::vec3 color);

    // Shader program ID
    GLuint ShaderID;

private:
    // Holds a list of pre-compiled Characters
    std::map<char, Character> Characters;

    // Render state
    GLuint VAO, VBO;

    // Projection matrix
    glm::mat4 Projection;

    // Initialize shaders
    bool InitShaders();

    // Initialize buffers
    void InitBuffers();
};

#endif // TEXTRENDERER_H

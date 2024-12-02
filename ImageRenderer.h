#ifndef IMAGERENDERER_H
#define IMAGERENDERER_H

#include <glm/glm.hpp>
#include <GL/glew.h>

class ImageRenderer {
public:
    // Constructor
    ImageRenderer(unsigned int width, unsigned int height);

    // Destructor
    ~ImageRenderer();

    // Render an image
    void RenderImage(GLuint textureID, float x, float y, float width, float height);

    // Shader program ID
    GLuint ShaderID;

private:
    // Render state
    GLuint VAO, VBO;

    // Projection matrix
    glm::mat4 Projection;

    // Initialize shaders
    bool InitShaders();

    // Initialize buffers
    void InitBuffers();
};

#endif // IMAGERENDERER_H

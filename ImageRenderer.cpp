#include "ImageRenderer.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// Vertex Shader Source
const char* imageVertexShaderSrc = R"glsl(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)glsl";

// Fragment Shader Source
const char* imageFragmentShaderSrc = R"glsl(
#version 330 core
in vec2 TexCoords;         // Interpolated texture coordinates from the vertex shader
out vec4 FragColor;        // Output color

uniform sampler2D image;   // The texture being sampled

void main() {
    // Sample the texture and output the color with alpha
    FragColor = texture(image, TexCoords);
}
)glsl";

// Helper function to compile a shader
GLuint CompileImageShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n"
            << infoLog << std::endl;
    }
    return shader;
}

// Constructor
ImageRenderer::ImageRenderer(unsigned int width, unsigned int height) {
    // Initialize projection matrix
    Projection = glm::ortho(0.0f, static_cast<float>(width),
        0.0f, static_cast<float>(height));

    // Initialize shaders and buffers
    if (!InitShaders()) {
        std::cerr << "Failed to initialize shaders for ImageRenderer." << std::endl;
    }
    InitBuffers();
}

// Destructor
ImageRenderer::~ImageRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(ShaderID);
}

// Initialize shaders
bool ImageRenderer::InitShaders() {
    GLuint vertexShader = CompileImageShader(GL_VERTEX_SHADER, imageVertexShaderSrc);
    GLuint fragmentShader = CompileImageShader(GL_FRAGMENT_SHADER, imageFragmentShaderSrc);

    // Shader Program
    ShaderID = glCreateProgram();
    glAttachShader(ShaderID, vertexShader);
    glAttachShader(ShaderID, fragmentShader);
    glLinkProgram(ShaderID);

    // Check for linking errors
    GLint success;
    glGetProgramiv(ShaderID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(ShaderID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
            << infoLog << std::endl;
        return false;
    }

    // Delete shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set the projection matrix uniform
    glUseProgram(ShaderID);
    glUniformMatrix4fv(glGetUniformLocation(ShaderID, "projection"), 1, GL_FALSE, glm::value_ptr(Projection));
    glUseProgram(0);

    return true;
}

// Initialize buffers
void ImageRenderer::InitBuffers() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Render image
void ImageRenderer::RenderImage(GLuint textureID, float x, float y, float width, float height) {

    // Save previous OpenGL states
    GLboolean blendingEnabled;
    glGetBooleanv(GL_BLEND, &blendingEnabled);
    GLint currentBlendSrc, currentBlendDst;
    glGetIntegerv(GL_BLEND_SRC, &currentBlendSrc);
    glGetIntegerv(GL_BLEND_DST, &currentBlendDst);
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    GLint currentVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Activate the shader program
    glUseProgram(ShaderID);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);


    // Calculate vertices for the quad
    float vertices[6][4] = {
    { x,         y,          0.0f, 0.0f },
    { x,         y + height, 0.0f, 1.0f },
    { x + width, y,          1.0f, 0.0f },

    { x,         y + height, 0.0f, 1.0f },
    { x + width, y + height, 1.0f, 1.0f },
    { x + width, y,          1.0f, 0.0f }
    };

    // Update VBO data
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // Render the quad
    glDrawArrays(GL_TRIANGLES, 0, 6);


    // Restore previous OpenGL states
    if (!blendingEnabled) {
        glDisable(GL_BLEND);
    }
    glBlendFunc(currentBlendSrc, currentBlendDst);
    glUseProgram(currentProgram);
    glBindVertexArray(currentVAO);
    glBindTexture(GL_TEXTURE_2D, 0);
}

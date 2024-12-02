//Autor: Nedeljko Tesanovic
//Opis: Primjer upotrebe tekstura

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip> // Include this for std::setprecision
#include <thread>
#include <chrono>



#include <GL/glew.h>
#include <GLFW/glfw3.h>

//stb_image.h je header-only biblioteka za ucitavanje tekstura.
//Potrebno je definisati STB_IMAGE_IMPLEMENTATION prije njenog ukljucivanja
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "TextRenderer.h"
#include "ImageRenderer.h"

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath); //Ucitavanje teksture, izdvojeno u funkciju
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);


std::string getCurrentTime() {
    std::time_t now = std::time(nullptr);
    std::tm* localTime = std::localtime(&now);
    char buffer[9];
    std::strftime(buffer, sizeof(buffer), "%H:%M:%S", localTime);
    return std::string(buffer);
}

bool logoMode = false;
bool transparentMode = true;
float upperChamberTemp = 4.0f;
float lowerChamberTemp = -30.0f;
float backgroundColor = 0.4f;
int activeLogoMode = 1;

double lastClickTime = 0.0;

std::string upperChamberTempStr = "4.0";
std::string lowerChamberTempStr = "-30.0";

void UpdateTemperatureString(bool upper) {
    std::ostringstream stream;
    if (upper) {
        stream << std::fixed << std::setprecision(1) << upperChamberTemp;
        upperChamberTempStr = stream.str();
    }
    else 
    {
        stream << std::fixed << std::setprecision(1) << lowerChamberTemp;
        lowerChamberTempStr = stream.str();
    }

}

int main(void)
{

    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window;
    unsigned int wWidth = 900;
    unsigned int wHeight = 900;
    const char wTitle[] = "Smart Fridge";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, NULL, NULL);
    
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window);


    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    //glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);


    unsigned int basicPosShader = createShader("shaders/basicPos.vert", "shaders/basicPos.frag");
    unsigned int uniformAlphaShader = createShader("shaders/basicPos.vert", "shaders/uniformAlpha.frag");
    unsigned int mode2Shader = createShader("shaders/basicPos.vert", "shaders/mode2.frag");
    unsigned int mode3Shader = createShader("shaders/mode3.vert", "shaders/basicPos.frag");
    unsigned int mode4Shader = createShader("shaders/mode4.vert", "shaders/basicPos.frag");
    unsigned int mode5Shader = createShader("shaders/mode5.vert", "shaders/mode5.frag");


    std::vector<unsigned int> vboIDs;
    std::vector<unsigned int> vaoIDs;
    std::vector<unsigned int> eboIDs;
    std::vector<unsigned int> textureIDs;

    // ********************************    OBJECTS  ************************************

    //  --------- COMMON OBJECTS --------------

    //  1) RECTANGLES


    float fridgeRectanglesV[] = {

     -0.5 , -0.8,     0.7, 0.7, 0.7,   // 1 - outline
      0.5 , -0.8,     0.7, 0.7, 0.7,
     -0.5 ,  0.8,     0.7, 0.7, 0.7,
      0.5 ,  0.8,     0.7, 0.7, 0.7,

     -0.355 , -0.05,   0.0, 1.0, 0.0, //2 - upper temp
     -0.145 , -0.05,   0.0, 1.0, 0.0,
     -0.355 , 0.05,    0.0, 1.0, 0.0,
     -0.145 , 0.05,    0.0, 1.0, 0.0,

     -0.355 , -0.25,   0.0, 1.0, 0.0, // 3 - lower temp 
     -0.145 , -0.25,   0.0, 1.0, 0.0,
     -0.355 , -0.15,   0.0, 1.0, 0.0,
     -0.145 , -0.15,   0.0, 1.0, 0.0,

     -0.25 , 0.8,   0.0, 0.0, 0.0,  // 4 - clock
      0.25 , 0.8,   0.0, 0.0, 0.0,
     -0.25 , 0.9,   0.0, 0.0, 0.0,
      0.25 , 0.9,   0.0, 0.0, 0.0,


    };

    unsigned int indices[] = {
        0, 1, 2,   // 1
        2, 1, 3,

        4, 5, 6,   // 2
        6, 5, 7,

        8, 9, 10, // 3
        10, 9, 11,

        12, 13, 14, // 4
        14, 13, 15,



    };

    unsigned int fridgeStride = 5 * sizeof(float);

    unsigned int fridgeRectanglesVBO, fridgeRectanglesVAO, fridgeRectanglesEBO;
    glGenVertexArrays(1, &fridgeRectanglesVAO);
    glGenBuffers(1, &fridgeRectanglesVBO);
    glGenBuffers(1, &fridgeRectanglesEBO);

    vboIDs.push_back(fridgeRectanglesVBO);
    vaoIDs.push_back(fridgeRectanglesVAO);
    eboIDs.push_back(fridgeRectanglesEBO);

    glBindVertexArray(fridgeRectanglesVAO);

    glBindBuffer(GL_ARRAY_BUFFER, fridgeRectanglesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fridgeRectanglesV), fridgeRectanglesV, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fridgeRectanglesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, fridgeStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, fridgeStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    // 2) LINES

    float fridgeLinesV[] = {

     -0.5 ,  0.1,     0.0, 0.0, 0.0,
      0.5 ,  0.1,     0.0, 0.0, 0.0,
     -0.5 , -0.3,    0.0, 0.0, 0.0,
      0.5 , -0.3,    0.0, 0.0, 0.0,
      0.0 ,  0.1,     0.0, 0.0, 0.0,
      0.0 , -0.3,    0.0, 0.0, 0.0,
      0.0 , -0.1,    0.0, 0.0, 0.0,
      0.5 , -0.1,    0.0, 0.0, 0.0,

    };

    unsigned int fridgeLinesVAO; //0 - N, 1 - strip
    glGenVertexArrays(1, &fridgeLinesVAO);
    unsigned int fridgeLinesVBO;
    glGenBuffers(1, &fridgeLinesVBO);

    vboIDs.push_back(fridgeLinesVBO);
    vaoIDs.push_back(fridgeLinesVAO);

    glBindVertexArray(fridgeLinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fridgeLinesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fridgeLinesV), fridgeLinesV, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, fridgeStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, fridgeStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    // ---------  GUI MODE OBJECTS  ------------

    float handlesVertices[] = {

     -0.4 , 0.55,    0.0, 0.0, 0.0,  // 1 - upper handle
     -0.34 , 0.55,     0.0, 0.0, 0.0,
     -0.4 , 0.35,    0.0, 0.0, 0.0,
     -0.34 , 0.35,     0.0, 0.0, 0.0,

     -0.4 , -0.65,    0.0, 0.0, 0.0,  // 2 - lower handle
     -0.34 , -0.65,     0.0, 0.0, 0.0,
     -0.4 , -0.45,    0.0, 0.0, 0.0,
     -0.34 , -0.45,     0.0, 0.0, 0.0,

      0.2 , -0.25,   1.0, 0.0, 0.0, // 3 - mode button
      0.3 , -0.25,   1.0, 0.0, 0.0,
      0.2 , -0.15,   1.0, 0.0, 0.0,
      0.3 , -0.15,   1.0, 0.0, 0.0,

    };

    unsigned int handlesIndices[] = {

        0, 1, 2,   // 1
        2, 1, 3,

        4, 5, 6,   // 2
        6, 5, 7,

        8, 9, 10,  // 3 
        10, 9, 11,

    };

    unsigned int handlesVBO, handlesVAO, handlesEBO;
    glGenVertexArrays(1, &handlesVAO);
    glGenBuffers(1, &handlesVBO);
    glGenBuffers(1, &handlesEBO);

    vboIDs.push_back(handlesVBO);
    vaoIDs.push_back(handlesVAO);
    eboIDs.push_back(handlesEBO);

    glBindVertexArray(handlesVAO);

    glBindBuffer(GL_ARRAY_BUFFER, handlesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(handlesVertices), handlesVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handlesEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(handlesIndices), handlesIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, fridgeStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, fridgeStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // TRANSPARENT MODE OBJECTS


    //   CHAMBERS


    float chamberVertices[] = {
    
     -0.45 , 0.15,    0.9, 0.9, 0.9,  // 1 - upper chamber
      0.45 , 0.15,    0.9, 0.9, 0.9,
     -0.45 ,  0.75,   0.9, 0.9, 0.9,
      0.45 ,  0.75,   0.9, 0.9, 0.9,
                     
     -0.45 , -0.75,   0.9, 0.9, 0.9,  // 2 - lower chamber
      0.45 , -0.75,   0.9, 0.9, 0.9,
     -0.45 , -0.35,   0.9, 0.9, 0.9,
      0.45 , -0.35,   0.9, 0.9, 0.9,

      0.2 , -0.25,   0.0, 1.0, 0.0, // 3 - mode button
      0.3 , -0.25,   0.0, 1.0, 0.0,
      0.2 , -0.15,   0.0, 1.0, 0.0,
      0.3 , -0.15,   0.0, 1.0, 0.0,

    };

    unsigned int chamberIndices[] = {

        0, 1, 2,   // 1
        2, 1, 3,

        4, 5, 6,   // 2
        6, 5, 7,

        8, 9, 10,  // 3
        10, 9, 11,
    };

    unsigned int chamberVBO, chamberVAO, chamberEBO;
    glGenVertexArrays(1, &chamberVAO);
    glGenBuffers(1, &chamberVBO);
    glGenBuffers(1, &chamberEBO);

    vboIDs.push_back(chamberVBO);
    vaoIDs.push_back(chamberVAO);
    eboIDs.push_back(chamberEBO);

    glBindVertexArray(chamberVAO);

    glBindBuffer(GL_ARRAY_BUFFER, chamberVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(chamberVertices), chamberVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chamberEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(chamberIndices), chamberIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, fridgeStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, fridgeStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    //  UPPER MEMBRANE

    float upperMembraneVertices[] = {

     -0.45 , 0.15,    0.0, 0.0, 1.0,  // 1 - upper chamber
      0.45 , 0.15,    0.0, 0.0, 1.0,
     -0.45 ,  0.75,   0.0, 0.0, 1.0,
      0.45 ,  0.75,   0.0, 0.0, 1.0,
                      
    };

    unsigned int upperMembraceIndices[] = {
    
        0, 1, 2,   // 1
        2, 1, 3,

    };

    unsigned int upperMembraneVBO, upperMembraneVAO, upperMembraneEBO;
    glGenVertexArrays(1, &upperMembraneVAO);
    glGenBuffers(1, &upperMembraneVBO);
    glGenBuffers(1, &upperMembraneEBO);

    vboIDs.push_back(upperMembraneVBO);
    vaoIDs.push_back(upperMembraneVAO);
    eboIDs.push_back(upperMembraneEBO);

    glBindVertexArray(upperMembraneVAO);

    glBindBuffer(GL_ARRAY_BUFFER, upperMembraneVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(upperMembraneVertices), upperMembraneVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, upperMembraneEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(upperMembraceIndices), upperMembraceIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, fridgeStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, fridgeStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    //  LOWER MEMBRANE

    float lowerMembraneVertices[] = {

     -0.45 , -0.75,   0.0, 0.0, 1.0,  // 1 - lower chamber
      0.45 , -0.75,   0.0, 0.0, 1.0,
     -0.45 , -0.35,   0.0, 0.0, 1.0,
      0.45 , -0.35,   0.0, 0.0, 1.0,
    };

    unsigned int lowerMembraceIndices[] = {

    0, 1, 2,   // 1
    2, 1, 3,

    };

    unsigned int lowerMembraneVBO, lowerMembraneVAO, lowerMembraneEBO;
    glGenVertexArrays(1, &lowerMembraneVAO);
    glGenBuffers(1, &lowerMembraneVBO);
    glGenBuffers(1, &lowerMembraneEBO);

    vboIDs.push_back(lowerMembraneVBO);
    vaoIDs.push_back(lowerMembraneVAO);
    eboIDs.push_back(lowerMembraneEBO);

    glBindVertexArray(lowerMembraneVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lowerMembraneVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lowerMembraneVertices), lowerMembraneVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lowerMembraneEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lowerMembraceIndices), lowerMembraceIndices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, fridgeStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, fridgeStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);




    // LOGO MODE


    float logoVertices[] = {

        // L
        -0.3f,  0.2f,  1.0f, 0.0f, 0.0f,  // Top of L
        -0.3f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom of L
        -0.3f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom of L
        -0.2f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom right of L

        // O (Base rectangle for the lock body)
        -0.1f,  0.2f,  1.0f, 0.0f, 0.0f,  // Top-left of O
         0.1f,  0.2f,  1.0f, 0.0f, 0.0f,  // Top-right of O
         0.1f,  0.2f,  1.0f, 0.0f, 0.0f,  // Top-right of O
         0.1f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-right of O
         0.1f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-right of O
        -0.1f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-left of O
        -0.1f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-left of O
        -0.1f,  0.2f,  1.0f, 0.0f, 0.0f,  // Top-left of O

        // O (Top shackle rectangle)
        -0.05f,  0.3f,  1.0f, 0.0f, 0.0f,  // Top-left of shackle
         0.05f,  0.3f,  1.0f, 0.0f, 0.0f,  // Top-right of shackle
         0.05f,  0.3f,  1.0f, 0.0f, 0.0f,  // Top-right of shackle
         0.05f,  0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-right of shackle
         0.05f,  0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-right of shackle
        -0.05f,  0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-left of shackle
        -0.05f,  0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-left of shackle
        -0.05f,  0.3f,  1.0f, 0.0f, 0.0f,  // Top-left of shackle

        // O (Keyhole inside the rectangle)
        0.0f,   0.1f,  1.0f, 0.0f, 0.0f,  // Top of keyhole
        0.0f,  -0.1f,  1.0f, 0.0f, 0.0f,  // Bottom of keyhole

        // K
         0.2f,  0.2f,  1.0f, 0.0f, 0.0f,  // Top of K
         0.2f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom of K
         0.2f,  0.0f,  1.0f, 0.0f, 0.0f,  // Middle of K
         0.3f,  0.2f,  1.0f, 0.0f, 0.0f,  // Top-right diagonal of K
         0.2f,  0.0f,  1.0f, 0.0f, 0.0f,  // Middle of K
         0.3f, -0.2f,  1.0f, 0.0f, 0.0f,  // Bottom-right diagonal of K
    };


    unsigned int logoLinesVAO; //0 - N, 1 - strip
    glGenVertexArrays(1, &logoLinesVAO);
    unsigned int logoLinesVBO;
    glGenBuffers(1, &logoLinesVBO);

    vboIDs.push_back(logoLinesVBO);
    vaoIDs.push_back(logoLinesVAO);

    glBindVertexArray(logoLinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, logoLinesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(logoVertices), logoVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, fridgeStride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, fridgeStride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);



    // ***********************************************************************************


    TextRenderer textRenderer(wWidth, wHeight);
    if (!textRenderer.Load("res/arial.ttf", 48)) { // Replace with your font path
        std::cerr << "Failed to load font for TextRenderer." << std::endl;
        return -1;
    }

    ImageRenderer imageRenderer(wWidth, wHeight);


    // TEXTURES

    unsigned appleTexture = loadImageToTexture("res/apple.png"); 
    glBindTexture(GL_TEXTURE_2D, appleTexture); 
    glGenerateMipmap(GL_TEXTURE_2D); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    textureIDs.push_back(appleTexture);

    unsigned jackTexture = loadImageToTexture("res/jack.png");
    glBindTexture(GL_TEXTURE_2D, jackTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    textureIDs.push_back(jackTexture);

    unsigned turkeyTex = loadImageToTexture("res/turkey.png");
    glBindTexture(GL_TEXTURE_2D, turkeyTex);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    textureIDs.push_back(turkeyTex);

    unsigned beerTexture = loadImageToTexture("res/beer.png");
    glBindTexture(GL_TEXTURE_2D, beerTexture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    textureIDs.push_back(beerTexture);



    GLsizei indicesCount = 0;
    std::string currentTimeString = getCurrentTime();
    lastClickTime = glfwGetTime();
    float offsetX = 0.0f;

    const double targetFrameTime = 1.0 / 60.0;
    double frameStartTime = 0;
    double frameEndTime = 0;
    double frameDuration = 0;
    double sleepTime = 0;


    unsigned int uA = glGetUniformLocation(mode2Shader, "uA");
    //float uOffsetX = glGetUniformLocation(mode3Shader, "uOffsetX");

    while (!glfwWindowShouldClose(window))
    {

        frameStartTime = glfwGetTime();
        glfwPollEvents();
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        
        double currentTime = glfwGetTime();
        if (currentTime - lastClickTime > 5.0) {
            logoMode = true;
        }
        else {
            logoMode = false;
        }

        if (!logoMode) {

            glClearColor(backgroundColor, backgroundColor, backgroundColor, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(basicPosShader);

            glBindVertexArray(fridgeRectanglesVAO);
            indicesCount = sizeof(indices) / sizeof(indices[0]);
            glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);

            glLineWidth(3);
            glBindVertexArray(fridgeLinesVAO);
            glDrawArrays(GL_LINES, 0, sizeof(fridgeLinesV) / fridgeStride);
            glLineWidth(1);

            textRenderer.RenderText("+", 400.0f, 433.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));

            textRenderer.RenderText("-", 250.0f, 434.0f, 1.2f, glm::vec3(0.0f, 0.0f, 0.0f));

            textRenderer.RenderText(upperChamberTempStr, 320.0f, 435.0f, 0.7f, glm::vec3(0.0f, 0.0f, 0.0f));

            textRenderer.RenderText("+", 400.0f, 342.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));

            textRenderer.RenderText("-", 250.0f, 342.0f, 1.2f, glm::vec3(0.0f, 0.0f, 0.0f));

            textRenderer.RenderText(lowerChamberTempStr, 300.0f, 344.0f, 0.7f, glm::vec3(0.0f, 0.0f, 0.0f));

            textRenderer.RenderText("- ENV +", 478.0f, 433.0f, 1.0f, glm::vec3(0.0f, 0.0f, 0.0f));

            currentTimeString = getCurrentTime();
            textRenderer.RenderText(currentTimeString, 385.0f, 820.0f, 0.7f, glm::vec3(1.0f, 0.0f, 0.0f));

            textRenderer.RenderText("Lazar Ristic RA150/2020", 10.0f, 10.0f, 0.7f, glm::vec3(0.0f, 0.0f, 1.0f));

            if (!transparentMode) {

                glBindVertexArray(handlesVAO);
                indicesCount = sizeof(handlesIndices) / sizeof(handlesIndices[0]);
                glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);

                textRenderer.RenderText("OFF", 545.0f, 355.0f, 0.4f, glm::vec3(0.0f, 0.0f, 0.0f));
            }
            else {

                glBindVertexArray(chamberVAO);
                indicesCount = sizeof(chamberIndices) / sizeof(chamberIndices[0]);
                glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);

                textRenderer.RenderText("ON", 550.0f, 355.0f, 0.4f, glm::vec3(0.0f, 0.0f, 0.0f));

                imageRenderer.RenderImage(appleTexture, 570, 525, 70, 70);

                imageRenderer.RenderImage(jackTexture, 430, 525, 130, 150);

                imageRenderer.RenderImage(beerTexture, 240, 491, 245, 210);

                imageRenderer.RenderImage(turkeyTex, 310, 100, 220, 190);

                float upperChamberTransparency = 0.5f - (upperChamberTemp / 7.0f) * 0.5f;
                float lowerChamberTransparency = 0.5f - ((lowerChamberTemp + 40.0f) / 22.0f) * 0.5f;

                glUseProgram(uniformAlphaShader);
                glUniform1f(glGetUniformLocation(uniformAlphaShader, "transparency"), upperChamberTransparency);

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glBindVertexArray(upperMembraneVAO);
                indicesCount = sizeof(upperMembraceIndices) / sizeof(upperMembraceIndices[0]);
                glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);

                glUniform1f(glGetUniformLocation(uniformAlphaShader, "transparency"), lowerChamberTransparency);
                glBindVertexArray(lowerMembraneVAO);
                indicesCount = sizeof(lowerMembraceIndices) / sizeof(lowerMembraceIndices[0]);
                glDrawElements(GL_TRIANGLES, indicesCount, GL_UNSIGNED_INT, 0);

                glDisable(GL_BLEND);

            }




            glBindVertexArray(0);
            glUseProgram(0);


            

        }
        else {

            glClearColor(backgroundColor, backgroundColor, backgroundColor, backgroundColor);
            glClear(GL_COLOR_BUFFER_BIT);


            if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) activeLogoMode = 1;
            if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) activeLogoMode = 2;
            if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) activeLogoMode = 3;
            if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) activeLogoMode = 4;
            if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) activeLogoMode = 5;


            switch (activeLogoMode) {
            case 2: // Pulsating
                glUseProgram(mode2Shader);
                glUniform1f(uA, abs(sin(3 * (glfwGetTime()))));

                glLineWidth(5);
                glBindVertexArray(logoLinesVAO);
                glDrawArrays(GL_LINES, 0, sizeof(logoVertices) / fridgeStride);
                glLineWidth(1);


                glBindVertexArray(0);
                glUseProgram(0);
                break;

            case 3: // Horizontal movement with wrap-around
                offsetX += 0.02f;
                if (offsetX > 2.0f) {
                    offsetX -= 2.0f; // Reset offset to maintain wrap-around behavior
                }
                glUseProgram(mode3Shader); // Activate shader program
                glUniform1f(glGetUniformLocation(mode3Shader, "uOffsetX"), offsetX);

                glLineWidth(5);
                glBindVertexArray(logoLinesVAO);
                glDrawArrays(GL_LINES, 0, sizeof(logoVertices) / fridgeStride);
                glLineWidth(1);


                glBindVertexArray(0);
                glUseProgram(0);
                break;

            case 4: // Circular motion
                glUseProgram(mode4Shader);
                glUniform2f(glGetUniformLocation(mode4Shader, "uPos"), 0.4* cos(glfwGetTime()* 0.7), 0.4* (sin(glfwGetTime()* 0.7)));
                
                glLineWidth(5);
                glBindVertexArray(logoLinesVAO);
                glDrawArrays(GL_LINES, 0, sizeof(logoVertices) / fridgeStride);
                glLineWidth(1);


                glBindVertexArray(0);
                glUseProgram(0);
                
                break;

            case 5: // Bouncing with transparency

                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                glUseProgram(mode5Shader);
                glUniform1f(glGetUniformLocation(mode5Shader, "offsetX"), 0.7 * cos(glfwGetTime() * 2));

                glLineWidth(5);
                glBindVertexArray(logoLinesVAO);
                glDrawArrays(GL_LINES, 0, sizeof(logoVertices) / fridgeStride);
                glLineWidth(1);

                glDisable(GL_BLEND);

                glBindVertexArray(0);
                glUseProgram(0);

                break;

            default: // Static display
                glUseProgram(basicPosShader);

                glLineWidth(5);
                glBindVertexArray(logoLinesVAO);
                glDrawArrays(GL_LINES, 0, sizeof(logoVertices) / fridgeStride);
                glLineWidth(1);


                glBindVertexArray(0);
                glUseProgram(0);
                break;
            }

            textRenderer.RenderText("Lazar Ristic RA150/2020", 10.0f, 10.0f, 0.7f, glm::vec3(0.0f, 0.0f, 1.0f));

        }

       

        // Calculate the time taken by this frame
        frameEndTime = glfwGetTime();
        frameDuration = frameEndTime - frameStartTime;
        //std::cout << frameDuration << "\n";

        // If the frame completed too quickly, sleep for the remaining time
        if (frameDuration < targetFrameTime)
        {
            // Sleep for the remaining time
            sleepTime = targetFrameTime - frameDuration;
            std::this_thread::sleep_for(std::chrono::duration<double>(sleepTime));
        }
        
        glfwSwapBuffers(window);
    }


    glDeleteTextures(textureIDs.size(), textureIDs.data());
    glDeleteBuffers(vboIDs.size(), vboIDs.data());
    glDeleteBuffers(eboIDs.size(), eboIDs.data());
    glDeleteVertexArrays(vaoIDs.size(), vaoIDs.data());

    glDeleteProgram(basicPosShader);
    glDeleteProgram(uniformAlphaShader);
    glDeleteProgram(mode2Shader);

    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
     std::string temp = ss.str();
     const char* sourceCode = temp.c_str();

    int shader = glCreateShader(type);
    
    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram();

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader);
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava koji je format boja ucitane slike
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData);
        glBindTexture(GL_TEXTURE_2D, 0);
        // oslobadjanje memorije zauzete sa stbi_load posto vise nije potrebna
        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    std::cout << "X:" << xpos << ", Y:" << ypos << "\n";
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);


    // ------  CTRL  ----------------

    if (mods == GLFW_MOD_CONTROL && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {

        lastClickTime = glfwGetTime();
        // MINUS
        if ((xpos >= 250 && xpos <= 270) && (ypos >= 448 && ypos <= 457)) {
            if (upperChamberTemp - 1.0f >= 0.0f) {
                upperChamberTemp -= 1.0f;
                UpdateTemperatureString(true);
            }
        }
        if ((xpos >= 250 && xpos <= 270) && (ypos >= 538 && ypos <= 547)) {
            if (lowerChamberTemp - 1.0f >= -40.0f) {
                lowerChamberTemp -= 1.0f;
                UpdateTemperatureString(false);
            }
        }

        // PLUS
        if ((xpos >= 400 && xpos <= 425) && (ypos >= 440 && ypos <= 460)) {
            if (upperChamberTemp + 1.0f <= 7.0f) {
                upperChamberTemp += 1.0f;
                UpdateTemperatureString(true);
            }
        }
        if ((xpos >= 400 && xpos <= 425) && (ypos >= 530 && ypos <= 550)) {
            if (lowerChamberTemp + 1.0f <= -18.0f) {
                lowerChamberTemp += 1.0f;
                UpdateTemperatureString(false);
            }
        }

        return;
    }
     
    // -------    SHIFT    ------------

    if (mods == GLFW_MOD_SHIFT && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {

        lastClickTime = glfwGetTime();
        // MINUS
        if ((xpos >= 250 && xpos <= 270) && (ypos >= 448 && ypos <= 457)) {
            if (upperChamberTemp - 5.0f >= 0.0f) {
                upperChamberTemp -= 5.0f;
                UpdateTemperatureString(true);
            }
        }
        if ((xpos >= 250 && xpos <= 270) && (ypos >= 538 && ypos <= 547)) {
            if (lowerChamberTemp - 5.0f >= -40.0f) {
                lowerChamberTemp -= 5.0f;
                UpdateTemperatureString(false);
            }
        }

        // PLUS
        if ((xpos >= 400 && xpos <= 425) && (ypos >= 440 && ypos <= 460)) {
            if (upperChamberTemp + 5.0f <= 7.0f) {  // check for upperChamberTemp bounds before adding
                upperChamberTemp += 5.0f;
                UpdateTemperatureString(true);
            }
        }
        if ((xpos >= 400 && xpos <= 425) && (ypos >= 530 && ypos <= 550)) {
            if (lowerChamberTemp + 5.0f <= -18.0f) {  // check for lowerChamberTemp bounds before adding
                lowerChamberTemp += 5.0f;
                UpdateTemperatureString(false);
            }
        }

        return;
    }


    // -------   LEFT CLICK  -----------

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {

        lastClickTime = glfwGetTime();
        // MINUS
        if ((xpos >= 250 && xpos <= 270) && (ypos >= 448 && ypos <= 457)) {
            if (upperChamberTemp - 0.1f >= 0.0f) {
                upperChamberTemp -= 0.1f;
                UpdateTemperatureString(true);
            }
        }
        if ((xpos >= 250 && xpos <= 270) && (ypos >= 538 && ypos <= 547)) {
            if (lowerChamberTemp - 0.1f >= -40.0f) {
                lowerChamberTemp -= 0.1f;
                UpdateTemperatureString(false);
            }
        }

        // PLUS
        if ((xpos >= 400 && xpos <= 425) && (ypos >= 440 && ypos <= 460)) {
            if (upperChamberTemp + 0.1f <= 7.0f) {  // check for upperChamberTemp bounds before adding
                upperChamberTemp += 0.1f;
                UpdateTemperatureString(true);
            }
        }
        if ((xpos >= 400 && xpos <= 425) && (ypos >= 530 && ypos <= 550)) {
            if (lowerChamberTemp + 0.1f <= -18.0f) {  // check for lowerChamberTemp bounds before adding
                lowerChamberTemp += 0.1f;
                UpdateTemperatureString(false);
            }
        }

        // Transparent mode toggle
        if ((xpos >= 540 && xpos <= 585) && (ypos >= 518 && ypos <= 560)) {
            transparentMode = !transparentMode;
        }

        //  Environment

        if ((xpos >= 480 && xpos <= 495) && (ypos >= 450 && ypos <= 458)) {
            if (backgroundColor - 0.05f >= 0.2f) {
                backgroundColor -= 0.05f;
            }
        }

        if ((xpos >= 620 && xpos <= 640) && (ypos >= 440 && ypos <= 460)) {
            if (backgroundColor + 0.05f <= 0.6f) {  
                backgroundColor += 0.05f;
            }
        }

    }

        
}
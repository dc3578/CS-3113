#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, megamanMatrix, projectionMatrix, blue_spikeMatrix, black_spikeMatrix1, black_spikeMatrix2;

// variables
float megaman_y = 4;
float bs_x = 0;
float bls_x1 = 0;
float bls_x2 = 0;
float rotate_deg = 0;
float lastTicks = 0;
int width = 640;
int height = 480;
int direction = 1;
GLuint p_TID, bs_TID, bls_TID;

GLuint LoadTexture(const char* filePath) {
    int w, h, n;
    unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);
    if (image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    stbi_image_free(image);
    return textureID;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project 1", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(0, 0, width, height);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    megamanMatrix = glm::mat4(1.0f);
    blue_spikeMatrix = glm::mat4(1.0f);
    black_spikeMatrix1 = glm::mat4(1.0f);
    black_spikeMatrix2 = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);
    //program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);

    glUseProgram(program.programID);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);
    // Good setting for transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    p_TID = LoadTexture("megaman.png");
    bs_TID = LoadTexture("spike_blue.png");
    bls_TID = LoadTexture("spike_black.png");
}

void ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            gameIsRunning = false;
        }
    }
}

void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float delta = ticks - lastTicks;
    lastTicks = ticks;
    megamanMatrix = glm::mat4(1.0f);
    black_spikeMatrix1 = glm::mat4(1.0f);
    blue_spikeMatrix = glm::mat4(1.0f);
    black_spikeMatrix2 = glm::mat4(1.0f);
    rotate_deg += 45.0f * delta;
    

    // moving megamen down and reset
    if (megaman_y > -4)
        megaman_y -= 2.0f * delta;
    else 
        megaman_y = 4;
    megamanMatrix = glm::translate(megamanMatrix, glm::vec3(-3.0f, megaman_y, 0.0f));
    
    // blue spikes oscillating and rotating
    if (bs_x > 4 || bs_x < 0)
        direction *= -1;
    if (direction == 1)
        bs_x += 1.0f * delta;
    else
        bs_x -= 1.0f * delta;
    blue_spikeMatrix = glm::translate(blue_spikeMatrix, glm::vec3(bs_x, 0.0f, 0.0f));
    blue_spikeMatrix = glm::rotate(blue_spikeMatrix, glm::radians(rotate_deg), glm::vec3(0.0f, 0.0f, 1.0f));

    // black spike 1 rotating
    black_spikeMatrix1 = glm::translate(black_spikeMatrix1, glm::vec3(2.0f, 2.0f, 0.0f));
    black_spikeMatrix1 = glm::rotate(black_spikeMatrix1, glm::radians(rotate_deg), glm::vec3(0.0f, 0.0f, 1.0f));

    // black spike 2 rotating
    black_spikeMatrix2 = glm::translate(black_spikeMatrix2, glm::vec3(2.0f, -2.0f, 0.0f));
    black_spikeMatrix2 = glm::rotate(black_spikeMatrix2, glm::radians(rotate_deg), glm::vec3(0.0f, 0.0f, 1.0f));
}

void DrawObject(glm::mat4 matrix, GLuint textureID) {
    program.SetModelMatrix(matrix);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
    float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    program.SetModelMatrix(megamanMatrix);

    // draw my sprites
    DrawObject(megamanMatrix, p_TID);
    DrawObject(blue_spikeMatrix, bs_TID);
    DrawObject(black_spikeMatrix1, bls_TID);
    DrawObject(black_spikeMatrix2, bls_TID);

    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
    SDL_GL_SwapWindow(displayWindow);
}

void Shutdown() {
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        Render();
    }

    Shutdown();
    return 0;
}
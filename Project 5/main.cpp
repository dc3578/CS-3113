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
#include <SDL_mixer.h>

#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Util.h"
#include "Scene.h"
#include "Level1.h"
#include "Level2.h"
#include "Level3.h"

#define LEVEL_COUNT 3
#define TITLE "Jumper"
#define INSTR "\"Kill and Touch Gray To Proceed\" "
#define MENUTEXT "Press Enter To Play"

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

SDL_Window* displayWindow;
bool gameIsRunning = true;
bool startgame = false;
GLuint fontTextureID;

Scene* currentScene;
Scene* sceneList[LEVEL_COUNT];


void SwitchToScene(Scene* scene) {
    currentScene = scene;
    currentScene->Initialize();
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Jumper", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);

    #ifdef _WINDOWS
        glewInit();
    #endif

    glViewport(0, 0, 640, 480);

    program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

    viewMatrix = glm::mat4(1.0f);
    modelMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    program.SetProjectionMatrix(projectionMatrix);
    program.SetViewMatrix(viewMatrix);

    glUseProgram(program.programID);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // get font texture ID
    fontTextureID = Util::LoadTexture("resources/font1.png");

    // Init Levels
    sceneList[0] = new Level1();
    sceneList[1] = new Level2();
    sceneList[2] = new Level3();
    SwitchToScene(sceneList[0]);
}

void ProcessInput() {
    currentScene->state.player->movement = glm::vec3(0);

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                gameIsRunning = false;
                break;
        
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        if (currentScene->state.player->collidedBottom) {
                            currentScene->state.player->jump = true;
                        }
                        break;
                    
                    case SDLK_RETURN:
                        if (!startgame) {
                            Mix_PlayMusic(currentScene->state.music, -1);
                            startgame = true;
                        }
                        break;
                }
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    // only process input when game starts
    if (startgame) {
        if (!currentScene->state.gameover) {
            const Uint8* keys = SDL_GetKeyboardState(NULL);
            if (keys[SDL_SCANCODE_LEFT]) {
                currentScene->state.player->movement.x = -1.0f;
                currentScene->state.player->animIndices = currentScene->state.player->animLeft;
            }
            else if (keys[SDL_SCANCODE_RIGHT]) {
                currentScene->state.player->movement.x = 1.0f;
                currentScene->state.player->animIndices = currentScene->state.player->animRight;
            }

            if (glm::length(currentScene->state.player->movement) > 1.0f) {
                currentScene->state.player->movement = glm::normalize(currentScene->state.player->movement);
            }
        }
        // stop all movement and accelleration once game is over
        else {
            currentScene->state.player->isActive = false;
        }
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;
void Update() {
    if (startgame) {
        float ticks = (float)SDL_GetTicks() / 1000.0f;
        float delta = ticks - lastTicks;
        lastTicks = ticks;
        delta += accumulator;

        if (delta < FIXED_TIMESTEP) {
            accumulator = delta;
            return;
        }
        while (delta >= FIXED_TIMESTEP) {
            // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
            currentScene->Update(FIXED_TIMESTEP);

            delta -= FIXED_TIMESTEP;
        }
        accumulator = delta;

        viewMatrix = glm::mat4(1.0f);
        if (currentScene->state.player->position.x > 5) {
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-currentScene->state.player->position.x, 3.75, 0));
        }
        else {
            viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
        }

    }
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Display Menu while game not started
    if (!startgame) {
        Util::DrawText(&program, fontTextureID, TITLE, 0.5, -0.25, glm::vec3(-0.75, 0.5, 0));
        Util::DrawText(&program, fontTextureID, INSTR, 0.25, -0.125, glm::vec3(-2.0, 0, 0));
        Util::DrawText(&program, fontTextureID, MENUTEXT, 0.5, -0.25, glm::vec3(-2.25, -0.5, 0));
    } 
    else {
        program.SetViewMatrix(viewMatrix);
        // render Scene 
        currentScene->Render(&program);

        if (currentScene->state.gameover) {
            Mix_HaltMusic();
        }
    }

    SDL_GL_SwapWindow(displayWindow);
}

void UpdateScene() {
    if (currentScene->state.nextScene >= 0) {
        int curr_lives = currentScene->state.player->lives;
        SwitchToScene(sceneList[currentScene->state.nextScene]);
        currentScene->state.player->lives = curr_lives;
    }
}

void Shutdown() {
    Mix_FreeChunk(currentScene->state.player->jumpSound);
    Mix_FreeMusic(currentScene->state.music);
    SDL_Quit();
}


int main(int argc, char* argv[]) {
    Initialize();

    while (gameIsRunning) {
        ProcessInput();
        Update();
        UpdateScene();
        Render();
    }

    Shutdown();
    return 0;
}
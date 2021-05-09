#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
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

#define LEVEL_COUNT 2
#define TITLE "Basement Treasures"
#define INSTR "\"Collect All Coins And Find Exit\" "
#define MENUTEXT "Press Enter To Play"

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

SDL_Window* displayWindow;
GLuint fontTextureID;
Scene* currentScene;
Scene* sceneList[LEVEL_COUNT];

bool gameIsRunning = true;
bool startgame = false;
float view_x = 0;
float view_y = 0;


void SwitchToScene(Scene* scene) {
    currentScene = scene;
    currentScene->Initialize();
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    displayWindow = SDL_CreateWindow("Basement Treasures", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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
            else if (keys[SDL_SCANCODE_UP]) {
                currentScene->state.player->movement.y = 1.0f;
                currentScene->state.player->animIndices = currentScene->state.player->animUp;
            }
            else if (keys[SDL_SCANCODE_DOWN]) {
                currentScene->state.player->movement.y = -1.0f;
                currentScene->state.player->animIndices = currentScene->state.player->animDown;
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

void UpdateScene() {
    if (currentScene->state.nextScene >= 0) {
        int curr_lives = currentScene->state.player->lives;
        SwitchToScene(sceneList[currentScene->state.nextScene]);
        currentScene->state.player->lives = curr_lives;
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

        float xpos = currentScene->state.player->position.x;
        float ypos = currentScene->state.player->position.y;
        // conditions to move view matrix in x direction
        if (xpos > 19) {
            view_x = -19;
        }
        else if (xpos > 5) {
            view_x = -xpos;
        }
        else {
            view_x = -5;
        }

        // conditions to move view matrix in y direciton
        if (ypos > -20 && ypos < -5) {
            view_y = -currentScene->state.player->position.y;
        }
        else if (ypos > -5) {
            view_y = 4;
        }
        else if (ypos < -20) {
            view_y = 20;
        } 
        viewMatrix = glm::translate(viewMatrix, glm::vec3(view_x, view_y, 0));

        UpdateScene();
    }
}

void Render() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Display Menu while game not started
    if (!startgame) {
        Util::DrawText(&program, fontTextureID, TITLE, 0.5, -0.25, glm::vec3(-2.0, 0.5, 0));
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

void Shutdown() {
    Mix_FreeChunk(currentScene->state.player->sfx);
    Mix_FreeMusic(currentScene->state.music);
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
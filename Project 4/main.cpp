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
#include "Entity.h"
#include <vector>
#define OBJECT_COUNT 30
#define ENEMY_COUNT 3
struct GameState {
    Entity *player;
    Entity *objects;
    Entity *enemies;
};

GameState state;
SDL_Window* displayWindow;
bool gameIsRunning = true;
float gravity = -9.81f;
GLuint fontTextureID;
bool gameover = false;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

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

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text,
    float size, float spacing, glm::vec3 position)
{
    float width = 1.0f / 16.0f;
    float height = 1.0f / 16.0f;
    std::vector<float> vertices;
    std::vector<float> texCoords;
    for (int i = 0; i < text.size(); i++) {
        int index = (int)text[i];
        float offset = (size + spacing) * i;
        float u = (float)(index % 16) / 16.0f;
        float v = (float)(index / 16) / 16.0f;
        vertices.insert(vertices.end(), {
            offset + (-0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (0.5f * size), -0.5f * size,
            offset + (0.5f * size), 0.5f * size,
            offset + (-0.5f * size), -0.5f * size,
            });
        texCoords.insert(texCoords.end(), {
            u, v,
            u, v + height,
            u + width, v,
            u + width, v + height,
            u + width, v,
            u, v + height,
            });
    } // end of for loop
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    program->SetModelMatrix(modelMatrix);
    glUseProgram(program->programID);
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTextureID);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void InitPlayer() {
    state.player = new Entity();
    state.player->position = glm::vec3(-4.0, -1, 0);
    state.player->acceleration = glm::vec3(0, gravity, 0);
    state.player->speed = 1.5f;
    state.player->textureID = LoadTexture("george_0.png");

    state.player->animRight = new int[4]{ 3, 7, 11, 15 };
    state.player->animLeft = new int[4]{ 1, 5, 9, 13 };
    state.player->animUp = new int[4]{ 2, 6, 10, 14 };
    state.player->animDown = new int[4]{ 0, 4, 8, 12 };

    state.player->animIndices = state.player->animRight;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
    state.player->animCols = 4;
    state.player->animRows = 4;
    state.player->height = 0.8f; 
    state.player->width = 0.8f;
    state.player->jumpPower = 7.0f;
    state.player->entityType = PLAYER;
}

void InitPlatforms() {
    state.objects = new Entity[OBJECT_COUNT];
    GLuint obstacle_TID = LoadTexture("platform_landing.png");

    int i = 0;
    float x = -5.0;
    float y = -3.5;
    for (int i = 0; i < OBJECT_COUNT; i++) {
        state.objects[i].entityType = PLATFORM;
        state.objects[i].textureID = obstacle_TID;
        // first 16 platforms for left/right walls
        if (i < 16) {
            // reset coordinates for right wall
            if (i == 8) {
                x = 5.0;
                y = -3.5;
            }
            state.objects[i].position = glm::vec3(x, y, 0);
            y += 1.0;
        }
        // next 9 platforms for the floor
        else if (i < 25) {
            if (i == 16) {
                x = -4.0;
            }
            state.objects[i].position = glm::vec3(x, -3.5, 0);
            x += 1.0;
        }
    }
    // last 5 platforms are obstacle platforms
    state.objects[25].position = glm::vec3(-2.5, -2.5, 0);
    state.objects[26].position = glm::vec3(-1.5, -2.5, 0);
    state.objects[27].position = glm::vec3(0.75, -1.5, 0);
    state.objects[28].position = glm::vec3(1.75, -1.5, 0);
    state.objects[29].position = glm::vec3(2.75, -1.5, 0);
}

void InitEnemies() {
    state.enemies = new Entity[ENEMY_COUNT];
    GLuint enemy_TID = LoadTexture("ken.png");

    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].textureID = enemy_TID;
        state.enemies[i].acceleration = glm::vec3(0, gravity, 0);
        state.enemies[i].width = 0.7f;
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].speed = 0.5f;
    }

    // on first jump platform
    state.enemies[0].position = glm::vec3(-1.5, -1.5, 0);
    state.enemies[0].aiType = WAITANDGO;
    state.enemies[0].aiState = IDLE;
    // on second jump platform
    state.enemies[1].position = glm::vec3(2.5, -0.5, 0);
    state.enemies[1].aiType = JUMPER;
    state.enemies[1].jumpPower = 3.0f;
    // on ground floor
    state.enemies[2].position = glm::vec3(4, -2.5, 0);
    state.enemies[2].aiType = CHASER;
    state.enemies[2].aiState = CHASING;
    
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Project 4", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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
    fontTextureID = LoadTexture("font1.png");
    // Initialize Player
    InitPlayer();
    // Initialize platforms And Landing Platform
    InitPlatforms();
    // Initialize Enemies
    InitEnemies();
}

void ProcessInput() {
    state.player->movement = glm::vec3(0);

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
                        if (state.player->collidedBottom) {
                            state.player->jump = true;
                        }
                        break;
                }
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    // only process input while game is not over
    if (!gameover) {
        const Uint8* keys = SDL_GetKeyboardState(NULL);
        if (keys[SDL_SCANCODE_LEFT]) {
            state.player->movement.x = -1.0f;
            state.player->animIndices = state.player->animLeft;
        }
        else if (keys[SDL_SCANCODE_RIGHT]) {
            state.player->movement.x = 1.0f;
            state.player->animIndices = state.player->animRight;
        }

        if (glm::length(state.player->movement) > 1.0f) {
            state.player->movement = glm::normalize(state.player->movement);
        }
    }
    // stop all movement and accelleration once game is over
    else {
        state.player->isActive = false;

        for (int i = 0; i < OBJECT_COUNT; i++) {
            state.objects[i].acceleration = glm::vec3(0);
            state.objects[i].velocity = glm::vec3(0);
        }

        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].acceleration = glm::vec3(0);
            state.enemies[i].velocity = glm::vec3(0);
            state.enemies[i].movement = glm::vec3(0);
        }
    }
}

#define FIXED_TIMESTEP 0.0166666f
float lastTicks = 0;
float accumulator = 0.0f;
void Update() {
    float ticks = (float)SDL_GetTicks() / 1000.0f;
    float delta = ticks - lastTicks;
    lastTicks = ticks;
    delta += accumulator;

    // only update platforms 1 time
    for (int i = 0; i < OBJECT_COUNT; i++) {
        state.objects[i].Update(0, NULL, NULL, NULL, 0, 0);
    }

    if (delta < FIXED_TIMESTEP) {
        accumulator = delta;
        return;
    }
    while (delta >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->Update(FIXED_TIMESTEP, state.player, state.objects, state.enemies, OBJECT_COUNT, ENEMY_COUNT);

        // update enemies
        for (int i = 0; i < ENEMY_COUNT; i++) {
            state.enemies[i].Update(FIXED_TIMESTEP, state.player, state.objects, state.enemies, OBJECT_COUNT, ENEMY_COUNT);
        }
        delta -= FIXED_TIMESTEP;
    }
    accumulator = delta;
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    // render players 
    state.player->Render(&program);
    // render objects
    for (int i = 0; i < OBJECT_COUNT; i++) {
        state.objects[i].Render(&program);
    }
    // render enemies
    for (int i = 0; i < ENEMY_COUNT; i++) {
        state.enemies[i].Render(&program);
    }

    if (state.player->kills == ENEMY_COUNT) {
        DrawText(&program, fontTextureID, "You Win!", 0.75, -0.375, glm::vec3(-3.5, 3.25, 0));
        gameover = true;
    }
    else if (state.player->lastCollision != NULL && state.player->hitEnemy && !state.player->hitEnemyHead) {
        DrawText(&program, fontTextureID, "You Lose!", 0.75, -0.375, glm::vec3(-3.5, 3.25, 0));
        gameover = true;
    }

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
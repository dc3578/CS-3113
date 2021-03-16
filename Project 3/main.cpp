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
#define OBJECT_COUNT 28
struct GameState {
    Entity *player;
    Entity* objects;
};

GameState state;
SDL_Window* displayWindow;
bool gameIsRunning = true;
float gravity = -0.0981f;
GLuint fontTextureID;
bool successful = false;
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

void MakePlatforms() {
    state.objects = new Entity[OBJECT_COUNT];
    GLuint obstacle_TID = LoadTexture("platform_obstacle.png");
    GLuint landing_TID = LoadTexture("platform_landing.png");

    int i = 0;
    float x = -4.5;
    float y = -3.5;
    // first 16 platforms for left/right walls
    for (; i < 16; i++) {
        // reset coordinates for right wall
        if (i == 8) {
            y = -3.5;
            x = 4.5;
        }
        state.objects[i].textureID = obstacle_TID;
        state.objects[i].position = glm::vec3(x, y, 0);
        y += 1.0;
    }
    // next 8 platforms for the floor : includes landing tiles
    x = -3.5;
    for (; i < 24; i++) {
        // tile 16 & 17 are the landing platforms
        if (i < 18) {
            state.objects[i].textureID = landing_TID;
            std::cout << i << std::endl;
        }
        else {
            state.objects[i].textureID = obstacle_TID;
        }
        state.objects[i].position = glm::vec3(x, -3.5, 0);
        x += 1.0;
    }
    // last 4 platforms are within the walls
    state.objects[24].textureID = obstacle_TID;
    state.objects[24].position = glm::vec3(-3.5, 2.25, 0);;
    state.objects[25].textureID = obstacle_TID;
    state.objects[25].position = glm::vec3(-2.5, 2.25, 0);;
    state.objects[26].textureID = obstacle_TID;
    state.objects[26].position = glm::vec3(0, 0, 0);;
    state.objects[27].textureID = obstacle_TID;
    state.objects[27].position = glm::vec3(1, 0, 0);;
}

void Initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("Lunar Lander!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_OPENGL);
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
    state.player = new Entity();
    state.player->position = glm::vec3(0, 3.75, 0);
    state.player->acceleration = glm::vec3(0, gravity, 0);
    state.player->textureID = LoadTexture("ship_red.png");

    // Initialize platforms And Landing Platform
    MakePlatforms();
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
        }
    }

    const Uint8* keys = SDL_GetKeyboardState(NULL);
    // only process input while game is not over
    if (!gameover) {
        if (keys[SDL_SCANCODE_LEFT]) {
            state.player->acceleration.x -= 0.05f;
        }
        else if (keys[SDL_SCANCODE_RIGHT]) {
            state.player->acceleration.x += 0.05f;
        }

        if (glm::length(state.player->movement) > 1.0f) {
            state.player->movement = glm::normalize(state.player->movement);
        }
    }
    // stop all movement and accelleration once game is over
    else {
        state.player->acceleration = glm::vec3(0);
        state.player->velocity = glm::vec3(0);
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
        state.objects[i].Update(0, NULL, 0);
    }


    if (delta < FIXED_TIMESTEP) {
        accumulator = delta;
        return;
    }
    while (delta >= FIXED_TIMESTEP) {
        // Update. Notice it's FIXED_TIMESTEP. Not deltaTime
        state.player->Update(FIXED_TIMESTEP, state.objects, OBJECT_COUNT);
        delta -= FIXED_TIMESTEP;
    }
    accumulator = delta;
}


void Render() {
    glClear(GL_COLOR_BUFFER_BIT);
    // render players and objects
    state.player->Render(&program);
    for (int i = 0; i < OBJECT_COUNT; i++) {
        state.objects[i].Render(&program);
    }

    // render text if collided with platforms
    if (state.player->lastCollision == &state.objects[16] || state.player->lastCollision == &state.objects[17]) {
        DrawText(&program, fontTextureID, "Mission Successful", 0.75, -0.375, glm::vec3(-3.5, 3.25, 0));
        successful = true;
        gameover = true;
    }
    else if (state.player->lastCollision != NULL && !successful) {
        DrawText(&program, fontTextureID, "Mission Failed", 0.75, -0.375, glm::vec3(-3.5, 3.25, 0));
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
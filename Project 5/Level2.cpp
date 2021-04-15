#include "Level2.h"

#define L2_ENEMY_COUNT 1

#define LEVEL2_WIDTH 14
#define LEVEL2_HEIGHT 8

unsigned int level2_data[] =
{
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
 3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
 3, 0, 0, 0, 1, 1, 2, 0, 0, 0, 0, 3, 3, 3,
 3, 1, 1, 0, 2, 2, 2, 0, 0, 0, 0, 3, 3, 3,
 3, 2, 2, 0, 2, 2, 2, 0, 0, 0, 0, 3, 3, 3
};

void Level2::Initialize() {
    state.nextScene = -1;
    GLuint mapTextureID = Util::LoadTexture("resources/tileset.png");
    state.map = new Map(LEVEL2_WIDTH, LEVEL2_HEIGHT, level2_data, mapTextureID, 1.0f, 4, 1);
    // Move over all of the player and enemy code from initialization.
    InitPlayer();
    InitEnemies();
    InitMusic();
}

void Level2::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, state.enemies, L2_ENEMY_COUNT, state.map);
    if (state.player->kills == L2_ENEMY_COUNT) {
        state.nextScene = 2;
    }
}

void Level2::Render(ShaderProgram* program) {
    state.map->Render(program);
    state.player->Render(program);

    // render lives
    float x = state.player->position.x;
    float y = state.player->position.y;
    GLuint font_TID = Util::LoadTexture("resources/font1.png");
    Util::DrawText(program, font_TID, "Lives: " + std::to_string(state.player->lives), 0.25, -0.15, glm::vec3(x - 0.25, y + 0.5, 0));
}

void Level2::InitPlayer() {
    state.player = new Entity();
    state.player->entityType = PLAYER;

    state.player->position = glm::vec3(1, -3, 0);
    state.player->savedPoint = glm::vec3(1, -3, 0);
    state.player->acceleration = glm::vec3(0, -9.81, 0);
    state.player->textureID = Util::LoadTexture("resources/george_0.png");

    state.player->speed = 2.0;
    state.player->jumpPower = 6.0f;
    state.player->jumpSound = Mix_LoadWAV("resources/jump.wav");

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
}

void Level2::InitEnemies() {
    state.enemies = new Entity[L2_ENEMY_COUNT];
    GLuint enemy_TID = Util::LoadTexture("resources/ken.png");

    for (int i = 0; i < L2_ENEMY_COUNT; i++) {
        state.enemies[i].textureID = enemy_TID;
        state.enemies[i].acceleration = glm::vec3(0, -9.81, 0);
        state.enemies[i].width = 0.7f;
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].speed = 1.0f;
        state.enemies[i].isActive = false;
    }

    // Jumping AI
    state.enemies[0].position = glm::vec3(10, -4, 0);
    state.enemies[0].aiType = JUMPER;
    state.enemies[0].jumpPower = 6.0f;

}

void Level2::InitMusic() {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    state.music = Mix_LoadMUS("resources/wholesome.mp3");
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
}
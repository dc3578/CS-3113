#include "Level1.h"

#define L1_ENEMY_COUNT 1

#define LEVEL1_WIDTH 15
#define LEVEL1_HEIGHT 8
unsigned int level1_data[] =
{
 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2,
 2, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 3, 2,
 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2,
 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2
};

void Level1::Initialize() {
    state.nextScene = -1;
    GLuint mapTextureID = Util::LoadTexture("resources/tileset.png");
    state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 4, 1);
	// Move over all of the player and enemy code from initialization.
    InitMusic();
    InitPlayer();
    InitEnemies();
}

void Level1::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, state.enemies, L1_ENEMY_COUNT, state.map);

    // update enemies 
    for (int i = 0; i < L1_ENEMY_COUNT; i++) {
        state.enemies[i].Update(deltaTime, state.player, state.enemies, L1_ENEMY_COUNT, state.map);
    }

    // move to next level after passing the gray tile
    int loc = int(LEVEL1_WIDTH * floor(-state.player->position.y + 1) + floor(state.player->position.x));
    if (level1_data[loc] == 3) {
        state.nextScene = 1;
    }
}

void Level1::Render(ShaderProgram* program) {
    state.map->Render(program);
    state.player->Render(program);

    // render enemies 
    for (int i = 0; i < L1_ENEMY_COUNT; i++) {
        state.enemies[i].Render(program);
    }

    // render lives
    float x = state.player->position.x - 0.25f;
    float y = state.player->position.y + 0.5f;
    GLuint font_TID = Util::LoadTexture("resources/font1.png");
    Util::DrawText(program, font_TID, "Lives: " + std::to_string(state.player->lives), 0.25f, -0.15f, glm::vec3(x, y, 0));

    // lose condition
    if (state.player->lives <= 0) {
        Util::DrawText(program, font_TID, "You Lose!", 0.5f, -0.25f, glm::vec3(4.5, -2.5, 0));
        Mix_HaltMusic();
        state.gameover = true;
        
        //stop all enemy movement
        for (int i = 0; i < L1_ENEMY_COUNT; i++) {
            state.enemies[i].acceleration = glm::vec3(0);
            state.enemies[i].velocity = glm::vec3(0);
            state.enemies[i].movement = glm::vec3(0);
        }
    }
}

void Level1::InitPlayer() {
    state.player = new Entity();
    state.player->entityType = PLAYER;

    state.player->position = glm::vec3(1, -5, 0);
    state.player->savedPoint = glm::vec3(1, -5, 0);
    state.player->acceleration = glm::vec3(0, -9.81, 0);
    state.player->textureID = Util::LoadTexture("resources/george_0.png");

    state.player->speed = 2.0;
    state.player->jumpPower = 5.5f;
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

void Level1::InitEnemies() {
    state.enemies = new Entity[L1_ENEMY_COUNT];
    GLuint enemy_TID = Util::LoadTexture("resources/ken.png");

    for (int i = 0; i < L1_ENEMY_COUNT; i++) {
        state.enemies[i].textureID = enemy_TID;
        state.enemies[i].acceleration = glm::vec3(0, -9.81, 0);
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].width = 0.8f;
        state.enemies[i].speed = 0.7f;
    }
    // Wait and Go AI
    state.enemies[0].position = glm::vec3(8, -4, 0);
    state.enemies[0].aiType = WAITANDGO;
    state.enemies[0].aiState = IDLE;
}

void Level1::InitMusic() {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    state.music = Mix_LoadMUS("resources/wholesome.mp3");
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
}
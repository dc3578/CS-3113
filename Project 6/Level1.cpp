#include "Level1.h"
#include <vector>

#define L1_ENEMY_COUNT 12
#define L1_COIN_COUNT 15

//exit at 7,-3
#define L1_WIDTH 25
#define L1_HEIGHT 25
unsigned int b = 86;
unsigned int w = 87;
unsigned int s = 39;
unsigned int level1_data[] =
{
    b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b,
    b, w, w, w, w, 0, 0, 0, 0, 0, 0, 0, 0, w, w, 0, 0, 0, 0, 0, 0, 0, 0, 0, b,
    b, 0, 0, 0, 0, 0, w, w, w, w, w, w, 0, w, w, 0, 0, 0, 0, 0, 0, 0, w, 0, b,
    b, 0, 0, 0, 0, 0, w, w, 0, s, 0, w, 0, w, w, 0, 0, 0, 0, w, w, 0, 0, 0, b,
    b, 0, 0, 0, 0, 0, w, w, 0, w, 0, w, 0, w, w, w, w, w, w, 0, w, w, w, 0, b,
    b, 0, 0, 0, 0, 0, w, w, 0, w, 0, w, 0, w, w, 0, 0, 0, 0, 0, 0, 0, w, 0, b,
    b, 0, 0, 0, 0, 0, w, w, 0, 0, w, w, 0, w, 0, 0, 0, 0, 0, 0, w, 0, w, 0, b,
    b, w, 0, w, w, w, w, w, 0, w, 0, w, 0, w, w, 0, 0, 0, 0, 0, w, 0, 0, 0, b,
    b, w, 0, 0, 0, 0, 0, 0, 0, 0, 0, w, 0, w, w, w, 0, w, w, w, 0, 0, 0, 0, b,
    b, w, w, w, 0, 0, 0, 0, 0, w, 0, w, 0, 0, 0, w, 0, 0, 0, w, 0, 0, 0, 0, b,
    b, w, w, w, 0, 0, 0, 0, 0, w, 0, w, 0, 0, 0, w, 0, 0, 0, w, w, w, w, 0, b,
    b, w, w, w, w, w, w, w, w, w, 0, w, 0, 0, 0, w, 0, 0, 0, w, 0, 0, 0, 0, b,
    b, 0, 0, 0, 0, 0, 0, 0, 0, w, 0, w, w, w, 0, w, 0, 0, 0, w, 0, w, w, w, b,
    b, 0, w, w, w, w, 0, 0, 0, w, 0, w, 0, w, 0, 0, 0, 0, 0, w, 0, w, w, w, b,
    b, 0, w, w, w, w, 0, 0, 0, 0, 0, 0, 0, w, w, w, w, w, w, w, 0, 0, 0, 0, b,
    b, 0, w, w, w, w, w, w, w, w, w, w, 0, w, w, w, w, w, w, w, w, w, w, 0, b,
    b, 0, 0, 0, 0, 0, 0, w, w, w, 0, 0, 0, w, 0, 0, 0, 0, w, w, 0, 0, 0, 0, b,
    b, 0, w, 0, 0, 0, 0, w, 0, w, 0, 0, 0, w, 0, w, w, w, w, w, 0, w, w, 0, b,
    b, 0, w, 0, 0, 0, 0, 0, 0, w, 0, 0, 0, w, 0, 0, 0, 0, 0, 0, 0, w, w, w, b,
    b, 0, w, w, w, 0, w, w, 0, w, 0, 0, 0, w, 0, 0, 0, w, w, w, w, w, w, 0, b,
    b, 0, w, w, 0, w, w, w, 0, 0, 0, 0, 0, w, 0, 0, 0, w, w, w, w, w, w, 0, b,
    b, 0, 0, 0, 0, w, w, w, w, 0, w, w, w, 0, 0, 0, 0, 0, 0, w, 0, 0, 0, 0, b,
    b, 0, 0, w, 0, w, w, w, w, 0, w, w, w, w, w, w, w, w, 0, w, 0, w, w, w, b,
    b, 0, 0, w, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, w, w, w, b,
    b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b,
};


GLuint font_TID;
GLuint tilesetID;
bool win = false;
bool lose = false;
std::string message;
std::vector<glm::vec3> enemyCords;
std::vector<glm::vec3> coinCords;

void Level1::Initialize() {
    tilesetID = Util::LoadTexture("resources/sokoban_tilesheet.png");
    font_TID = Util::LoadTexture("resources/font.png");
    state.map = new Map(L1_WIDTH, L1_HEIGHT, level1_data, tilesetID, 1.0f, 13, 8);
    state.nextScene = -1;
    // initialization of things
    InitMusic();
    InitPlayer();
    InitEnemies();
    InitCoins();
}

void Level1::Update(float deltaTime) {
    state.player->Update(deltaTime, state.player, state.enemies, L1_ENEMY_COUNT, state.map);

    // update enemies 
    for (int i = 0; i < L1_ENEMY_COUNT; i++) {
        state.enemies[i].Update(deltaTime, state.player, state.enemies, L1_ENEMY_COUNT, state.map);
    }

    // update coins 
    for (int i = 0; i < L1_COIN_COUNT; i++) {
        state.coins[i].Update(deltaTime, state.player, state.coins, L1_COIN_COUNT, state.map);
    }

    int loc = int(L1_WIDTH * ceil(-state.player->position.y) + floor(state.player->position.x));
    if (level1_data[loc] == s && state.player->coins == L1_COIN_COUNT) {
        win = true;
    }
    if (state.player->lives <= 0) {
        lose = true;
    }
}

void Level1::Render(ShaderProgram* program) {
    state.map->Render(program);
    state.player->Render(program);
    //state.exit->Render(program);

    // render enemies 
    for (int i = 0; i < L1_ENEMY_COUNT; i++) {
        state.enemies[i].Render(program);
    }

    // render coins 
    for (int i = 0; i < L1_COIN_COUNT; i++) {
        state.coins[i].Render(program);
    }

    // render lives
    float x = state.player->position.x - 0.25f;
    float y = state.player->position.y + 0.5f;
    Util::DrawText(program, font_TID, "Lives: " + std::to_string(state.player->lives), 0.25f, -0.15f, glm::vec3(x, y, 0));

    // render coins left to collect
    x = state.player->position.x - 0.5f;
    y = state.player->position.y - 0.5f;
    Util::DrawText(program, font_TID, "Coins Left: " + std::to_string(L1_COIN_COUNT - state.player->coins), 0.25f, -0.15f, glm::vec3(x, y, 0));
    
    if (win) {
        message = "You Win!";
    }
    else if (lose) {
        message = "You Lose!";
    }
    if (win || lose) {
        float xpos = state.player->position.x;
        float ypos = state.player->position.y + 2;
        Util::DrawText(program, font_TID, message, 0.5f, -0.25f, glm::vec3(xpos, ypos, 0));
        Mix_HaltMusic();
        state.gameover = true;

        //stop all enemy movement
        for (int i = 0; i < L1_ENEMY_COUNT; i++) {
            state.enemies[i].velocity = glm::vec3(0);
            state.enemies[i].movement = glm::vec3(0);
        }
    }
}

void Level1::InitPlayer() {
    state.player = new Entity();
    state.player->entityType = PLAYER;

    state.player->position = glm::vec3(1, -23, 0);
    state.player->savedPoint = glm::vec3(1, -23, 0);
    state.player->textureID = Util::LoadTexture("resources/assets/player.png");
    state.player->bumpSound = Mix_LoadWAV("resources/sounds/bump.wav");
    Mix_VolumeChunk(state.player->bumpSound, MIX_MAX_VOLUME / 5);

    state.player->minMapHeight = -L1_HEIGHT;

    state.player->speed = 2.0;
    state.player->width = 0.8f;
    state.player->height = 1.0f;


    state.player->animRight = new int[4]{ 4, 3, 4, 5 };
    state.player->animLeft = new int[4]{ 10, 9, 10, 11 };
    state.player->animUp = new int[4]{ 7, 6, 7, 8 };
    state.player->animDown = new int[4]{ 1, 0, 1, 2 };

    state.player->animRows = 4;
    state.player->animCols = 3;

    state.player->animIndices = state.player->animRight;
    state.player->animFrames = 4;
    state.player->animIndex = 0;
    state.player->animTime = 0;
}

void Level1::CreateEnemyCords() {
    enemyCords.push_back(glm::vec3(7, -12, 0));
    enemyCords.push_back(glm::vec3(11, -18, 0));
    enemyCords.push_back(glm::vec3(6, -9, 0));
    enemyCords.push_back(glm::vec3(16, -6, 0));
    enemyCords.push_back(glm::vec3(14, -19, 0));
    enemyCords.push_back(glm::vec3(4, -17, 0));
    enemyCords.push_back(glm::vec3(13, -10, 0));
    enemyCords.push_back(glm::vec3(17, -11, 0));
}

void Level1::InitEnemies() {
    state.enemies = new Entity[L1_ENEMY_COUNT];
    GLuint enemy_TID = Util::LoadTexture("resources/assets/spike_idle.png");
    GLuint walk_TID = Util::LoadTexture("resources/assets/enemy_walk.png");
    GLuint wait_TID = Util::LoadTexture("resources/assets/spike_moving.png");
    GLuint fly_TID = Util::LoadTexture("resources/assets/enemy_flying.png");
    
    CreateEnemyCords();
    for (int i = 0; i < L1_ENEMY_COUNT; i++) {
        if (i < 8) {

            state.enemies[i].textureID = enemy_TID;
            state.enemies[i].position = enemyCords[i];
        }
        state.enemies[i].entityType = ENEMY;
        state.enemies[i].speed = 1.0f;
    }
    // Moving Ai
    // Left and Right Patrollers
    state.enemies[8].textureID = walk_TID;
    state.enemies[8].position = glm::vec3(12, -23, 0);
    state.enemies[8].aiType = PATROLER;
    state.enemies[8].aiState = XDIR;
    // Wait And Go
    state.enemies[9].textureID = wait_TID;
    state.enemies[9].position = glm::vec3(8, -18, 0);
    state.enemies[9].aiType = WAITANDGO;
    state.enemies[9].aiState = IDLE;
    // Up and Down
    state.enemies[10].textureID = fly_TID;
    state.enemies[10].position = glm::vec3(4, -4, 0);
    state.enemies[10].aiType = PATROLER;
    state.enemies[10].aiState = YDIR;
    // Left and Right
    state.enemies[11].textureID = fly_TID;
    state.enemies[11].position = glm::vec3(21, -5, 0);
    state.enemies[11].aiType = PATROLER;
    state.enemies[11].aiState = XDIR;
}

void Level1::CreateCoinCords() {
    coinCords.push_back(glm::vec3(3, -4, 0));
    coinCords.push_back(glm::vec3(3, -17, 0));
    coinCords.push_back(glm::vec3(4, -21, 0));
    coinCords.push_back(glm::vec3(5, -9, 0));
    coinCords.push_back(glm::vec3(10, -16, 0));
    coinCords.push_back(glm::vec3(13, -21, 0));
    coinCords.push_back(glm::vec3(15, -1, 0));
    coinCords.push_back(glm::vec3(17, -10, 0));
    coinCords.push_back(glm::vec3(19, -4, 0));
    coinCords.push_back(glm::vec3(23, -19, 0));
    coinCords.push_back(glm::vec3(14, -6, 0));
    coinCords.push_back(glm::vec3(9, -22, 0));
    coinCords.push_back(glm::vec3(14, -11, 0));
    coinCords.push_back(glm::vec3(15, -3, 0));
    coinCords.push_back(glm::vec3(18, -3, 0));
}

void Level1::InitCoins() {
    state.coins = new Entity[L1_COIN_COUNT];
    GLuint coin_TID = Util::LoadTexture("resources/assets/coin.png");

    CreateCoinCords();
    for (int i = 0; i < L1_COIN_COUNT; i++) {
        state.coins[i].textureID = coin_TID;
        state.coins[i].entityType = COIN;
        state.coins[i].sfx = Mix_LoadWAV("resources/sounds/coin.wav"); 
        state.coins[i].position = coinCords[i];
    }
}


void Level1::InitMusic() {
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    state.music = Mix_LoadMUS("resources/sounds/SCP-x7x.mp3");
    Mix_VolumeMusic(MIX_MAX_VOLUME / 3);
}
#include "MenuScene.h"

#define TITLE "Jumper"
#define MENUTEXT "Please Press Enter To Play"

void MenuScene::Initialize() {
    state.nextScene = -1;
    startgame = false;
    fontTextureID = Util::LoadTexture("resources/font1.png");
}

void MenuScene::Render(ShaderProgram* program) {
    Util::DrawText(program, fontTextureID, TITLE, 0.5, -0.25, glm::vec3(-0.75, 0.5, 0));
    Util::DrawText(program, fontTextureID, MENUTEXT, 0.5, -0.25, glm::vec3(-2.25, 0, 0));
}

void MenuScene::Update(float deltaTime) {}
void MenuScene::InitPlayer() {}
void MenuScene::InitEnemies() {}
void MenuScene::InitMusic() {}

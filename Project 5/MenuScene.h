#include "Scene.h"
class MenuScene : public Scene {

public:
	GLuint fontTextureID;
	bool startgame = false;

	void Initialize() override;
	void Update(float deltaTime) override;
	void Render(ShaderProgram* program) override;
	void InitPlayer() override;
	void InitEnemies() override;
	void InitMusic() override;
};
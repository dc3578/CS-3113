#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"

SDL_Window* displayWindow;
bool gameIsRunning = true;
ShaderProgram program;

glm::mat4 viewMatrix, projectionMatrix, p1_matrix, p2_matrix, pong_matrix;
glm::vec3 p1_position, p2_position, pong_position, p1_movement, p2_movement, pong_movement;
int width = 640;
int height = 480;
float lastTicks = 0.0f;
float paddle_speed = 2.0f;
float pong_speed = 2.5f;
bool startgame = false;
bool gameover = false;
int xdir = 1;
int ydir = 1;


void Initialize() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Project 2 - Pong", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, width, height);
	program.Load("shaders/vertex.glsl", "shaders/fragment.glsl");
	viewMatrix = glm::mat4(1.0f);
	p1_matrix = glm::mat4(1.0f);
	p2_matrix = glm::mat4(1.0f);
	pong_matrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
	
	// set up players and pong locations
	p1_position = glm::vec3(-4.5f, 0.0f, 0.0f);
	p2_position = glm::vec3(4.5f, 0.0f, 0.0f);
	pong_position = glm::vec3(0.0f, 0.0f, 0.0f);
	// set up pong movements
	pong_movement = glm::vec3(0.5f, -1.0f, 0.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	program.SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	glUseProgram(program.programID);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
}

void ProcessInput() {
	// reset movements
	p1_movement = glm::vec3(0);
	p2_movement = glm::vec3(0);

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
				// starts the game
				startgame = true;
			}
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	// allow paddle movement until the game is over
	if (!gameover) {
		// player 1 buttons
		if (keys[SDL_SCANCODE_W]) {
			p1_movement.y = 1.0f;
		}
		else if (keys[SDL_SCANCODE_S]) {
			p1_movement.y = -1.0f;
		}
		if (glm::length(p1_movement) > 1.0f) {
			p1_movement = glm::normalize(p1_movement);
		}

		// player 2 buttons
		if (keys[SDL_SCANCODE_UP]) {
			p2_movement.y = 1.0f;
		}
		else if (keys[SDL_SCANCODE_DOWN]) {
			p2_movement.y = -1.0f;
		}
		if (glm::length(p2_movement) > 1.0f) {
			p2_movement = glm::normalize(p2_movement);
		}
	}
}

void p1Update(float delta) {
	p1_position += p1_movement * paddle_speed * delta;
	if (p1_position.y < 2.75 && p1_position.y > -2.75) {
		p1_matrix = glm::mat4(1.0f);
		p1_matrix = glm::translate(p1_matrix, p1_position);
	}
	else {
		p1_position -= p1_movement * paddle_speed * delta;
	}
}

void p2Update(float delta) {
	p2_position += p2_movement * paddle_speed * delta;
	if (p2_position.y < 2.75 && p2_position.y > -2.75) {
		p2_matrix = glm::mat4(1.0f);
		p2_matrix = glm::translate(p2_matrix, p2_position);
	}
	else {
		p2_position -= p2_movement * paddle_speed * delta;
	}

}

bool isColliding(glm::vec3 player_position, glm::vec3 pong_position) {
	float w1 = 0.4f;
	float h1 = 0.4f;
	float w2 = 0.4f;
	float h2 = 2.0f;
	float xdist = fabs(player_position.x - pong_position.x) - ((w1 + w2) / 2.0f);
	float ydist = fabs(player_position.y - pong_position.y) - ((h1 + h2) / 2.0f);
	return xdist < 0 && ydist < 0;
}

void CollisionUpdate() {
	bool leftCollide = isColliding(p1_position, pong_position);
	bool rightCollide = isColliding(p2_position, pong_position);
	// check if collide with left player
	if (leftCollide) {
		pong_movement.x = 1.0f;
	}
	// check if collide with right player
	if (rightCollide) {
		pong_movement.x = -1.0f;
	}
	// check if collide with top of window
	if (pong_position.y >= 3.5) {
		pong_movement.y = -1.0f;
	}
	// check if collide with bottom of window
	if (pong_position.y <= -3.5) {
		pong_movement.y = 1.0f;
	}
	// check if passed either player
	if (pong_position.x <= -4.5 || pong_position.x >= 4.5) {
		// stops the game
		startgame = false;
		gameover = true;
	}
}

void pongUpdate(float delta) {
	if (startgame) {
		pong_position += pong_movement * pong_speed * delta;
		CollisionUpdate();
		pong_matrix = glm::mat4(1.0f);
		pong_matrix = glm::translate(pong_matrix, pong_position);
	}
}

void Update() {
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float delta = ticks - lastTicks;
	lastTicks = ticks;
	p1Update(delta);
	p2Update(delta);
	pongUpdate(delta);
}

void DrawObject(glm::mat4 matrix, float vertices[]) {
	program.SetModelMatrix(matrix);
	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Render() {
	glClear(GL_COLOR_BUFFER_BIT);
	// pong
	float pong_vertices[] = {-0.2, -0.2, 0.2, -0.2, 0.2, 0.2, -0.2, -0.2, 0.2, 0.2, -0.2, 0.2};
	DrawObject(pong_matrix, pong_vertices);
	// paddles
	float paddle_vertices[] = { -0.2, -1.0, 0.2, -1.0, 0.2, 1.0, -0.2, -1.0, -0.2, 1.0, 0.2, 1.0 };
	DrawObject(p1_matrix, paddle_vertices);
	DrawObject(p2_matrix, paddle_vertices);

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
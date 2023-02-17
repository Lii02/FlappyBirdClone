#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include <SDL2/SDL.h>
#undef main
#include <SDL2/SDL_image.h>

typedef std::chrono::high_resolution_clock::time_point TimePoint;
#define UNIT_X 64
#define UNIT_Y 64

struct Vector2 {
	float x;
	float y;

	Vector2() {
		this->x = 0;
		this->y = 0;
	}

	Vector2(float v) {
		this->x = v;
		this->y = v;
	}

	Vector2(float x, float y) {
		this->x = x;
		this->y = y;
	}

	Vector2 operator-(Vector2 right) {
		return Vector2(x - right.x, y - right.y);
	}
	
	Vector2 operator+(Vector2 right) {
		return Vector2(x + right.x, y + right.y);
	}

	Vector2 operator/(Vector2 right) {
		return Vector2(x / right.x, y / right.y);
	}
};

struct Rectangle {
	Vector2 position, size;

	Rectangle() {
		position = { 0, 0 };
		size = { 1, 1 };
	}

	Rectangle(Vector2 position, Vector2 size) {
		this->position = position;
		this->size = size;
	}
};

namespace {
	SDL_Renderer* renderer;
	Vector2 cameraPosition;
}

class Sprite {
private:
	SDL_Texture* texture;
public:
	Rectangle rectangle;
	float angle;

	Sprite(std::string filename, Rectangle rectangle, float angle) {
		this->rectangle = rectangle;
		this->angle = angle;
		SDL_Surface* surface = IMG_Load(filename.c_str());
		texture = SDL_CreateTextureFromSurface(renderer, surface);
		SDL_FreeSurface(surface);
		SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	}

	~Sprite() {
		SDL_DestroyTexture(texture);
	}

	void Draw() {
		SDL_Rect r = { UNIT_X * rectangle.position.x - (UNIT_X * cameraPosition.x), UNIT_Y * rectangle.position.y + (UNIT_Y * cameraPosition.y), UNIT_X * rectangle.size.x, UNIT_Y * rectangle.size.y };
		SDL_RenderCopyEx(renderer, texture, NULL, &r, angle, NULL, SDL_FLIP_NONE);
	}
};

bool IsColliding(Rectangle left, Rectangle right) {
	Vector2 leftMin = left.position - left.size / 2.0f;
	Vector2 leftMax = left.position + left.size / 2.0f;
	Vector2 rightMin = right.position - right.size / 2.0f;
	Vector2 rightMax = right.position + right.size / 2.0f;
	if (leftMin.x < rightMax.x && leftMax.x > rightMin.x &&
		leftMin.y < rightMax.y && leftMax.y > rightMin.y)
		return true;
	else
		return false;
}

#define PIPE_HEIGHT 11

class Pipe {
private:
	int entrance;
	float x;
	Sprite* spr;
public:
	Pipe() {
		spr = new Sprite("pipe.png", Rectangle(), 0.0f);
		entrance = rand() % PIPE_HEIGHT;
	}

	~Pipe() {
		delete spr;
	}

	void Draw() {
		for (int i = 0; i < PIPE_HEIGHT; i++) {
			if (i == entrance || i == entrance + 1)
				continue;
			spr->rectangle.position = Vector2(x + cameraPosition.x, i + cameraPosition.y);
			spr->Draw();
		}
	}
};

int main() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	srand(time(NULL));
	SDL_Window* window = SDL_CreateWindow("Flappy Bird Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 832, 832, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	bool running = true;

	const float playerStartY = 5.75f;
	Sprite* sprite = new Sprite("ship.png", Rectangle({ 0.0f, playerStartY }, { 2, 1 }), 0.0f);
	Sprite* floor = new Sprite("floor.png", Rectangle(), 0);
	
	TimePoint begin, end;
	double delta = 0.0;

	while (running) {
		begin = std::chrono::high_resolution_clock::now();
		float dt = static_cast<float>(delta / 1000);

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				running = false;
				break;
			}
		}
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

		for (int x = 0; x < 13; x++) {
			for (int y = PIPE_HEIGHT; y < 15; y++) {
				floor->rectangle.position = { (float)x, (float)y };
				floor->Draw();
			}
		}

		sprite->Draw();

		SDL_RenderPresent(renderer);
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> deltaDuration = end - begin;
		delta = deltaDuration.count();
	}

	delete floor;
	delete sprite;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
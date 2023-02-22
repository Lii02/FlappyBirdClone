#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <queue>
#include <cstdint>
#include <SDL2/SDL.h>
#undef main
#include <SDL2/SDL_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

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

	Vector2 operator+=(Vector2 right) {
		return Vector2(x += right.x, y += right.y);
	}

	Vector2 operator/(Vector2 right) {
		return Vector2(x / right.x, y / right.y);
	}

	Vector2 operator*(Vector2 right) {
		return Vector2(x * right.x, y * right.y);
	}

	Vector2 operator*=(Vector2 right) {
		return Vector2(x *= right.x, y *= right.y);
	}

	friend std::ostream& operator<<(std::ostream& os, const Vector2& vec) {
		os << vec.x;
		os << ' ';
		os << vec.y;
		return os;
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

	Rectangle(float v) {
		this->position = v;
		this->size = v;
	}

	bool Contains(Rectangle with) {
		if (position.x < with.position.x + with.size.x &&
			position.x + size.x > with.position.x &&
			position.y < with.position.y + with.size.y &&
			position.y + size.y > with.position.y)
			return true;
		else
			return false;
	}

	friend std::ostream& operator<<(std::ostream& os, const Rectangle& rect) {
		os << rect.position;
		os << ", ";
		os << rect.size;
		return os;
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

#define PIPE_HEIGHT 11

class Pipe {
private:
	int entrance;
	Sprite* spr;
public:
	Rectangle top, bottom;
	float x;

	Pipe(float x) {
		this->x = x;
		spr = new Sprite("assets/pipe.png", Rectangle(), 0.0f);
		entrance = rand() % (PIPE_HEIGHT - 2);
		bool isBottom = false;
		top.position = bottom.position = Vector2(x, -1);
		for (int i = 0; i < PIPE_HEIGHT; i++) {
			if (i == entrance || i == entrance + 1 || i == entrance + 2) {
				isBottom = true;
				bottom.position.y = i + 1;
				continue;
			}
			top.size.x = bottom.size.x = 1;
			if (isBottom)
				bottom.size.y++;
			else
				top.size.y++;
		}
		std::swap(top, bottom);
	}

	~Pipe() {
		delete spr;
	}

	void Draw() {
		for (int i = 0; i < PIPE_HEIGHT; i++) {
			if (i == entrance || i == entrance + 1 || i == entrance + 2)
				continue;
			spr->rectangle.position = Vector2(x, i);
			spr->Draw();
		}
	}

	bool IsColliding(Rectangle playerBox) {
		return playerBox.Contains(top) || playerBox.Contains(bottom);
	}
};

int main() {
	SDL_Init(SDL_INIT_EVERYTHING);
	IMG_Init(IMG_INIT_PNG);
	srand(time(NULL));
	SDL_Window* window = SDL_CreateWindow("Flappy Bird Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 832, 832, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	bool running = true;

	bool playing = false;
	const float moveSpeed = 2.75f;
	const float jumpForce = 3000.0f;
	constexpr float gravity = (UNIT_Y / -9.8f);
	const Vector2 startPosition = Vector2(5.8f, 5.75f);
	Sprite* player = new Sprite("assets/ship.png", Rectangle(startPosition, { 0.75f, 0.75f }), 0.0f);
	Sprite* floor = new Sprite("assets/floor.png", Rectangle(), 0);
	Vector2 velocity;
	Vector2 acceleration = Vector2(0, 0);
	std::vector<Pipe*> pipes;

	bool keys[0xFF] = { 0 };
	bool lastKeys[0xFF] = { 0 };
	TimePoint begin, end;
	double delta = 0.0;
	float pipeX = 0;
	bool gameOver = false;

	auto Reset = [&]() {
		for (Pipe* p : pipes) {
			delete p;
		}
		pipes.clear();
		player->rectangle.position = startPosition;
		cameraPosition = 0;
		pipeX = 0;
		playing = true;
		std::cout << "Reset" << std::endl;
	};

	while (running) {
		begin = std::chrono::high_resolution_clock::now();
		float dt = static_cast<float>(delta / 1000);
		velocity.y = 0;

		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_QUIT:
				running = false;
				break;
			case SDL_KEYDOWN:
				keys[ev.key.keysym.sym] = true;
				break;
			case SDL_KEYUP:
				keys[ev.key.keysym.sym] = false;
				break;
			}
		}
		SDL_RenderClear(renderer);
		SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

		if (!playing && keys[SDLK_SPACE] && !lastKeys[SDLK_SPACE]) {
			Reset();
		}

		Rectangle groundBox = Rectangle(Vector2(cameraPosition.x, PIPE_HEIGHT), Vector2(13, 1));
		for (int x = cameraPosition.x; x < cameraPosition.x + 13; x++) {
			for (int y = PIPE_HEIGHT; y < 15; y++) {
				floor->rectangle.position = { (float)x, (float)y };
				floor->rectangle.size = 1;
				floor->Draw();
			}
		}

		if (playing) {
			if (!pipes.size()) {
				int counter = 0;
				while (counter < 10) {
					pipeX += 8;
					pipes.push_back(new Pipe(player->rectangle.position.x + counter + pipeX));
					counter++;
				}
				pipeX = 0;
			}

			player->rectangle.position.x += dt * moveSpeed;
			player->Draw();
			cameraPosition.x += dt * moveSpeed;
			acceleration.y -= gravity;

			velocity += acceleration * dt;
			if (keys[SDLK_SPACE] && !lastKeys[SDLK_SPACE]) {
				acceleration = 0;
				acceleration.y -= jumpForce;
			}
			player->rectangle.position += velocity * dt;

			if (player->rectangle.Contains(groundBox)) {
				std::cout << "You Died!" << std::endl;
				playing = false;
			}

			for (size_t i = 0; i < pipes.size(); i++) {
				Pipe* p = pipes[i];
				p->Draw();

				if (p->IsColliding(player->rectangle)) {
					std::cout << "You Died!" << std::endl;
					playing = false;
				}

				if (p->x < cameraPosition.x - 2) {
					delete p;
					pipes.erase(pipes.begin() + i);
				}
			}
		}

		SDL_RenderPresent(renderer);
		end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> deltaDuration = end - begin;
		delta = deltaDuration.count();
		memcpy(lastKeys, keys, 0xFF);
	}

	delete floor;
	delete player;
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	IMG_Quit();
	SDL_Quit();
	return 0;
}
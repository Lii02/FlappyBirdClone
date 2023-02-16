#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cstdint>
#include <SDL2/SDL.h>
#undef main
#include <SDL2/SDL_image.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace {
    SDL_Renderer* renderer;
}

struct Vector2 {
    float x;
    float y;
};

class Rectangle {
public:
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
        SDL_Rect r = { rectangle.position.x, rectangle.position.y, rectangle.size.x, rectangle.size.y };
        SDL_RenderCopyEx(renderer, texture, NULL, &r, angle, NULL, SDL_FLIP_NONE);
    }
};

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);
    SDL_Window* window = SDL_CreateWindow("Flappy Bird Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 800, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    bool running = true;

    Sprite* sprite = new Sprite("ship.png", Rectangle({ 0.0f, 0.0f }, { 128, 64 }), 0.0f);

    while (running) {
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

        sprite->Draw();

        SDL_RenderPresent(renderer);
    }

    delete sprite;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}
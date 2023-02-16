#include <iostream>
#include <vector>
#include <string>
#include <SDL2/SDL.h>
#undef main
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

class Vector2 {
public:
    float x, y;
};

class Rectangle {
public:
    Vector2 position, size;
};

class Sprite {
private:
    SDL_Texture* tex;
public:
    Sprite() {

    }

    ~Sprite() {

    }
};

int main() {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Renderer* renderer;
    SDL_Window* window;
    SDL_CreateWindowAndRenderer(800, 800, SDL_WINDOW_SHOWN, &window, &renderer);
    SDL_SetWindowTitle(window, "Flappy Bird Clone");
    bool running = true;

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

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
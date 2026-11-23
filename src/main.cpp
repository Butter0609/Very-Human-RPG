#include <SDL3/SDL.h>
#include <iostream>
#include "cardClass.h"
#include "animations.h"

int main() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        cerr << "SDL_Init failed: " << SDL_GetError() << '\n';
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("My Game", 800, 600, 0);

    if (!window) {
        cerr << "Could not create window: " << SDL_GetError() << '\n';
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    if (!renderer) {
        cerr << "Could not create renderer: " << SDL_GetError() << '\n';
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const char* basePath = SDL_GetBasePath();
    if (!basePath) {
        cerr << "Couldn't locate executable: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    const string spritePath = string(basePath) + "assets/playerSprite.bmp";

    SDL_Surface* surface = SDL_LoadBMP(spritePath.c_str());
    if (!surface) {
        cerr << "Couldn't load BMP: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* sprite = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);

    if (!sprite) {
        cerr << "Couldn't create texture: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Event event;
    bool running = true;
    const SDL_FRect spriteRect = {100.0f, 100.0f, 64.0f, 64.0f};

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        int uin = quick_read();
        if (uin == ERR) {
            ;
        } else if (uin == w) {
            cout << "w" << endl;
        } else if (uin == a) {
            cout << "a" << endl;
        } else if (uin == s) {
            cout << "s" << endl;
        } else if (uin == d) {
            cout << "d" << endl;
        }
        SDL_SetRenderDrawColor(renderer, 25, 30, 45, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, sprite, nullptr, &spriteRect);
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(sprite);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

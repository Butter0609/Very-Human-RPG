#pragma once

#include <SDL3/SDL.h>
class spritesheet {
    public: 
        SDL_Texture* texture;
        int frameWidth;
        int frameHeight;
        int numFrames;
        int currentFrame;
        int frameDuration; // Duration of each frame in milliseconds
        Uint32 lastFrameTime; // Time when the last frame was updated

        spritesheet(SDL_Texture* tex, int fWidth, int fHeight, int nFrames, int fDuration)
            : texture(tex), frameWidth(fWidth), frameHeight(fHeight), numFrames(nFrames),
              currentFrame(0), frameDuration(fDuration), lastFrameTime(0) {}

        void update() {
            Uint32 currentTime = SDL_GetTicks();
            if (currentTime - lastFrameTime >= frameDuration) {
                currentFrame = (currentFrame + 1) % numFrames;
                lastFrameTime = currentTime;
            }
        }

        void render(SDL_Renderer* renderer, int x, int y) {
            SDL_FRect srcRect = { static_cast<float>(currentFrame * frameWidth), 0.0f,
                                  static_cast<float>(frameWidth), static_cast<float>(frameHeight) };
            SDL_FRect destRect = { static_cast<float>(x), static_cast<float>(y),
                                   static_cast<float>(frameWidth), static_cast<float>(frameHeight) };
            SDL_RenderTexture(renderer, texture, &srcRect, &destRect);
        }
};
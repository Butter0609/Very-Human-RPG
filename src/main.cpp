#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include "cardClass.hpp"
#include "animations.hpp"

namespace {
enum class MenuMode { None, Inventory, Pause };

struct MenuButton {
    SDL_FRect bounds;
    const char* label;
};

struct Room {
    SDL_Texture* texture;
    SDL_FRect bounds;
    float scale;
    float width;
    float height;
    std::vector<SDL_FRect> obstacles;
};

SDL_Texture* createRoomTexture(SDL_Renderer* renderer, SDL_Surface* atlas,
                               const SDL_Rect& sourceRect) {
    SDL_Surface* roomSurface = SDL_CreateSurface(sourceRect.w, sourceRect.h,
                                                  SDL_PIXELFORMAT_RGBA32);
    if (!roomSurface || !SDL_BlitSurface(atlas, &sourceRect, roomSurface, nullptr)) {
        std::cerr << "Couldn't crop room from indoor map: " << SDL_GetError() << '\n';
        SDL_DestroySurface(roomSurface);
        return nullptr;
    }

    if (!SDL_LockSurface(roomSurface)) {
        std::cerr << "Couldn't lock room surface: " << SDL_GetError() << '\n';
        SDL_DestroySurface(roomSurface);
        return nullptr;
    }
    for (int y = 0; y < roomSurface->h; ++y) {
        auto* pixel = static_cast<Uint8*>(roomSurface->pixels) + y * roomSurface->pitch;
        for (int x = 0; x < roomSurface->w; ++x, pixel += 4) {
            const bool checkerBackground =
                (pixel[0] == 255 && pixel[1] == 196 && pixel[2] == 222) ||
                (pixel[0] == 142 && pixel[1] == 255 && pixel[2] == 146);
            const bool atlasPadding = pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0;
            if (checkerBackground || atlasPadding) {
                pixel[3] = 0;
            }
        }
    }
    SDL_UnlockSurface(roomSurface);

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, roomSurface);
    SDL_DestroySurface(roomSurface);
    if (!texture) {
        std::cerr << "Couldn't create room texture: " << SDL_GetError() << '\n';
        return nullptr;
    }
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    return texture;
}

SDL_FRect toScreenRect(const Room& room, const SDL_FRect& mapRect) {
    return {room.bounds.x + mapRect.x * room.scale,
            room.bounds.y + mapRect.y * room.scale,
            mapRect.w * room.scale, mapRect.h * room.scale};
}

bool overlaps(const SDL_FRect& first, const SDL_FRect& second) {
    return first.x < second.x + second.w && first.x + first.w > second.x &&
           first.y < second.y + second.h && first.y + first.h > second.y;
}

std::vector<MenuButton> getMenuButtons(MenuMode mode) {
    if (mode == MenuMode::Pause) {
        return {{{270.0f, 245.0f, 260.0f, 42.0f}, "Resume"},
                {{270.0f, 299.0f, 260.0f, 42.0f}, "Inventory"},
                {{270.0f, 353.0f, 260.0f, 42.0f}, "Quit"}};
    }
    if (mode == MenuMode::Inventory) {
        return {{{270.0f, 390.0f, 260.0f, 42.0f}, "Close"}};
    }
    return {};
}

bool contains(const SDL_FRect& rect, float x, float y) {
    return x >= rect.x && x <= rect.x + rect.w &&
           y >= rect.y && y <= rect.y + rect.h;
}

void drawButton(SDL_Renderer* renderer, const MenuButton& button, bool selected) {
    SDL_SetRenderDrawColor(renderer, selected ? 72 : 48, selected ? 116 : 62,
                           selected ? 124 : 78, 255);
    SDL_RenderFillRect(renderer, &button.bounds);
    SDL_SetRenderDrawColor(renderer, 180, 218, 210, 255);
    SDL_RenderRect(renderer, &button.bounds);
    SDL_RenderDebugText(renderer, button.bounds.x + 14.0f,
                        button.bounds.y + 16.0f, button.label);
}
}

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

    SDL_Surface* rgbaSurface = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
    SDL_DestroySurface(surface);
    if (!rgbaSurface) {
        cerr << "Couldn't convert sprite transparency: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!SDL_LockSurface(rgbaSurface)) {
        cerr << "Couldn't lock sprite surface: " << SDL_GetError() << '\n';
        SDL_DestroySurface(rgbaSurface);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    for (int y = 0; y < rgbaSurface->h; ++y) {
        auto* pixel = static_cast<Uint8*>(rgbaSurface->pixels) + y * rgbaSurface->pitch;
        for (int x = 0; x < rgbaSurface->w; ++x, pixel += 4) {
            const bool atlasBackground = pixel[0] == 115 && pixel[1] == 197 && pixel[2] == 165;
            const bool frameSeparator = pixel[0] == 64 && pixel[1] == 64 && pixel[2] == 64;
            if (atlasBackground || frameSeparator) {
                pixel[3] = 0;
            }
        }
    }
    SDL_UnlockSurface(rgbaSurface);

    SDL_Texture* sprite = SDL_CreateTextureFromSurface(renderer, rgbaSurface);
    SDL_DestroySurface(rgbaSurface);

    if (!sprite) {
        cerr << "Couldn't create texture: " << SDL_GetError() << '\n';
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    if (!SDL_SetTextureBlendMode(sprite, SDL_BLENDMODE_BLEND)) {
        cerr << "Couldn't enable sprite transparency: " << SDL_GetError() << '\n';
        SDL_DestroyTexture(sprite);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    if (!SDL_SetTextureScaleMode(sprite, SDL_SCALEMODE_NEAREST)) {
        cerr << "Couldn't set sprite sampling mode: " << SDL_GetError() << '\n';
        SDL_DestroyTexture(sprite);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    const string mapPath = string(basePath) + "assets/indoorMaps1.bmp";
    SDL_Surface* mapAtlas = SDL_LoadBMP(mapPath.c_str());
    if (!mapAtlas) {
        cerr << "Couldn't load indoor maps: " << SDL_GetError() << '\n';
        SDL_DestroyTexture(sprite);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    const SDL_Rect diningRoomCrop = {576, 24, 176, 144};
    const SDL_Rect bedroomCrop = {800, 24, 144, 144};
    SDL_Texture* diningRoomTexture = createRoomTexture(renderer, mapAtlas, diningRoomCrop);
    SDL_Texture* bedroomTexture = createRoomTexture(renderer, mapAtlas, bedroomCrop);
    SDL_DestroySurface(mapAtlas);
    if (!diningRoomTexture || !bedroomTexture) {
        SDL_DestroyTexture(diningRoomTexture);
        SDL_DestroyTexture(bedroomTexture);
        SDL_DestroyTexture(sprite);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Event event;
    bool running = true;
    constexpr float playerWidth = 52.0f;
    constexpr float playerHeight = 108.0f;
    constexpr float mapScale = 4.0f;
    Room diningRoom{diningRoomTexture,
                    {(800.0f - diningRoomCrop.w * mapScale) / 2.0f, 12.0f,
                     diningRoomCrop.w * mapScale, diningRoomCrop.h * mapScale},
                    mapScale, static_cast<float>(diningRoomCrop.w),
                    static_cast<float>(diningRoomCrop.h),
                    {{98.0f, 96.0f, 27.0f, 28.0f},
                     {80.0f, 96.0f, 15.0f, 17.0f},
                     {80.0f, 113.0f, 15.0f, 17.0f},
                     {129.0f, 96.0f, 15.0f, 17.0f},
                     {129.0f, 113.0f, 15.0f, 17.0f},
                     {97.0f, 59.0f, 16.0f, 27.0f},
                     {113.0f, 60.0f, 29.0f, 23.0f},
                     {0.0f, 42.0f, 16.0f, 4.0f},
                     {52.0f, 42.0f, 124.0f, 4.0f}}};
    Room bedroom{bedroomTexture,
                 {(800.0f - bedroomCrop.w * mapScale) / 2.0f, 12.0f,
                  bedroomCrop.w * mapScale, bedroomCrop.h * mapScale},
                 mapScale, static_cast<float>(bedroomCrop.w),
                 static_cast<float>(bedroomCrop.h),
                 {{108.0f, 78.0f, 18.0f, 32.0f},
                  {63.0f, 28.0f, 18.0f, 27.0f},
                  {83.0f, 39.0f, 12.0f, 16.0f},
                  {112.0f, 26.0f, 31.0f, 30.0f},
                  {0.0f, 42.0f, 12.0f, 4.0f},
                  {36.0f, 42.0f, 108.0f, 4.0f},
                  {130.0f, 48.0f, 14.0f, 15.0f}}};
    int currentRoom = 1;
    Room* activeRoom = &bedroom;
    float playerX = bedroom.bounds.x + 93.0f * mapScale;
    float playerY = bedroom.bounds.y + 75.0f * mapScale;
    constexpr float walkSpeed = 150.0f;
    constexpr float runSpeed = 270.0f;
    constexpr int frameWidth = 16;
    constexpr int frameHeight = 33;
    constexpr int frameColumnStride = 17;
    constexpr int walkStartX = 2;
    constexpr int runStartX = 56;
    int directionRow = 0;
    int animationFrame = 0;
    Uint64 lastFrameTime = SDL_GetTicks();
    Uint64 previousTime = lastFrameTime;
    MenuMode menuMode = MenuMode::None;
    int selectedButton = 0;
    float mouseX = -1.0f;
    float mouseY = -1.0f;
    std::string interactionMessage;
    Uint64 interactionMessageUntil = 0;
    bool doormatMessageShown = false;
    bool doorWalkActive = false;
    float doorWalkRemaining = 0.0f;
    int doorTargetRoom = 0;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            } else if (event.type == SDL_EVENT_MOUSE_MOTION) {
                mouseX = event.motion.x;
                mouseY = event.motion.y;
                const auto buttons = getMenuButtons(menuMode);
                for (int index = 0; index < static_cast<int>(buttons.size()); ++index) {
                    if (contains(buttons[index].bounds, mouseX, mouseY)) {
                        selectedButton = index;
                    }
                }
            } else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                       event.button.button == SDL_BUTTON_LEFT) {
                mouseX = event.button.x;
                mouseY = event.button.y;
                const auto buttons = getMenuButtons(menuMode);
                for (int index = 0; index < static_cast<int>(buttons.size()); ++index) {
                    if (!contains(buttons[index].bounds, mouseX, mouseY)) {
                        continue;
                    }
                    selectedButton = index;
                    if (menuMode == MenuMode::Pause) {
                        if (index == 0) menuMode = MenuMode::None;
                        else if (index == 1) menuMode = MenuMode::Inventory;
                        else running = false;
                    } else if (menuMode == MenuMode::Inventory) {
                        menuMode = MenuMode::None;
                    }
                    selectedButton = 0;
                    break;
                }
            } else if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
                const SDL_Keycode key = event.key.key;
                if (key == SDLK_ESCAPE) {
                    menuMode = menuMode == MenuMode::Pause ? MenuMode::None : MenuMode::Pause;
                    selectedButton = 0;
                } else if (key == SDLK_TAB) {
                    menuMode = menuMode == MenuMode::Inventory ? MenuMode::None : MenuMode::Inventory;
                    selectedButton = 0;
                } else if (key == SDLK_E && menuMode == MenuMode::None) {
                    const float localX = (playerX + playerWidth * 0.5f - activeRoom->bounds.x) /
                                         activeRoom->scale;
                    const float localY = (playerY + playerHeight - activeRoom->bounds.y) /
                                         activeRoom->scale;
                    if (currentRoom == 0 && localX >= 10.0f && localX <= 53.0f && localY >= 119.0f) {
                        interactionMessage = "This doorway is not connected yet.";
                    } else {
                        interactionMessage = "Nothing nearby to interact with.";
                    }
                    interactionMessageUntil = SDL_GetTicks() + 1800;
                } else if (menuMode != MenuMode::None &&
                           (key == SDLK_DOWN || key == SDLK_S || key == SDLK_UP || key == SDLK_W)) {
                    const auto buttons = getMenuButtons(menuMode);
                    if (!buttons.empty()) {
                        const int delta = (key == SDLK_DOWN || key == SDLK_S) ? 1 : -1;
                        selectedButton = (selectedButton + delta + static_cast<int>(buttons.size())) %
                                         static_cast<int>(buttons.size());
                    }
                } else if (menuMode != MenuMode::None &&
                           (key == SDLK_RETURN || key == SDLK_KP_ENTER || key == SDLK_SPACE)) {
                    if (menuMode == MenuMode::Pause) {
                        if (selectedButton == 0) menuMode = MenuMode::None;
                        else if (selectedButton == 1) menuMode = MenuMode::Inventory;
                        else running = false;
                    } else if (menuMode == MenuMode::Inventory) {
                        menuMode = MenuMode::None;
                    }
                    selectedButton = 0;
                }
            }
        }

        const Uint64 now = SDL_GetTicks();
        const float deltaTime = std::min(static_cast<float>(now - previousTime) / 1000.0f, 0.05f);
        previousTime = now;
        const bool* keys = SDL_GetKeyboardState(nullptr);
        const bool movingEnabled = menuMode == MenuMode::None && !doorWalkActive;
        float moveX = 0.0f;
        float moveY = 0.0f;
        if (movingEnabled) {
            if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) moveX -= 1.0f;
            if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) moveX += 1.0f;
            if (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W]) moveY -= 1.0f;
            if (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S]) moveY += 1.0f;
        }
        if (doorWalkActive) {
            moveY = -1.0f;
        }
        const bool isMoving = doorWalkActive || moveX != 0.0f || moveY != 0.0f;
        const bool isRunning = !doorWalkActive &&
                               (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]);
        if (isMoving) {
            const float length = std::sqrt(moveX * moveX + moveY * moveY);
            moveX /= length;
            moveY /= length;
            const float speed = doorWalkActive ? 44.0f : (isRunning ? runSpeed : walkSpeed);
            if (doorWalkActive) {
                playerY = std::max(activeRoom->bounds.y, playerY - speed * deltaTime);
                doorWalkRemaining -= deltaTime;
                if (doorWalkRemaining <= 0.0f) {
                    currentRoom = doorTargetRoom;
                    activeRoom = currentRoom == 0 ? &diningRoom : &bedroom;
                    if (currentRoom == 0) {
                        playerX = activeRoom->bounds.x + 30.0f * activeRoom->scale;
                        playerY = activeRoom->bounds.y + 43.0f * activeRoom->scale;
                    } else {
                        playerX = activeRoom->bounds.x + 17.0f * activeRoom->scale;
                        playerY = activeRoom->bounds.y + 51.0f * activeRoom->scale;
                    }
                    doorWalkActive = false;
                    doormatMessageShown = false;
                }
            } else {
            const float minX = activeRoom->bounds.x;
            const float maxX = activeRoom->bounds.x + activeRoom->bounds.w - playerWidth;
            const float minY = activeRoom->bounds.y;
            const float maxY = activeRoom->bounds.y + activeRoom->bounds.h - playerHeight;
            auto collidesAt = [&](float x, float y) {
                const SDL_FRect footBox = {x + playerWidth * 0.2f,
                                           y + playerHeight * 0.78f,
                                           playerWidth * 0.6f, playerHeight * 0.22f};
                for (const SDL_FRect& obstacle : activeRoom->obstacles) {
                    if (overlaps(footBox, toScreenRect(*activeRoom, obstacle))) {
                        return true;
                    }
                }
                return false;
            };
            const float nextX = std::clamp(playerX + moveX * speed * deltaTime, minX, maxX);
            if (!collidesAt(nextX, playerY)) {
                playerX = nextX;
            }
            const float nextY = std::clamp(playerY + moveY * speed * deltaTime, minY, maxY);
            if (!collidesAt(playerX, nextY)) {
                playerY = nextY;
            }

            const float playerCenterX = playerX + playerWidth * 0.5f;
            const float playerFootY = playerY + playerHeight;
            const float localCenterX = (playerCenterX - activeRoom->bounds.x) / activeRoom->scale;
            const float localFootY = (playerFootY - activeRoom->bounds.y) / activeRoom->scale;
            if (currentRoom == 1 && moveY < 0.0f && localCenterX >= 18.0f &&
                localCenterX <= 30.0f && localFootY <= 50.0f) {
                doorWalkActive = true;
                doorTargetRoom = 0;
                doorWalkRemaining = 0.75f;
            } else if (currentRoom == 0 && moveY < 0.0f && localCenterX >= 35.0f &&
                       localCenterX <= 47.0f && localFootY <= 50.0f) {
                doorWalkActive = true;
                doorTargetRoom = 1;
                doorWalkRemaining = 0.75f;
            }

            const float updatedLocalX = (playerX + playerWidth * 0.5f - activeRoom->bounds.x) /
                                        activeRoom->scale;
            const float updatedLocalY = (playerY + playerHeight - activeRoom->bounds.y) /
                                        activeRoom->scale;
            const bool onUnconnectedMat = currentRoom == 0 && updatedLocalX >= 10.0f &&
                                          updatedLocalX <= 53.0f && updatedLocalY >= 119.0f;
            if (onUnconnectedMat && !doormatMessageShown) {
                interactionMessage = "This doorway is not connected yet.";
                interactionMessageUntil = now + 2500;
                doormatMessageShown = true;
            } else if (!onUnconnectedMat) {
                doormatMessageShown = false;
            }
            }
            if (std::abs(moveY) >= std::abs(moveX)) directionRow = moveY < 0.0f ? 3 : 0;
            else directionRow = moveX < 0.0f ? 2 : 1;

            const Uint64 frameDuration = doorWalkActive ? 250 : (isRunning ? 90 : 150);
            if (now - lastFrameTime >= frameDuration) {
                animationFrame = (animationFrame + 1) % 3;
                lastFrameTime = now;
            }
        } else {
            animationFrame = 0;
            lastFrameTime = now;
        }

        SDL_SetRenderDrawColor(renderer, 25, 30, 45, 255);
        SDL_RenderClear(renderer);
        SDL_RenderTexture(renderer, activeRoom->texture, nullptr, &activeRoom->bounds);
        const int animationStartX = isRunning ? runStartX : walkStartX;
        const SDL_FRect sourceRect = {
            static_cast<float>(animationStartX + animationFrame * frameColumnStride),
            static_cast<float>(89 + directionRow * frameHeight),
            static_cast<float>(frameWidth), static_cast<float>(frameHeight)
        };
        const SDL_FRect playerRect = {playerX, playerY, playerWidth, playerHeight};
        SDL_RenderTexture(renderer, sprite, &sourceRect, &playerRect);

        if (menuMode != MenuMode::None) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
            const SDL_FRect shade = {0.0f, 0.0f, 800.0f, 600.0f};
            SDL_RenderFillRect(renderer, &shade);
            SDL_SetRenderDrawColor(renderer, 27, 38, 48, 255);
            const SDL_FRect panel = {240.0f, 170.0f, 320.0f, 300.0f};
            SDL_RenderFillRect(renderer, &panel);
            SDL_SetRenderDrawColor(renderer, 145, 192, 183, 255);
            SDL_RenderRect(renderer, &panel);
            SDL_SetRenderDrawColor(renderer, 225, 238, 222, 255);
            SDL_RenderDebugText(renderer, 270.0f, 198.0f,
                                menuMode == MenuMode::Pause ? "Paused" : "Inventory");
            if (menuMode == MenuMode::Inventory) {
                SDL_RenderDebugText(renderer, 270.0f, 240.0f, "No items yet.");
            }
            const auto buttons = getMenuButtons(menuMode);
            for (int index = 0; index < static_cast<int>(buttons.size()); ++index) {
                drawButton(renderer, buttons[index], index == selectedButton);
            }
        } else {
            SDL_SetRenderDrawColor(renderer, 225, 238, 222, 255);
            SDL_RenderDebugText(renderer, 12.0f, 12.0f, "WASD / Arrows: Move   Shift: Run");
            SDL_RenderDebugText(renderer, 12.0f, 28.0f, "Tab: Inventory   Esc: Pause   E: Interact");
            if (now < interactionMessageUntil) {
                SDL_RenderDebugText(renderer, 12.0f, 560.0f, interactionMessage.c_str());
            }
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(diningRoomTexture);
    SDL_DestroyTexture(bedroomTexture);
    SDL_DestroyTexture(sprite);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

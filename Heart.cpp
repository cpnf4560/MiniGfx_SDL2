#include "Heart.h"
#include <iostream>

Heart::Heart(int x, int y, int w, int h, int healthValue, int speedValue, SDL_Texture* tex)
    : health(healthValue), texture(tex), speed(speedValue) {
    position = Vec2D(x, y);
    rect = {x, y, w, h};
}

void Heart::update(float dt) {
    position.y += speed * dt;
    rect.y = static_cast<int>(position.y);
}

void Heart::render(SDL_Renderer* renderer, float x, float y) {
    SDL_Rect drawRect = rect;
    drawRect.x += (int)x;
    drawRect.y += (int)y;
    if (texture) {
        SDL_RenderCopy(renderer, texture, nullptr, &drawRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        SDL_RenderFillRect(renderer, &drawRect);
    }
}

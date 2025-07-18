#include "Bullet.h"
#include "GameManager.h"
#include <SDL.h>

Bullet::Bullet(int x, int y, int w, int h, int speed) : baseSpeed(speed) {
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    // Usa a velocidade do GameManager (que será maior que a base)
    currentSpeed = GameManager::getInstance().getBulletSpeed();
}

void Bullet::update(float dt) {
    // Atualiza velocidade dinamicamente
    currentSpeed = GameManager::getInstance().getBulletSpeed();

    // Move a bala para cima
    rect.y -= static_cast<int>(currentSpeed * dt);
}

void Bullet::render(SDL_Renderer* renderer, float x, float y) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // Amarelo
    SDL_RenderFillRect(renderer, &rect);
}

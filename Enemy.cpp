#include "Enemy.h"
#include "GameManager.h"
#include <SDL.h>

Enemy::Enemy(int x, int y, int w, int h, int speed, SDL_Renderer* renderer, std::vector<SDL_Texture*>& textures)
    : baseSpeed(speed), destroyed(false), gameRenderer(renderer), enemyTextures(&textures), textureIndex(0) {

    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    // Usa a velocidade do GameManager (que será maior que a base)
    currentSpeed = GameManager::getInstance().getEnemySpeed();
}

void Enemy::update(float dt) {
    // Atualiza velocidade dinamicamente
    currentSpeed = GameManager::getInstance().getEnemySpeed();

    // Move o inimigo para baixo
    rect.y += static_cast<int>(currentSpeed * dt);

    // Remove se sair da tela (ajusta conforme a tua altura)
    if (rect.y > 600) {
        destroyed = true;
    }
}

void Enemy::render(SDL_Renderer* renderer, float x, float y) {
    if (!destroyed && enemyTextures && !enemyTextures->empty()) {
        // Usa as texturas que já tens
        SDL_Texture* currentTexture = (*enemyTextures)[textureIndex % enemyTextures->size()];
        SDL_RenderCopy(renderer, currentTexture, nullptr, &rect);
    } else if (!destroyed) {
        // Fallback: renderização simples
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Vermelho
        SDL_RenderFillRect(renderer, &rect);
    }
}

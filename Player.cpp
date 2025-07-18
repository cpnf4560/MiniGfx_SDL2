#include "Player.h"
#include "GameManager.h"

Player::Player(int x, int y, int w, int h, SDL_Renderer* renderer, SDL_Texture* initialTexture)
    : gameRenderer(renderer), playerTexture(initialTexture) {
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    speed = GameManager::getInstance().getPlayerSpeed();
}

Player::~Player() {
    // Não destroy texture aqui se não foi criada por esta classe
}

void Player::move(int dx, int dy) {
    // Atualiza a velocidade com base no GameManager
    speed = GameManager::getInstance().getPlayerSpeed();

    // Move o jogador com a velocidade dinâmica
    rect.x += static_cast<int>(dx * speed * 0.016f); // Assumindo ~60 FPS
    rect.y += static_cast<int>(dy * speed * 0.016f);

    // Limites da tela (ajusta conforme a tua resolução)
    if (rect.x < 0) rect.x = 0;
    if (rect.y < 0) rect.y = 0;
    if (rect.x + rect.w > 800) rect.x = 800 - rect.w; // Assumindo largura 800
    if (rect.y + rect.h > 600) rect.y = 600 - rect.h; // Assumindo altura 600
}

void Player::update(float dt) {
    // Atualiza velocidade se necessário
    speed = GameManager::getInstance().getPlayerSpeed();
}

void Player::render(SDL_Renderer* renderer, float x, float y) {
    if (playerTexture) {
        SDL_RenderCopy(renderer, playerTexture, nullptr, &rect);
    } else {
        // Fallback para renderização simples
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // Verde
        SDL_RenderFillRect(renderer, &rect);
    }
}

void Player::render(SDL_Renderer* renderer, SDL_Texture* textureToRender, float x, float y) {
    if (textureToRender) {
        SDL_RenderCopy(renderer, textureToRender, nullptr, &rect);
    } else {
        render(renderer, x, y);
    }
}

void Player::setTexture(SDL_Texture* newTexture) {
    playerTexture = newTexture;
}

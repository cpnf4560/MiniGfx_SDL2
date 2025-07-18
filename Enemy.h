#ifndef ENEMY_H
#define ENEMY_H

#include "GameObject.h"
#include <SDL.h>
#include <vector>

class Enemy : public GameObject {
public:
    // Construtor compatível com o teu código antigo (7 parâmetros)
    Enemy(int x, int y, int w, int h, int speed, SDL_Renderer* renderer, std::vector<SDL_Texture*>& textures);

    void update(float dt) override;
    void render(SDL_Renderer* renderer, float x, float y) override;
    const SDL_Rect& getRect() const { return rect; }
    bool isDestroyed() const { return destroyed; }
    void destroy() { destroyed = true; }

private:
    float currentSpeed;                     // Velocidade dinâmica atual
    int baseSpeed;                         // Velocidade base
    bool destroyed;
    SDL_Renderer* gameRenderer;
    std::vector<SDL_Texture*>* enemyTextures;
    int textureIndex;
};

#endif // ENEMY_H

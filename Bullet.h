#ifndef BULLET_H
#define BULLET_H

#include "GameObject.h"

class Bullet : public GameObject {
public:
    // Construtor compatível com o teu código antigo (5 parâmetros)
    Bullet(int x, int y, int w, int h, int speed);

    void update(float dt) override;
    void render(SDL_Renderer* renderer, float x, float y) override;
    const SDL_Rect& getRect() const { return rect; }
    int getY() const { return rect.y; }
    int getH() const { return rect.h; }

private:
    float currentSpeed; // Velocidade dinâmica atual
    int baseSpeed;      // Velocidade base passada no construtor
};

#endif // BULLET_H

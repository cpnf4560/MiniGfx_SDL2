#ifndef HEART_H
#define HEART_H

#include "GameObject.h"
#include <SDL.h>

class Heart : public GameObject {
public:
    Heart(int x, int y, int w, int h, int healthValue, int speedValue, SDL_Texture* tex);

    void update(float dt) override;
    void render(SDL_Renderer* renderer, float x, float y) override;

    int getHealth() const { return health; }
    const SDL_Rect& getRect() const { return rect; }
    int getY() const { return rect.y; }
    int getW() const { return rect.w; }
    int getH() const { return rect.h; }

private:
    int health;
    SDL_Texture* texture;
    int speed;
};

#endif // HEART_H

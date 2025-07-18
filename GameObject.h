#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "Vec2D.h"
#include <SDL.h>

class GameObject {
public:
    GameObject();
    virtual ~GameObject();

    virtual void update(float dt) = 0;
    virtual void render(SDL_Renderer* renderer, float x, float y) = 0;

    Vec2D position;
    Vec2D velocity;
    SDL_Rect rect;
};

#endif // GAMEOBJECT_H

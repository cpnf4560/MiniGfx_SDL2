#ifndef PLAYER_H
#define PLAYER_H

#include "GameObject.h"
#include <SDL.h>

class Player : public GameObject {
public:
    Player(int x, int y, int w, int h, SDL_Renderer* renderer, SDL_Texture* initialTexture);
    ~Player();

    void move(int dx, int dy);
    void update(float dt) override;
    void render(SDL_Renderer* renderer, float x, float y) override;
    void render(SDL_Renderer* renderer, SDL_Texture* textureToRender, float x, float y);

    void setTexture(SDL_Texture* newTexture);

    const SDL_Rect& getRect() const { return rect; }
    int getX() const { return rect.x; }
    int getY() const { return rect.y; }
    int getW() const { return rect.w; }
    int getH() const { return rect.h; }

private:
    SDL_Renderer* gameRenderer;
    SDL_Texture* playerTexture;
    float speed; // Velocidade dinâmica
};

#endif // PLAYER_H

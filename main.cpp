#include <SDL.h>
#include "Game.h"
#include "GameManager.h"

int main(int argc, char* argv[]) {
    GameManager::getInstance().resetGame();
    Game game;

    if (game.init("Space Defender", 800, 600)) {
        Uint32 lastTime = SDL_GetTicks();

        while (game.isRunning()) {  // Removed 'running' variable
            Uint32 currentTime = SDL_GetTicks();
            float deltaTime = (currentTime - lastTime) / 1000.0f;
            lastTime = currentTime;

            game.handleEvents();
            game.update(deltaTime);
            game.render();
            SDL_Delay(16);
        }
    }

    game.clean();
    return 0;
}

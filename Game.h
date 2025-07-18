#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "Heart.h"

// Forward declaration
class GameManager;

namespace GameHelpers {
    void updatePlayerMovement(Player* player, bool left, bool right, bool up, bool down, float speed, float dt);
    void handleBulletEnemyCollisions(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies, int& score, GameManager& gm, Mix_Chunk* soundHit);
    void handlePlayerEnemyCollisions(Player* player, std::vector<Enemy>& enemies, int& playerLives, bool& hitAnimationActive, Uint32& hitAnimationStartTime, Mix_Chunk* soundHit);
}

enum GameState {
    GAME_STATE_MENU,
    GAME_STATE_PLAYER_SELECTION,
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER
};

enum PlayerChoice {
    PLAYER_TRUMP = 0,
    PLAYER_CARLOS = 1,
    PLAYER_BLUEY = 2
};

class Game {
public:
    Game();
    ~Game();

    bool init(const char* title, int width, int height);
    void handleEvents();
    void update(float dt);
    void render();
    void clean();

    bool isRunning() const { return m_isRunning; }

private:
    static const int ENEMY_SPAWN_DELAY = 800;
    static constexpr const char* HIGHSCORE_FILE = "highscore.txt";

    bool m_isRunning;
    SDL_Window* window;
    SDL_Renderer* renderer;

    // Game state
    GameState currentGameState;
    bool gameOver;

    // Player
    Player* player;
    PlayerChoice selectedPlayer;
    int currentSelectionIndex;
    bool holdingLeft, holdingRight, holdingUp, holdingDown;
    int playerLives;
    bool playerHitAnimationActive;
    Uint32 playerHitAnimationStartTime;

    // Score and timing
    int score;
    int highscore;
    Uint32 lastEnemySpawnTime;
    int lastHeartSpawnScore;

        // Variáveis para fullscreen scaling
    int originalWidth;
    int originalHeight;
    float scaleX;
    float scaleY;
    int offsetX;
    int offsetY;

    // Game Objects
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Heart> hearts;

       // Textures (keep existing order)
    std::vector<SDL_Texture*> playerSelectionTextures;
    std::vector<SDL_Texture*> backgroundTextures;
    std::vector<SDL_Texture*> playerBaseTextures;
    std::vector<SDL_Texture*> playerHitTextures;
    std::vector<SDL_Texture*> playerHeartTextures;
    std::vector<std::vector<SDL_Texture*>> enemyTextures;


    SDL_Texture* currentBackgroundTexture;
    SDL_Texture* currentHeartTexture;
    std::vector<SDL_Texture*> currentEnemyTextures;

    SDL_Texture* scoreTexture;
    SDL_Texture* gameOverTexture;
    SDL_Texture* menuTexture;
    SDL_Texture* highscorePlayingTexture;

    // Audio
    TTF_Font* font;
    Mix_Chunk* soundHit;
    Mix_Chunk* soundMiss;
    Mix_Chunk* soundGameOver;
    Mix_Chunk* soundLaser;
    Mix_Music* menuMusic;
    std::vector<Mix_Music*> gameMusics;

    // Private methods
    void loseLife();
    void gainLife();
    void spawnSingleEnemy(int enemySpeed);
    void spawnHeartPickup();
    void updateScoreTexture();
    void updateHighscorePlayingTexture();
    void playSound(Mix_Chunk* sound);
    void increaseScore(int amount);
    void decreaseScore(int amount);
    void saveHighscore();
    void loadHighscore();
    void updateMenuTexture();
    void calculateScale();

    // Resource loading
    void loadPlayerSelectionTextures();
    void loadBackgroundTextures();
    void loadGameMusics();
    void loadPlayerAnimationTextures();
    void loadHeartTextures();
    void loadEnemyTextures();
    void setupPlayerBasedOnSelection();
};

#endif // GAME_H

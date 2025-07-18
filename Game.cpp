#include "Game.h"
#include "GameManager.h"  // ← NOVA LINHA ADICIONADA
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <utility>

namespace GameHelpers {
    void updatePlayerMovement(Player* player, bool left, bool right, bool up, bool down, int speed) {
        int dx = 0;
        int dy = 0;

        if (left) dx -= speed;
        if (right) dx += speed;
        if (up) dy -= speed;
        if (down) dy += speed;

        if (dx != 0 || dy != 0) {
            player->move(dx, dy);
        }
    }

void handleBulletEnemyCollisions(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies, int& score, GameManager& gm, Mix_Chunk* soundHit) {
    for (auto bullet = bullets.begin(); bullet != bullets.end(); ) {
        bool hit = false;
        for (auto enemy = enemies.begin(); enemy != enemies.end(); ) {
            SDL_Rect bulletRect = bullet->getRect();
            SDL_Rect enemyRect = enemy->getRect();

            if (SDL_HasIntersection(&bulletRect, &enemyRect)) {
                enemy = enemies.erase(enemy);
                bullet = bullets.erase(bullet);

                // DEBUG: Adiciona estes prints temporários
                //std::cout << "HIT DETECTADO! Score antes: " << score << std::endl;
                score++;
                gm.addScore(1);
                //std::cout << "Score depois: " << score << std::endl;
                //std::cout << "GameManager score: " << gm.getScore() << std::endl;

                if (soundHit) {
                    Mix_PlayChannel(-1, soundHit, 0);
                }

                hit = true;
                break;
            } else {
                ++enemy;
            }
        }
        if (!hit) {
            ++bullet;
        }
    }
}

    void handlePlayerEnemyCollisions(Player* player, std::vector<Enemy>& enemies, int& playerLives, bool& hitAnimationActive, Uint32& hitAnimationStartTime, Mix_Chunk* soundHit) {
        for (auto enemy = enemies.begin(); enemy != enemies.end(); ) {
            if (SDL_HasIntersection(&player->getRect(), &enemy->getRect())) {
                enemy = enemies.erase(enemy);
                playerLives--;  // Mantém a lógica original
                hitAnimationActive = true;
                hitAnimationStartTime = SDL_GetTicks();

                if (soundHit) {
                    Mix_PlayChannel(-1, soundHit, 0);
                }
            } else {
                ++enemy;
            }
        }
    }
}

Game::Game() :
    m_isRunning(false),
    window(nullptr),
    renderer(nullptr),
    currentGameState(GAME_STATE_MENU),
    gameOver(false),
    player(nullptr),
    selectedPlayer(PLAYER_TRUMP),
    currentSelectionIndex(0),
    holdingLeft(false),
    holdingRight(false),
    holdingUp(false),
    holdingDown(false),
    playerLives(7),
    playerHitAnimationActive(false),
    playerHitAnimationStartTime(0),
    score(0),
    highscore(0),
    lastEnemySpawnTime(0),
    lastHeartSpawnScore(-1),
    originalWidth(800),    // ← NOVO
    originalHeight(600),   // ← NOVO
    currentBackgroundTexture(nullptr),
    currentHeartTexture(nullptr),
    scoreTexture(nullptr),
    gameOverTexture(nullptr),
    menuTexture(nullptr),
    highscorePlayingTexture(nullptr),
    font(nullptr),
    soundHit(nullptr),
    soundMiss(nullptr),
    soundGameOver(nullptr),
    soundLaser(nullptr),
    menuMusic(nullptr),
    scaleX(1.0f),         // ← NOVO
    scaleY(1.0f),         // ← NOVO
    offsetX(0),           // ← NOVO
    offsetY(0)            // ← NOVO
{}

Game::~Game() {
    clean();
}

void Game::calculateScale() {
    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    // DEBUG: Verifica se a função está a ser chamada
    std::cout << "calculateScale() chamada!" << std::endl;
    std::cout << "Window size: " << windowWidth << "x" << windowHeight << std::endl;

    // Calcula a escala mantendo aspect ratio
    float scaleXCalc = (float)windowWidth / originalWidth;
    float scaleYCalc = (float)windowHeight / originalHeight;

    // Usa a menor escala para manter proporções
    float scale = std::min(scaleXCalc, scaleYCalc);
    scaleX = scale;
    scaleY = scale;

    // Calcula offset para centralizar
    int scaledWidth = (int)(originalWidth * scale);
    int scaledHeight = (int)(originalHeight * scale);
    offsetX = (windowWidth - scaledWidth) / 2;
    offsetY = (windowHeight - scaledHeight) / 2;

    std::cout << "Scale: " << scale << ", Offset: " << offsetX << ", " << offsetY << std::endl;
}

bool Game::init(const char* title, int width, int height) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! " << Mix_GetError() << "\n";
        return false;
    }

    soundLaser = Mix_LoadWAV("assets/sounds/laser.wav");
    if (!soundLaser) {
        std::cerr << "Failed to load laser sound: " << Mix_GetError() << std::endl;
    }

    if (TTF_Init() == -1) {
        std::cerr << "TTF_Init failed: " << TTF_GetError() << "\n";
        return false;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! " << IMG_GetError() << "\n";
        return false;
    }

    window = SDL_CreateWindow(title,
                                 SDL_WINDOWPOS_CENTERED,
                                 SDL_WINDOWPOS_CENTERED,
                                 width, height,
                                 SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << "\n";
        return false;
    }

        // Adiciona estas linhas:
    originalWidth = width;   // ← NOVO
    originalHeight = height; // ← NOVO
    calculateScale();        // ← NOVO

    font = TTF_OpenFont("assets/fonts/arial.ttf", 24);
    if (!font) {
        std::cerr << "Failed to load font: " << TTF_GetError() << "\n";
        return false;
    }

    loadPlayerSelectionTextures();
    loadBackgroundTextures();
    loadPlayerAnimationTextures();
    loadGameMusics();
    loadHeartTextures();
    loadEnemyTextures();

    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* gameOverSurface = TTF_RenderText_Solid(font, "GAME OVER", white);
    if (!gameOverSurface) {
        std::cerr << "TTF_RenderText_Solid failed for Game Over: " << TTF_GetError() << "\n";
        return false;
    }
    gameOverTexture = SDL_CreateTextureFromSurface(renderer, gameOverSurface);
    SDL_FreeSurface(gameOverSurface);
    if (!gameOverTexture) {
        std::cerr << "SDL_CreateTextureFromSurface failed for Game Over: " << SDL_GetError() << "\n";
        return false;
    }

    menuMusic = Mix_LoadMUS("assets/sounds/bomba_32.wav");
    if (!menuMusic) {
        std::cerr << "Failed to load menu music: " << Mix_GetError() << "\n";
        return false;
    }
    Mix_PlayMusic(menuMusic, -1);

    soundHit = Mix_LoadWAV("assets/sounds/hit.wav");
    soundMiss = Mix_LoadWAV("assets/sounds/miss.wav");
    soundGameOver = Mix_LoadWAV("assets/sounds/gameover.wav");
    if (!soundHit || !soundMiss || !soundGameOver) {
        std::cerr << "Failed to load sound effects: " << Mix_GetError() << "\n";
        return false;
    }

    player = new Player(width/2 - (31/2), height - 40, 31, 40, renderer, nullptr);

    loadHighscore();
    updateHighscorePlayingTexture();
    updateMenuTexture();

    // ← NOVA LINHA: Inicializar GameManager
    GameManager::getInstance().resetGame();

    m_isRunning = true;
    srand(time(NULL));
    return true;
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_isRunning = false;
        }

        if (event.type == SDL_KEYDOWN) {
            if (currentGameState == GAME_STATE_MENU) {
                currentGameState = GAME_STATE_PLAYER_SELECTION;
                Mix_HaltMusic();
            } else if (currentGameState == GAME_STATE_PLAYER_SELECTION) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    currentSelectionIndex = (currentSelectionIndex - 1 + playerSelectionTextures.size()) % playerSelectionTextures.size();
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    currentSelectionIndex = (currentSelectionIndex + 1) % playerSelectionTextures.size();
                } else if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    selectedPlayer = (PlayerChoice)currentSelectionIndex;
                    setupPlayerBasedOnSelection();
                    currentGameState = GAME_STATE_PLAYING;
                    score = 0;
                    playerLives = 7;
                    this->enemies.clear();
                    this->bullets.clear();
                    this->hearts.clear();
                    lastHeartSpawnScore = -1;
                    player->move(400 - (31/2) - player->getX(), 600 - 40 - player->getY());
                    lastEnemySpawnTime = SDL_GetTicks();
                    updateScoreTexture();
                    // ← NOVA LINHA: Reset do GameManager quando começa novo jogo
                    GameManager::getInstance().resetGame();
                }
            }
            else if (currentGameState == GAME_STATE_GAME_OVER) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    currentGameState = GAME_STATE_PLAYER_SELECTION;
                    gameOver = false;
                    score = 0;
                    playerLives = 7;
                    enemies.clear();  // Acesso direto ao membro
                    bullets.clear();  // Acesso direto ao membro
                    hearts.clear();   // Acesso direto ao membro
                    lastHeartSpawnScore = -1;
                    player->move(400 - (31/2) - player->getX(), 600 - 40 - player->getY());
                    lastEnemySpawnTime = SDL_GetTicks();
                    updateScoreTexture();
                    Mix_HaltMusic();
                    // ← NOVA LINHA: Reset do GameManager quando reinicia
                    GameManager::getInstance().resetGame();
                }
            } else if (currentGameState == GAME_STATE_PLAYING) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    holdingLeft = true;
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    holdingRight = true;
                } else if (event.key.keysym.sym == SDLK_UP) {
                    holdingUp = true;
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    holdingDown = true;
                }
                else if (event.key.keysym.sym == SDLK_SPACE) {
                    // ← ALTERADO: Usar velocidade dinâmica das balas
                    bullets.emplace_back(player->getX() + player->getW() / 2 - 2,
                                         player->getY(), 4, 10, static_cast<int>(GameManager::getInstance().getBulletSpeed() / 60.0f));
                    // Tocar som do laser
                    if (soundLaser) {
                        Mix_PlayChannel(-1, soundLaser, 0);
                    }
                }
                else if (event.key.keysym.sym == SDLK_F11) {
                Uint32 flags = SDL_GetWindowFlags(window);
                if (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) {
                    SDL_SetWindowFullscreen(window, 0);
                } else {
                    SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
                calculateScale(); // ← NOVA LINHA: Recalcula escala após mudança
            }
            }
        }
        else if (event.type == SDL_KEYUP) {
            if (currentGameState == GAME_STATE_PLAYING) {
                if (event.key.keysym.sym == SDLK_LEFT) {
                    holdingLeft = false;
                } else if (event.key.keysym.sym == SDLK_RIGHT) {
                    holdingRight = false;
                } else if (event.key.keysym.sym == SDLK_UP) {
                    holdingUp = false;
                } else if (event.key.keysym.sym == SDLK_DOWN) {
                    holdingDown = false;
                }
            }
        }
    }
}

void Game::update(float dt) {
    if (currentGameState == GAME_STATE_MENU || currentGameState == GAME_STATE_PLAYER_SELECTION || currentGameState == GAME_STATE_GAME_OVER) {
        return;
    }

    if (playerHitAnimationActive && SDL_GetTicks() - playerHitAnimationStartTime > 200) {
        playerHitAnimationActive = false;
    }

    if ((score % 10 == 5) && (score >= 5) && (score <= 45) && (score != lastHeartSpawnScore) && (hearts.empty())) {
        spawnHeartPickup();
        lastHeartSpawnScore = score;
    }

    // ← ALTERADO: Usar velocidades do GameManager em vez de cálculo manual
    GameManager& gm = GameManager::getInstance();
    float current_playerSpeed = gm.getPlayerSpeed();
    float current_enemySpeed = gm.getEnemySpeed();

    // ← ALTERADO: Movimento do jogador usando GameHelper
    GameHelpers::updatePlayerMovement(player, holdingLeft, holdingRight, holdingUp, holdingDown, static_cast<int>(current_playerSpeed * dt));

    for (auto& bullet : bullets) {  // Acesso direto ao membro
        bullet.update(dt);
    }

    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](const Bullet& b) { return b.getY() + b.getH() < 0; }),
        bullets.end());

    for (auto enemy = enemies.begin(); enemy != enemies.end(); ) {
        enemy->update(dt);
        if (enemy->getRect().y > 600) {
            enemy = enemies.erase(enemy);
            loseLife();
        } else {
            ++enemy;
        }
    }

    // ← ALTERADO: Usar GameHelper para colisões jogador-inimigo
    GameHelpers::handlePlayerEnemyCollisions(player, enemies, playerLives, playerHitAnimationActive, playerHitAnimationStartTime, soundHit);

    for (auto heart = hearts.begin(); heart != hearts.end(); ) {
        heart->update(dt);
        if (heart->getY() > 600) {
            heart = hearts.erase(heart);
        }
        else if (SDL_HasIntersection(&player->getRect(), &heart->getRect())) {
            gainLife();
            heart = hearts.erase(heart);
        } else {
            ++heart;
        }
    }

    // ← ALTERADO: Usar GameHelper para colisões bala-inimigo
    GameHelpers::handleBulletEnemyCollisions(bullets, enemies, score, gm, soundHit);
    updateScoreTexture();

    if (playerLives <= 0) {
        playSound(soundGameOver);
        currentGameState = GAME_STATE_GAME_OVER;
        gameOver = true;
        saveHighscore();
        updateMenuTexture();
        Mix_HaltMusic();
        return;
    }

    // ← ALTERADO: Spawn de inimigos baseado na velocidade atual
    int speedLevel = gm.getScore() / 10 + 1;
    if (enemies.size() < static_cast<size_t>(5 + speedLevel)) {
        spawnSingleEnemy(static_cast<int>(current_enemySpeed));
    }
}

void Game::render() {
    // Limpa tudo
    SDL_RenderSetViewport(renderer, nullptr);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // *** MUDANÇA RADICAL: Escala TODO o renderer ***
    SDL_RenderSetLogicalSize(renderer, 800, 600);

    // Background normal
    if (currentGameState != GAME_STATE_MENU && currentBackgroundTexture) {
        SDL_RenderCopy(renderer, currentBackgroundTexture, NULL, NULL);
    } else {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
    }

    if (currentGameState == GAME_STATE_MENU) {
        if (menuTexture) {
            int texW, texH;
            SDL_QueryTexture(menuTexture, NULL, NULL, &texW, &texH);
            SDL_Rect dstRect = {(800 - texW) / 2, (600 - texH) / 2, texW, texH};
            SDL_RenderCopy(renderer, menuTexture, NULL, &dstRect);
        }
    } else if (currentGameState == GAME_STATE_PLAYER_SELECTION) {
        SDL_Color white = {255, 255, 255, 255};
        std::string selectText = "Escolha o seu Jogador!";
        SDL_Surface* selectSurface = TTF_RenderText_Solid(font, selectText.c_str(), white);
        if (selectSurface) {
            SDL_Texture* selectTexture = SDL_CreateTextureFromSurface(renderer, selectSurface);
            SDL_FreeSurface(selectSurface);
            if (selectTexture) {
                int texW, texH;
                SDL_QueryTexture(selectTexture, NULL, NULL, &texW, &texH);
                SDL_Rect dstRect = {(800 - texW) / 2, 50, texW, texH};
                SDL_RenderCopy(renderer, selectTexture, NULL, &dstRect);
                SDL_DestroyTexture(selectTexture);
            }
        }

        int startX = (800 - (3 * 100 + 2 * 50)) / 2;
        for (int i = 0; i < static_cast<int>(playerSelectionTextures.size()); ++i) {
            SDL_Rect playerRect = {startX + (int)(i * (100 + 50)), 200, 100, 100};

            if (i == currentSelectionIndex) {
                SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
                SDL_RenderDrawRect(renderer, &playerRect);
            }
            SDL_RenderCopy(renderer, playerSelectionTextures[i], NULL, &playerRect);
        }

        std::string confirmText = "Use SETAS para escolher e ENTER para confirmar!";
        SDL_Surface* confirmSurface = TTF_RenderText_Solid(font, confirmText.c_str(), white);
        if (confirmSurface) {
            SDL_Texture* confirmTexture = SDL_CreateTextureFromSurface(renderer, confirmSurface);
            SDL_FreeSurface(confirmSurface);
            if (confirmTexture) {
                int texW, texH;
                SDL_QueryTexture(confirmTexture, NULL, NULL, &texW, &texH);
                SDL_Rect dstRect = {(800 - texW) / 2, 600 - 50, texW, texH};
                SDL_RenderCopy(renderer, confirmTexture, NULL, &dstRect);
                SDL_DestroyTexture(confirmTexture);
            }
        }
    } else if (currentGameState == GAME_STATE_GAME_OVER) {
        SDL_Color white = {255, 255, 255, 255};

        TTF_Font* largeFont = TTF_OpenFont("assets/fonts/arial.ttf", 72);
        if (largeFont) {
            SDL_Surface* goSurface = TTF_RenderText_Solid(largeFont, "GAME OVER", white);
            if (goSurface) {
                SDL_Texture* goTexture = SDL_CreateTextureFromSurface(renderer, goSurface);
                SDL_FreeSurface(goSurface);
                if (goTexture) {
                    int texW, texH;
                    SDL_QueryTexture(goTexture, NULL, NULL, &texW, &texH);
                    SDL_Rect dstRect = {(800 - texW) / 2, (600 - texH) / 2 - 100, texW, texH};
                    SDL_RenderCopy(renderer, goTexture, NULL, &dstRect);
                    SDL_DestroyTexture(goTexture);
                }
            }
            TTF_CloseFont(largeFont);
        }

        std::string roundScoreText = "Pontuacao da Ronda: " + std::to_string(score);
        SDL_Surface* rsSurface = TTF_RenderText_Solid(font, roundScoreText.c_str(), white);
        if (rsSurface) {
            SDL_Texture* rsTexture = SDL_CreateTextureFromSurface(renderer, rsSurface);
            SDL_FreeSurface(rsSurface);
            if (rsTexture) {
                int texW, texH;
                SDL_QueryTexture(rsTexture, NULL, NULL, &texW, &texH);
                SDL_Rect dstRect = {(800 - texW) / 2, (600 / 2) + 10, texW, texH};
                SDL_RenderCopy(renderer, rsTexture, NULL, &dstRect);
                SDL_DestroyTexture(rsTexture);
            }
        }

        std::string currentHighscoreText = "Highscore: " + std::to_string(highscore);
        SDL_Surface* highscoreGOSurface = TTF_RenderText_Solid(font, currentHighscoreText.c_str(), white);
        if (highscoreGOSurface) {
            SDL_Texture* highscoreGOTexture = SDL_CreateTextureFromSurface(renderer, highscoreGOSurface);
            SDL_FreeSurface(highscoreGOSurface);
            if (highscoreGOTexture) {
                int texW, texH;
                SDL_QueryTexture(highscoreGOTexture, NULL, NULL, &texW, &texH);
                SDL_Rect dstRect = {(800 - texW) / 2, (600 / 2) + 50, texW, texH};
                SDL_RenderCopy(renderer, highscoreGOTexture, NULL, &dstRect);
                SDL_DestroyTexture(highscoreGOTexture);
            }
        }

        std::string replayText = "Pressione ENTER para jogar de novo";
        SDL_Surface* replaySurface = TTF_RenderText_Solid(font, replayText.c_str(), white);
        if (replaySurface) {
            SDL_Texture* replayTexture = SDL_CreateTextureFromSurface(renderer, replaySurface);
            SDL_FreeSurface(replaySurface);

            if (replayTexture) {
                int texW, texH;
                SDL_QueryTexture(replayTexture, NULL, NULL, &texW, &texH);
                SDL_Rect dstRect = {(800 - texW) / 2, (600 - texH) / 2 + 50 + 40, texW, texH};
                SDL_RenderCopy(renderer, replayTexture, NULL, &dstRect);
                SDL_DestroyTexture(replayTexture);
            }
        }

        TTF_Font* smallFont = TTF_OpenFont("assets/fonts/arial.ttf", 16);
        if (smallFont) {
            SDL_Surface* ufcdSurface = TTF_RenderText_Solid(smallFont, "UFCD 0809 - Programacao em CPP", white);
            if (ufcdSurface) {
                SDL_Texture* ufcdTexture = SDL_CreateTextureFromSurface(renderer, ufcdSurface);
                SDL_FreeSurface(ufcdSurface);
                if (ufcdTexture) {
                    int texW, texH;
                    SDL_QueryTexture(ufcdTexture, NULL, NULL, &texW, &texH);
                    SDL_Rect dstRect = {(800 - texW) / 2, 600 - texH - 10, texW, texH};
                    SDL_RenderCopy(renderer, ufcdTexture, NULL, &dstRect);
                    SDL_DestroyTexture(ufcdTexture);
                }
            }
            TTF_CloseFont(smallFont);
        }
    } else if (currentGameState == GAME_STATE_PLAYING) {
        if (playerHitAnimationActive) {
            player->render(renderer, playerHitTextures[selectedPlayer], 0.0f, 0.0f);
        } else {
            player->render(renderer, playerBaseTextures[selectedPlayer], 0.0f, 0.0f);
        }

        for (auto& enemy : enemies) {
            enemy.render(renderer, 0.0f, 0.0f);
        }

        for (auto& bullet : bullets) {
            bullet.render(renderer, 0.0f, 0.0f);
        }

        for (auto& heart : hearts) {
            heart.render(renderer, 0.0f, 0.0f);
        }

        if (scoreTexture) {
            SDL_Rect dstRect = {10, 10, 150, 40};
            SDL_RenderCopy(renderer, scoreTexture, NULL, &dstRect);
        }

        if (highscorePlayingTexture) {
            int texW, texH;
            SDL_QueryTexture(highscorePlayingTexture, NULL, NULL, &texW, &texH);
            SDL_Rect dstRect = {10, 10 + 40 + 5, texW, texH};
            SDL_RenderCopy(renderer, highscorePlayingTexture, NULL, &dstRect);
        }

        if (currentHeartTexture) {
            int heartSize = 30;
            int maxHearts = 7;
            int totalHeartsWidth = (heartSize * maxHearts) + (5 * (maxHearts - 1));
            int startX = 800 - totalHeartsWidth - 10;

            for (int i = 0; i < playerLives; ++i) {
                SDL_Rect heartRect = {startX + (i * (heartSize + 5)), 10, heartSize, heartSize};
                SDL_RenderCopy(renderer, currentHeartTexture, NULL, &heartRect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

void Game::clean() {
    if (player) {
        delete player;
        player = nullptr;
    }

    if (scoreTexture) {
        SDL_DestroyTexture(scoreTexture);
        scoreTexture = nullptr;
    }

    if (gameOverTexture) {
        SDL_DestroyTexture(gameOverTexture);
        gameOverTexture = nullptr;
    }

    if (menuTexture) {
        SDL_DestroyTexture(menuTexture);
        menuTexture = nullptr;
    }

    if (highscorePlayingTexture) {
        SDL_DestroyTexture(highscorePlayingTexture);
        highscorePlayingTexture = nullptr;
    }

    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }

    if (soundLaser) {
        Mix_FreeChunk(soundLaser);
        soundLaser = nullptr;
    }
    if (soundHit) {
        Mix_FreeChunk(soundHit);
        soundHit = nullptr;
    }
    if (soundMiss) {
        Mix_FreeChunk(soundMiss);
        soundMiss = nullptr;
    }
    if (soundGameOver) {
        Mix_FreeChunk(soundGameOver);
        soundGameOver = nullptr;
    }

    if (menuMusic) Mix_FreeMusic(menuMusic);
    for (auto music : gameMusics) {
        if (music) Mix_FreeMusic(music);
    }
    gameMusics.clear();

    Mix_CloseAudio();

    for (auto texture : playerHeartTextures) {
        if (texture) SDL_DestroyTexture(texture);
    }
    playerHeartTextures.clear();
    currentHeartTexture = nullptr;

    for (auto texture : backgroundTextures) {
        if (texture) SDL_DestroyTexture(texture);
    }
    backgroundTextures.clear();

    for (auto texture : playerSelectionTextures) {
        if (texture) SDL_DestroyTexture(texture);
    }
    playerSelectionTextures.clear();

    for (auto texture : playerBaseTextures) {
        if (texture) SDL_DestroyTexture(texture);
    }
    playerBaseTextures.clear();

    for (auto texture : playerHitTextures) {
        if (texture) SDL_DestroyTexture(texture);
    }
    playerHitTextures.clear();

    for (auto& innerVec : enemyTextures) {
        for (auto texture : innerVec) {
            if (texture) SDL_DestroyTexture(texture);
        }
    }
    enemyTextures.clear();
    currentEnemyTextures.clear();

    this->hearts.clear();

    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void Game::loseLife() {
    playerLives--;
    playSound(soundMiss);
    playerHitAnimationActive = true;
    playerHitAnimationStartTime = SDL_GetTicks();
    std::cout << "Vida perdida! Vidas restantes: " << playerLives << "\n";
}

void Game::gainLife() {
    if (playerLives < 10) {
        playerLives++;
        std::cout << "Vida extra ganha! Vidas restantes: " << playerLives << "\n";
    } else {
        std::cout << "Vida extra perdida (vidas maximas atingidas).\n";
    }
}

void Game::spawnSingleEnemy(int enemySpeed) {
    if (SDL_GetTicks() - lastEnemySpawnTime > ENEMY_SPAWN_DELAY) {
        int x_pos = rand() % (800 - 40);
        enemies.emplace_back(x_pos, 50, 40, 20, enemySpeed, renderer, currentEnemyTextures);
        lastEnemySpawnTime = SDL_GetTicks();
    }
}

void Game::spawnHeartPickup() {
    if (currentHeartTexture && hearts.empty()) {
        int x_pos = rand() % (800 - 30);
        int y_pos = -50;
        int heartSize = 30;

        // ← CORRIGIDO: Usar velocidade proporcional aos inimigos
        GameManager& gm = GameManager::getInstance();
        int heartSpeed = static_cast<int>(gm.getEnemySpeed() * 0.6f); // 60% da velocidade dos inimigos

        hearts.emplace_back(x_pos, y_pos, heartSize, heartSize, 1, heartSpeed, currentHeartTexture);
        std::cout << "Coracao extra no ecrã!\n";
    }
}

void Game::updateScoreTexture() {
    if (scoreTexture) {
        SDL_DestroyTexture(scoreTexture);
        scoreTexture = nullptr;
    }

    SDL_Color white = {255, 255, 255, 255};
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* surface = TTF_RenderText_Solid(font, scoreText.c_str(), white);

    if (!surface) {
        std::cerr << "TTF_RenderText_Solid failed: " << TTF_GetError() << "\n";
        return;
    }

    scoreTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!scoreTexture) {
        std::cerr << "SDL_CreateTextureFromSurface failed: " << SDL_GetError() << "\n";
    }
}

void Game::updateHighscorePlayingTexture() {
    if (highscorePlayingTexture) {
        SDL_DestroyTexture(highscorePlayingTexture);
        highscorePlayingTexture = nullptr;
    }

    SDL_Color white = {255, 255, 255, 255};
    std::string highscoreText = "Highscore: " + std::to_string(highscore);
    SDL_Surface* surface = TTF_RenderText_Solid(font, highscoreText.c_str(), white);

    if (!surface) {
        std::cerr << "TTF_RenderText_Solid failed for highscore playing texture: " << TTF_GetError() << "\n";
        return;
    }

    highscorePlayingTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!highscorePlayingTexture) {
        std::cerr << "SDL_CreateTextureFromSurface failed for highscore playing texture: " << SDL_GetError() << "\n";
    }
}

void Game::playSound(Mix_Chunk* sound) {
    if (sound) {
        Mix_PlayChannel(-1, sound, 0);  // ← CORRIGIDO: usar o som passado como parâmetro
    }
}

void Game::increaseScore(int amount) {
    score += amount;
    // ← ALTERADO: Atualizar também o GameManager
    GameManager::getInstance().addScore(amount);
    updateScoreTexture();
    playSound(soundHit);
}

void Game::decreaseScore(int amount) {
    score -= amount;
    updateScoreTexture();
}

void Game::saveHighscore() {
    if (score > highscore) {
        highscore = score;
        std::ofstream file(HIGHSCORE_FILE);
        if (file.is_open()) {
            file << highscore;
            file.close();
            std::cout << "New highscore: " << highscore << "\n";
        } else {
            std::cerr << "Unable to open highscore file for writing!\n";
        }
        updateHighscorePlayingTexture();
    }
}

void Game::loadHighscore() {
    std::ifstream file(HIGHSCORE_FILE);
    if (file.is_open()) {
        file >> highscore;
        file.close();
        std::cout << "Loaded highscore: " << highscore << "\n";
    } else {
        std::cerr << "No highscore file found or unable to open. Highscore set to 0.\n";
        highscore = 0;
    }
    updateHighscorePlayingTexture();
}

void Game::updateMenuTexture() {
    if (menuTexture) {
        SDL_DestroyTexture(menuTexture);
    }

    SDL_Color white = {255, 255, 255, 255};
    std::string menuText = "SPACE DEFENDER\n\n";
    menuText += "Highscore: " + std::to_string(highscore) + "\n\n";
    menuText += "Pressione qualquer tecla para iniciar o jogo";

    SDL_Surface* surface = TTF_RenderText_Blended_Wrapped(font, menuText.c_str(), white, 700);
    if (!surface) {
        std::cerr << "TTF_RenderText_Blended_Wrapped failed for Menu: " << TTF_GetError() << "\n";
        return;
    }
    menuTexture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    if (!menuTexture) {
        std::cerr << "SDL_CreateTextureFromSurface failed for Menu: " << SDL_GetError() << "\n";
    }
}

void Game::loadPlayerSelectionTextures() {
    std::vector<std::string> filenames = {
        "assets/images/player1.png", "assets/images/player2.png", "assets/images/player3.png"
    };
    for (const auto& filename : filenames) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load player selection image " << filename << ": " << IMG_GetError() << "\n";
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture from " << filename << ": " << SDL_GetError() << "\n";
            continue;
        }
        playerSelectionTextures.push_back(texture);
    }
}

void Game::loadBackgroundTextures() {
    std::vector<std::string> filenames = {
        "assets/images/background1.png", "assets/images/background2.png", "assets/images/background3.png"
    };
    for (const auto& filename : filenames) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load background image " << filename << ": " << IMG_GetError() << "\n";
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture from " << filename << ": " << SDL_GetError() << "\n";
            continue;
        }
        backgroundTextures.push_back(texture);
    }
    if (!backgroundTextures.empty()) {
        currentBackgroundTexture = backgroundTextures[0];
    }
}

void Game::loadGameMusics() {
    std::vector<std::string> filenames = {
        "assets/sounds/musica1.wav", "assets/sounds/musica2.wav", "assets/sounds/musica3.wav"
    };
    for (const auto& filename : filenames) {
        Mix_Music* music = Mix_LoadMUS(filename.c_str());
        if (!music) {
            std::cerr << "Failed to load music " << filename << ": " << Mix_GetError() << "\n";
            continue;
        }
        gameMusics.push_back(music);
    }
}

void Game::loadPlayerAnimationTextures() {
    for (int i = 1; i <= 3; ++i) {
        std::string filename = "assets/images/player" + std::to_string(i) + ".1.png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load player base image " << filename << ": " << IMG_GetError() << "\n";
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture from " << filename << ": " << SDL_GetError() << "\n";
            continue;
        }
        playerBaseTextures.push_back(texture);
    }

    for (int i = 1; i <= 3; ++i) {
        std::string filename = "assets/images/player" + std::to_string(i) + ".2.png";
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load player hit image " << filename << ": " << IMG_GetError() << "\n";
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture from " << filename << ": " << SDL_GetError() << "\n";
            continue;
        }
        playerHitTextures.push_back(texture);
    }
}

void Game::loadHeartTextures() {
    std::vector<std::string> filenames = {
        "assets/images/heart1.png", "assets/images/heart2.png", "assets/images/heart3.png"
    };
    for (const auto& filename : filenames) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load heart image " << filename << ": " << IMG_GetError() << "\n";
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture from " << filename << ": " << SDL_GetError() << "\n";
            continue;
        }
        playerHeartTextures.push_back(texture);
    }
    if (!playerHeartTextures.empty()) {
        currentHeartTexture = playerHeartTextures[0];
    }
}

void Game::loadEnemyTextures() {
    enemyTextures.resize(3);

    std::vector<std::string> enemy1Filenames = { "assets/images/inimigo1.png" };
    for (const auto& filename : enemy1Filenames) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load enemy image " << filename << ": " << IMG_GetError() << "\n";
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture from " << filename << ": " << SDL_GetError() << "\n";
            continue;
        }
        enemyTextures[PLAYER_TRUMP].push_back(texture);
    }

    std::vector<std::string> enemy2Filenames = { "assets/images/inimigo2.png" };
    for (const auto& filename : enemy2Filenames) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load enemy image " << filename << ": " << IMG_GetError() << "\n";
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture from " << filename << ": " << SDL_GetError() << "\n";
            continue;
        }
        enemyTextures[PLAYER_CARLOS].push_back(texture);
    }

    std::vector<std::string> enemy3Filenames = { "assets/images/inimigo3.png", "assets/images/inimigo3.1.png" };
    for (const auto& filename : enemy3Filenames) {
        SDL_Surface* surface = IMG_Load(filename.c_str());
        if (!surface) {
            std::cerr << "Failed to load enemy image " << filename << ": " << IMG_GetError() << "\n";
            continue;
        }
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
        if (!texture) {
            std::cerr << "Failed to create texture from " << filename << ": " << SDL_GetError() << "\n";
            continue;
        }
        enemyTextures[PLAYER_BLUEY].push_back(texture);
    }
}

void Game::setupPlayerBasedOnSelection() {
    if (selectedPlayer < playerBaseTextures.size() && playerBaseTextures[selectedPlayer]) {
        player->setTexture(playerBaseTextures[selectedPlayer]);
    } else {
        std::cerr << "Erro: Textura base do jogador selecionado nao disponivel.\n";
    }

    if (selectedPlayer < backgroundTextures.size() && backgroundTextures[selectedPlayer]) {
        currentBackgroundTexture = backgroundTextures[selectedPlayer];
    } else {
        std::cerr << "Erro: Textura de fundo selecionada nao disponivel.\n";
    }

    if (selectedPlayer < playerHeartTextures.size() && playerHeartTextures[selectedPlayer]) {
        currentHeartTexture = playerHeartTextures[selectedPlayer];
    } else {
        std::cerr << "Erro: Textura de coracao selecionada nao disponivel.\n";
    }

    if (selectedPlayer < enemyTextures.size() && !enemyTextures[selectedPlayer].empty()) {
        currentEnemyTextures = enemyTextures[selectedPlayer];
    } else {
        std::cerr << "Erro: Texturas de inimigo selecionadas nao disponiveis.\n";
    }

    if (selectedPlayer < gameMusics.size() && gameMusics[selectedPlayer]) {
        Mix_PlayMusic(gameMusics[selectedPlayer], -1);
    } else {
        std::cerr << "Erro: Musica de jogo selecionada nao disponivel.\n";
    }
}

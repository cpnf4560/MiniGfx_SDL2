#include "GameManager.h"
#include <algorithm>

GameManager GameManager::instance;

// Velocidades base (ajusta estes valores conforme necessário)
float GameManager::baseBulletSpeed = 600.0f;
float GameManager::baseEnemySpeed = 120.0f;
float GameManager::basePlayerSpeed = 150.0f;

// Velocidades atuais (começam iguais às base)
float GameManager::currentBulletSpeed = 600.0f;
float GameManager::currentEnemySpeed = 120.0f;
float GameManager::currentPlayerSpeed = 150.0f;

// Score
int GameManager::score = 0;
int GameManager::lastSpeedUpdate = 0;

GameManager& GameManager::getInstance() {
    return instance;
}

void GameManager::updateSpeeds() {
    int speedLevel = score / 10; // A cada 10 pontos aumenta a velocidade

    if (speedLevel > lastSpeedUpdate) {
        lastSpeedUpdate = speedLevel;

        // Incremento de velocidade (ajusta estes valores conforme necessário)
        float bulletIncrement = 80.0f * speedLevel;    // +80 por nível
        float enemyIncrement = 30.0f * speedLevel;     // +30 por nível
        float playerIncrement = 20.0f * speedLevel;    // +20 por nível

        currentBulletSpeed = baseBulletSpeed + bulletIncrement;
        currentEnemySpeed = baseEnemySpeed + enemyIncrement;
        currentPlayerSpeed = basePlayerSpeed + playerIncrement;

        // Limites máximos para evitar velocidades absurdas
        currentBulletSpeed = std::min(currentBulletSpeed, 1500.0f);
        currentEnemySpeed = std::min(currentEnemySpeed, 400.0f);
        currentPlayerSpeed = std::min(currentPlayerSpeed, 500.0f);
    }
}

void GameManager::addScore(int points) {
    score += points;
    updateSpeeds();
}

void GameManager::resetGame() {
    score = 0;
    lastSpeedUpdate = 0;
    currentBulletSpeed = baseBulletSpeed;
    currentEnemySpeed = baseEnemySpeed;
    currentPlayerSpeed = basePlayerSpeed;
}

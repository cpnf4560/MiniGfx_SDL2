#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

class GameManager {
public:
    static GameManager& getInstance();

    // Velocidades base
    static float baseBulletSpeed;
    static float baseEnemySpeed;
    static float basePlayerSpeed;

    // Velocidades atuais
    static float currentBulletSpeed;
    static float currentEnemySpeed;
    static float currentPlayerSpeed;

    // Score
    static int score;
    static int lastSpeedUpdate;

    void updateSpeeds();
    void addScore(int points);
    void resetGame();

    float getBulletSpeed() const { return currentBulletSpeed; }
    float getEnemySpeed() const { return currentEnemySpeed; }
    float getPlayerSpeed() const { return currentPlayerSpeed; }
    int getScore() const { return score; }

private:
    GameManager() {}
    static GameManager instance;
};

#endif // GAMEMANAGER_H

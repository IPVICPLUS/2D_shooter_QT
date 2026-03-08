#ifndef GAMEVIEW_H
#define GAMEVIEW_H
#pragma once
#include "enemy.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QKeyEvent>
#include <QTimer>
#include <QList>
#include <QElapsedTimer>
#include <QPushButton>
#include <QTimeLine>
#include <QGraphicsTextItem>
#include <QGraphicsRectItem>
#include <QSlider>
#include <QComboBox>
#include <QMouseEvent>
#include <QSet>

class Player;
class Enemy;
class Bullet;
class QGraphicsTextItem;
class HealthPickUp;
class Powerup;
class EnemyBullet;
class GameView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit GameView(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
        void keyReleaseEvent(QKeyEvent* event) override;

private slots:


    void tick();
    void startGameFromMenu();
    void quitFromMenu();
    void openSettings();



private:

     QSet<int> m_keysDown;

    void buildMenu();
    void showMenu();
    void showGame();
    void resetGame();
    void spawnEnemy();
    void updateHud();
    void cleanup();
    void spawnHealthPickup();
    void showSettings();
    void buildSettings();
    void spawnPowerup();
    void spawnEnemy(Enemy::Kind kind);
    void clearEnemies();

    QElapsedTimer m_bossPauseTimer;
    bool m_bossPauseActive = false;
    int m_bossPauseMs = 10000;


    // ---Menu animation ---

    QGraphicsTextItem* m_menuTitle = nullptr;
    QTimeLine* m_titleTimeLine = nullptr;

    // --- Menue preview actor (bakgrund) ---

    QGraphicsRectItem* m_previewPlayer = nullptr;
    QPointF m_previewVel = QPointF(2.2, 0.0);
    qreal m_previewBaseY = 420;
    qreal m_previewT = 0.0;

    // -----Scenes-----
    QGraphicsScene m_menuScene;
    QGraphicsScene m_gameScene;
    QGraphicsScene m_settingsScene;


    // -----Settings values-----
    int m_volume = 50; // 0-100
    enum class Difficulty {Easy, Normal, Hard};
    Difficulty m_difficulty = Difficulty::Normal;

    // -- settings widgets --
    QSlider* m_volumeSlider = nullptr;
    QComboBox* m_difficultyBox = nullptr;
    QPushButton* m_backBtn = nullptr;


    //game state
    QTimer m_timer;

    Player* m_player = nullptr;
    QList<Enemy*> m_enemies;
    QList<Bullet*> m_bullets;
    QList<HealthPickUp*> m_pickups;
    QList<Powerup*> m_powerups;
    QList<EnemyBullet*> m_enemyBullets;

    QElapsedTimer m_spawnTimer;
    int m_spawnEveryMs = 3000;
    int m_baseSpawnMs = 3000;
    int m_minSpawnMs = 600;

    QElapsedTimer m_pickupSpawnTimer;
    int m_pickupSpawnMs = 8000;

    QElapsedTimer m_powerupTimer;
    int m_powerupEveryMs = 8000;
    // hud
    QGraphicsTextItem* m_hud = nullptr;
    int m_score = 0;

    bool m_gameOver = false;
    bool m_bossSpawned = false;

    QPushButton* m_startBtn = nullptr;
    QPushButton* m_quitBtn = nullptr;
    QPushButton* m_settingsBtn = nullptr;

};

#endif // GAMEVIEW_H

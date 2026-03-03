#ifndef PLAYER_H
#define PLAYER_H
#pragma once

#include <QGraphicsRectItem>
#include <QKeyEvent>
#include <QSet>
#include <QRectF>
#include <QElapsedTimer>
#include <QPointF>
#include <QGraphicsEllipseItem>

class Bullet;


class Player : public QGraphicsRectItem
{
public:
    Player();

    void updateMovement(const QRectF& bounds);

    //hp
    int hp() const {return m_hp; }
    void takeDamage(int dmg);
    // Ammo
    int ammoInMag() const { return m_ammoInMag; }
    int magSize() const {return m_magSize; }
    bool isReloading() const { return m_reloading; }
    bool shieldActive() const { return m_shieldActive; }
    bool minigunActive() const { return m_minigunActive; }



    // skapa ett skott om möjligt returnerar Bullet* om den sköt annars nullptr
    Bullet* tryShoot();

    // starta en reload
    void startReload();

    // shield och minigun
    void activateShield(int durationMs);
    void activateMinigun(int durationMS);
    void updateVisuals();
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

private:

    QSet<int> m_keysDown;
    qreal m_speed = 4.0;

    QPointF m_aimDir= QPointF(1, 0); // senaste riktingen default är höger

    // skott

    QElapsedTimer m_shotTimer;
    int m_shotCooldownMs = 150;

    // reload
    int m_magSize = 12;
    int m_ammoInMag = 12;
    bool m_reloading = false;
    QElapsedTimer m_reloadTimer;
    int m_reloadMs = 800;

    // shield
    bool m_shieldActive = false;
    QElapsedTimer m_shieldTimer;
    int m_shieldMs = 0;
    QGraphicsEllipseItem* m_shieldRing = nullptr;
    // minigun
    bool m_minigunActive = false;
    QElapsedTimer m_minigunTimer;
    int m_miniGunMs = 0;

    int m_normalCooldownMs = 120;
    int m_miniGuncooldownMs = 45;
    // hp
    int m_hp = 5;

    QElapsedTimer m_damageTimer;
    int m_invulnMs = 600; // "i-Frames"
};

#endif // PLAYER_H

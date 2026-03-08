#ifndef ENEMY_H
#define ENEMY_H
#pragma once

#include <QGraphicsRectItem>
#include <QRectF>
#include <QPointF>
#include <QElapsedTimer>

class Enemy : public QGraphicsRectItem
{
public:
    enum class Kind{ Chaser, Shooter, Tank, MiniBoss};

    Enemy(Kind kind = Kind::Chaser);

    void stepTowards(const QPointF& target);
    void step(const QPointF& playerCenter); // update varje tic
    void hit(int dmg = 1 );
    bool isDead() const { return m_hp <= 0; }
    Kind kind() const {return m_kind; }
    void onShotFired() {m_shootTimer.restart(); }
    bool wantsToShoot() const;
    QPointF shootDirection(const QPointF& playerCenter) const;
    void updateVisuals();
    int maxHp() const{ return m_maxHp; }
    int hp() const{return m_hp; }
private:


    void moveTowards(const QPointF& playerCenter);


    //----READ BOSS HEALTHBAR----


    int m_maxHp = 2;

    Kind m_kind;

    int m_hp = 2;
    qreal m_speed = 1.7;

    //flashen
    QElapsedTimer m_flashTimer;
    int m_flashMs = 120;
    bool m_flashing = false;

    // shooting (för shooters/boss)
    QElapsedTimer m_shootTimer;
    int m_shootEveryMs = 900;

};

#endif // ENEMY_H

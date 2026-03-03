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
    Enemy();

    void stepTowards(const QPointF& target);
    void hit(); // när man blir träffad

    bool isDead() const { return m_hp <=0;}
    void updateVisuals();

private:
    int m_hp = 2;
    qreal m_speed = 1.7;
    QElapsedTimer m_flashTimer;
    int m_flashMs = 120;
    bool m_flashing = false;
};

#endif // ENEMY_H

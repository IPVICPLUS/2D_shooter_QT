#ifndef BULLET_H
#define BULLET_H
#pragma once
#include <QGraphicsRectItem>

class Bullet : public QGraphicsRectItem
{
public:
    explicit Bullet(qreal vx, qreal vy);

    void step();

private:
    qreal m_vx = 0;
    qreal m_vy = 0;
};

#endif // BULLET_H

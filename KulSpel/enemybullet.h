#ifndef ENEMYBULLET_H
#define ENEMYBULLET_H

#include <QGraphicsRectItem>

class enemyBullet : public QGraphicsRectItem
{
public:
    enemyBullet(qreal vx, qreal vy);

    void step();

private:
    qreal m_vx, m_vy;
};

#endif // ENEMYBULLET_H

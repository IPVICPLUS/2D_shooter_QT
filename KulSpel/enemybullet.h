#ifndef ENEMYBULLET_H
#define ENEMYBULLET_H

#include <QGraphicsRectItem>

class EnemyBullet : public QGraphicsRectItem
{
public:
    EnemyBullet(qreal vx, qreal vy);

    void step();

private:
    qreal m_vx, m_vy;
};

#endif // ENEMYBULLET_H

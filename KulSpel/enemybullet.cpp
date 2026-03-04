#include "enemybullet.h"

#include <QBrush>
#include <QPen>

EnemyBullet::EnemyBullet(qreal vx, qreal vy) : m_vx(vx), m_vy(vy)
{
    setRect(-4, -2, 8, 4);
    setBrush(QBrush(QColor(255, 230, 120)));
    setPen(Qt::NoPen);
}

void EnemyBullet::step()
{
    moveBy(m_vx, m_vy);
}

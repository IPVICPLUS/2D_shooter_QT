#include "bullet.h"
#include <QBrush>
#include <QPen>
Bullet::Bullet(qreal vx, qreal vy)
    : m_vx(vx), m_vy(vy)
{
    setRect(0, 0, 10, 4);
    setBrush(QBrush(QColor(255, 220, 120)));
    setPen(Qt::NoPen);
}

void Bullet::step()
{
    moveBy(m_vx, m_vy);
}

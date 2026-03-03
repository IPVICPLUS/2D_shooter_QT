#include "enemy.h"
#include <QBrush>
#include <QPen>
#include <cmath>
Enemy::Enemy()
{
    setRect(0, 0, 36, 36);
    setBrush(QBrush(QColor(255, 90, 90)));
    setPen(Qt::NoPen);
}

void Enemy::stepTowards(const QPointF& target)
{


    QPointF me = sceneBoundingRect().center();
    QPointF d = target - me;

    qreal len = std::sqrt(d.x()*d.x() + d.y()*d.y());
    if(len < 0.001) return;

    d /= len; // normalisera
    moveBy(d.x() * m_speed, d.y() * m_speed);


}

void Enemy::hit()
{
    m_hp--;
    m_flashing = true;
    m_flashTimer.restart();
    setBrush(QBrush(QColor(255, 255, 255)));
    // byt färg lite när den tar skada (enkel feedback)

}

void Enemy::updateVisuals()
{
    if(m_flashing && m_flashTimer.elapsed() >= m_flashMs){
        m_flashing = false;

        // återställ färg baserat på hp
        if(m_hp >= 2) setBrush(QBrush(QColor(255, 90, 90)));
        else if( m_hp == 1) setBrush(QBrush(QColor(255, 160, 160)));
    }
}

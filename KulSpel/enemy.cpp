#include "enemy.h"
#include <QBrush>
#include <QPen>
#include <cmath>

static QPointF norm(const QPointF& v){
    double len = std::sqrt(v.x()*v.x() + v.y()*v.y());
    if (len < 0.001) return QPointF(1,0);
    return QPointF(v.x()/len, v.y()/len);
}
Enemy::Enemy(Kind kind) : m_kind(kind)
{
    setPen(Qt::NoPen);

    // default stats per typ
    switch(m_kind){
    case Kind::Chaser:
        setRect(0,0,36,36);
        setBrush(QBrush(QColor(255,90,90)));
        m_hp = 2; m_speed = 1.8;
        break;


    case Kind::Shooter:
        setRect(0,0,34,34);
        setBrush(QBrush(QColor(255,144,60)));
        m_hp = 2; m_speed = 1.2;
        m_shootEveryMs = 900;
        m_shootTimer.start();
        break;

    case Kind::Tank:
        setRect(0,0,44,44);
        setBrush(QBrush(QColor(200, 60, 220)));
        m_hp = 6; m_speed = 0.8;
        break;

    case Kind::Miniboss:
        setRect(0,0,80,80);
        setBrush(QBrush(QColor(130,60,255)));
        m_hp = 40; m_speed = 0.7;
        m_shootEveryMs = 400;
        m_shootTimer.start();
        break;


    }
}



void Enemy::hit(int dmg)
{
    m_hp -= dmg;
    m_flashing = true;
    m_flashTimer.restart();
    setBrush(QBrush(Qt::white));
    // byt färg lite när den tar skada (enkel feedback)

}

void Enemy::updateVisuals()
{
    if(m_flashing && m_flashTimer.elapsed() >= m_flashMs){
        m_flashing = false;

        // återställ färg baserat på typ
        switch(m_kind){
        case Kind::Chaser: setBrush(QBrush(QColor(255,90,90))); break;
        case Kind::Shooter: setBrush(QBrush(QColor(255,140,60))); break;
        case Kind::Tank: setBrush(QBrush(QColor(200,60,220))); break;
        case Kind::Miniboss: setBrush(QBrush(QColor(130,60,255))); break;
        }
    }
}

void Enemy::moveTowards(const QPointF& PlayerCenter){

}

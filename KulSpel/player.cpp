#include "player.h"
#include "bullet.h"

#include <cmath>
#include <QBrush>
#include <QPen>
#include <QGraphicsScene>
Player::Player()
{
    setRect(0, 0, 40, 40);
    setBrush(QBrush(QColor(80, 160, 255)));
    setPen(Qt::NoPen);

    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setFocus();

    m_shieldRing = new QGraphicsEllipseItem(-8, -8, 56, 56, this);
    m_shieldRing->setPen(QPen(QColor(120, 180, 255), 3));
    m_shieldRing->setBrush(Qt::NoBrush);
    m_shieldRing->setVisible(false);
    m_shieldRing->setZValue(-1);


    m_shotTimer.start();
    m_reloadTimer.start();
    m_damageTimer.start();



}

void Player :: takeDamage(int dmg)
{
    if(dmg >0){
        if(m_shieldActive) return;

        if(m_damageTimer.elapsed() < m_invulnMs) return;
        m_damageTimer.restart();

    }

    m_hp-= dmg;

    if(m_hp < 0) m_hp = 0;
    if(m_hp >15) m_hp = 15;

    /*// lite visuell feedback
    setBrush(QBrush(QColor(80, 160, 255)).color().lighter(150));
    //snabb reset kan göras senare med timer låter det vara eneklt just nu
    setBrush(QBrush(QColor(80, 160, 255)));*/
}

void Player::startReload()
{
    if(m_reloading) return;
    if(m_ammoInMag == m_magSize) return; // redan full

    m_reloading = true;
    m_reloadTimer.restart();

}

void Player::activateShield(int durationMs)
{
    m_shieldActive = true;
    m_shieldMs = durationMs;
    m_shieldTimer.restart();
}

void Player::activateMinigun(int durationMs)
{
    m_minigunActive = true;
    m_miniGunMs = durationMs;
    m_minigunTimer.restart();

}

Bullet* Player::tryShoot()
{
    if(!scene()) return nullptr;

    // Reload låser skjutningen
    if(m_reloading) return nullptr;

    //cooldown
    int cooldown = m_minigunActive ? m_miniGuncooldownMs : m_normalCooldownMs;
    if (m_shotTimer.elapsed() < cooldown) return nullptr;
    // ammo
    if(m_ammoInMag <= 0) return nullptr;

    m_shotTimer.restart();
    m_ammoInMag--;

    // riktning normaliserad
    QPointF d = m_aimDir;
    qreal len = std::sqrt(d.x()*d.x() + d.y()*d.y());
    if(len < 0.001) d = QPointF(1,0);
    else d/= len;

    const qreal speed = 10.0;
    auto* b = new Bullet(d.x()*speed, d.y()*speed);

    QPointF start = sceneBoundingRect().center();
    b->setPos(start.x() + d.x() *25, start.y() + d.y()*25);

    return b;
}
void Player::keyPressEvent(QKeyEvent* event){
    m_keysDown.insert(event->key());

    if(event->key() == Qt::Key_R){
        startReload();
    }
    event->accept();
}

void Player::keyReleaseEvent(QKeyEvent* event){
    m_keysDown.remove(event->key());
    event->accept();
}

void Player::updateVisuals()
{
    if(m_shieldRing){
        m_shieldRing->setVisible(m_shieldActive);
    }
    if(m_minigunActive)
        setBrush(QBrush(QColor(255, 220, 120)));
    else
        setBrush(QBrush(QColor(80, 160, 255)));
}


void Player::updateMovement(const QRectF& bounds)
{
    // shield expire
    if(m_shieldActive && m_shieldTimer.elapsed() >= m_shieldMs){
        m_shieldActive = false;
    }

    if(m_minigunActive && m_minigunTimer.elapsed() >= m_miniGunMs){
        m_minigunActive = false;
    }
    if(m_reloading && m_reloadTimer.elapsed() >= m_reloadMs){
        m_ammoInMag = m_magSize;
        m_reloading = false;
    }

    qreal dx = 0;
    qreal dy = 0;

    if(m_keysDown.contains(Qt::Key_Left) || m_keysDown.contains(Qt::Key_A)) dx -= m_speed;
    if(m_keysDown.contains(Qt::Key_Right) || m_keysDown.contains(Qt::Key_D)) dx += m_speed;
    if(m_keysDown.contains(Qt::Key_Up) || m_keysDown.contains(Qt::Key_W)) dy -= m_speed;
    if(m_keysDown.contains(Qt::Key_Down) || m_keysDown.contains(Qt::Key_S)) dy = m_speed;

    // uppdatera aimDir till senaste input ( om någon)
    if(dx != 0  || dy !=0)
        m_aimDir = QPointF(dx, dy);
    moveBy(dx, dy);

    // clamp inom scenen

    QRectF r = sceneBoundingRect();
    QPointF p = pos();

    if(r.left() < bounds.left()) p.setX(bounds.left());
    if(r.right() > bounds.right()) p.setX(bounds.right()- rect().width());
    if(r.top() < bounds.top()) p.setY(bounds.top());
    if(r.bottom() > bounds.bottom()) p.setY(bounds.bottom() - rect().height());

    updateVisuals();

    setPos(p);
}

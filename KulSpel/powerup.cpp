#include "powerup.h"
#include <QBrush>
#include <QPen>

Powerup::Powerup(Type t) : m_type(t)
{
    setRect(0, 0, 26, 26);
    setPen(Qt::NoPen);

    // färg per powerup .. går att byta till sprites senare
    if (t == Type::Health) setBrush(QBrush(QColor(100, 255, 100)));
    if (t == Type::Shield) setBrush(QBrush(QColor(120, 180, 255)));
    if (t == Type::Minigun) setBrush(QBrush(QColor(255, 220, 120)));
}

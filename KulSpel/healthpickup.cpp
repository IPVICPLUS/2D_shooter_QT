#include "healthpickup.h"
#include <QBrush>
#include <QPen>
HealthPickUp::HealthPickUp()
{
    setRect(0, 0, 25,25);
    setBrush(QBrush(QColor(100, 255, 100)));
    setPen(Qt::NoPen);
}

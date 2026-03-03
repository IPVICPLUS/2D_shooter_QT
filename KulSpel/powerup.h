#ifndef POWERUP_H
#define POWERUP_H
#pragma once

#include <QGraphicsRectItem>

class Powerup : public QGraphicsRectItem
{
public:
    enum class Type{ Health, Shield, Minigun };

    explicit Powerup(Type t);
    Type powerupType() const {return m_type;   }

private:
    Type m_type;
};

#endif // POWERUP_H

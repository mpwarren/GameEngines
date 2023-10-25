#ifndef DEATHZONE_H
#define DEATHZONE_H

#include "CollidableObject.h"

class DeathZone : public CollidableObject{
    public:
        DeathZone(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath);
};

#endif
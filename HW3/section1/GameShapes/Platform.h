#ifndef PLATFORM_H
#define PLATFORM_H

#include <SFML/Graphics.hpp>
#include "CollidableObject.h"

class Platform : public CollidableObject{
    public:
        Platform();
        Platform(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath);
};
#endif
#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "Platform.h"
#include "CollidableObject.h"

class Player : public CollidableObject{
    public:
        Player(sf::Vector2f size, sf::Vector2f startPosition, std::string texturePath);
        void movePlayer(sf::Keyboard::Key);
};
#endif
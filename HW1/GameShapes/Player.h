#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "Platform.h"
#include "Collider.h"

class Player : public sf::RectangleShape{
    public:
        Player(sf::Vector2f size, sf::Vector2f startPosition);
        void movePlayer(sf::Keyboard::Key, Platform platform);

};
#endif
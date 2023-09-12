#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "Platform.h"

class Player : public sf::CircleShape{
    public:
        Player(float radius, sf::Vector2f startPosition);
        void movePlayer(sf::Keyboard::Key, Platform platform);

    private:
        bool collidedDown;
        bool collidedUp;
        bool collidedLeft;
        bool collidedRight;

};
#endif
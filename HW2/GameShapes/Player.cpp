#include "Player.h"
#include <iostream>

Player::Player(sf::Vector2f size, sf::Vector2f startPosition, std::string texturePath) : CollidableObject(size, startPosition, texturePath){
    setPosition(startPosition);
}

void Player::movePlayer(sf::Keyboard::Key key, Platform platform){
    int xDir = 0;
    int yDir = 0;

    if(key == sf::Keyboard::W){
        yDir = -1;
    }
    else if(key == sf::Keyboard::S){
        yDir = 1;
    }
    else if(key == sf::Keyboard::A){
        xDir = -1;
    }
    else if(key == sf::Keyboard::D){
        xDir = 1;
    }

    move(xDir, yDir);

    CheckCollision(platform);
}
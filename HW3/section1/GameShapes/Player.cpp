#include "Player.h"
#include <iostream>

Player::Player(int id, sf::Vector2f size, sf::Vector2f startPosition, std::string texturePath) : CollidableObject(id, size, startPosition, texturePath){
    setPosition(startPosition);
    objId = "PL";
}

void Player::movePlayer(sf::Keyboard::Key key, int64_t frameDelta){
    int xDir = 0;
    int yDir = 0;

    int velocity = 1 * frameDelta;

    if(key == sf::Keyboard::W){
        yDir = -1 * velocity;
    }
    else if(key == sf::Keyboard::S){
        yDir = 1 * velocity;
    }
    else if(key == sf::Keyboard::A){
        xDir = -1 * velocity;
    }
    else if(key == sf::Keyboard::D){
        xDir = 1 * velocity;
    }

    move(xDir, yDir);
}
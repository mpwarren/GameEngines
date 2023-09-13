#include "Player.h"
#include <iostream>

Player::Player(sf::Vector2f size, sf::Vector2f startPosition) : sf::RectangleShape(size){
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

    if(!getGlobalBounds().intersects(platform.getGlobalBounds())){
        move(xDir, yDir);
    }
    else{ //there is collision
        if(key == sf::Keyboard::S && !platform.getGlobalBounds().contains(sf::Vector2f(getPosition().x, getPosition().y + 2 * getRadius()))){
            move(xDir, yDir);
        }
        else if (key != sf::Keyboard::S){
            move(xDir, yDir);
        }
    }
}
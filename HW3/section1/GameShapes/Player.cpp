#include "Player.h"
#include <iostream>

Player::Player(int id, sf::Vector2f size, sf::Vector2f startPosition, std::string texturePath) : CollidableObject(id, size, startPosition, texturePath){
    setPosition(startPosition);
    objId = PLAYER_ID;
    jumpHeight = 200;
}

void Player::movePlayer(sf::Keyboard::Key key, int64_t frameDelta){
    int xDir = 0;
    int yDir = 0;

    int velocity = frameDelta;

    if(key == sf::Keyboard::A){
        xDir = -1 * velocity;
    }
    else if(key == sf::Keyboard::D){
        xDir = 1 * velocity;
    }

    move(xDir, yDir);

}

void Player::gravity(int64_t frameDelta){
    int velocity = frameDelta;
    if(!jumping){
        move(0, velocity);
    }
    else{
        move(0, -1 * velocity);
        if(getPosition().y <= jumpPeak){
            jumping = false;
            jumpPeak = 0;
        }
    }
}

bool Player::isJumping(){
    return jumping;
}

void Player::setJumping(){
    if(!jumping && colliding){
        jumping = true;
        jumpPeak = getPosition().y - jumpHeight;
    }
}

void Player::setColliding(bool isColliding){
    colliding = isColliding;
    if(jumping && colliding){
        //end the jump if it hits something
        jumping = false;
        jumpPeak = 0;
    }
}
#include "Player.h"
#include <iostream>

Player::Player(int id, sf::Vector2f size, sf::Vector2f startPosition, std::string texturePath) : CollidableObject(id, size, startPosition, texturePath){
    setPosition(startPosition);
    objId = PLAYER_ID;
    jumpHeight = 200;
}

void Player::movePlayer(sf::Keyboard::Key key, int64_t frameDelta){
    float xDir = 0;
    float yDir = 0;

    float velocity = 0.5;

    if(key == sf::Keyboard::A){
        xDir = -1 * velocity * frameDelta;
    }
    else if(key == sf::Keyboard::D){
        xDir = 1 * velocity * frameDelta;
    }

    move(xDir, yDir);

}

void Player::gravity(int64_t frameDelta, bool groundLevel){
    int velocity = frameDelta;
    if(!jumping && !groundLevel){
        move(0, velocity);
    }
    else if(jumping){
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
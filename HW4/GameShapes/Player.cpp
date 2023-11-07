#include "Player.h"
#include <iostream>

Player::Player(int id, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath) : CollidableObject(id, size, position, startingPosition, texturePath){
    setPosition(position);
    objId = PLAYER_ID;
    jumpHeight = 200;
}

void Player::movePlayer(char key, int64_t frameDelta){
    float xDir;

    float velocity = 0.5;

    if(key == 'A'){
        xDir = -1 * velocity * frameDelta;
    }
    else if(key == 'D'){
        xDir = 1 * velocity * frameDelta;
    }

    move(xDir, 0);

}

bool Player::gravity(int64_t frameDelta){
    float velocity = 0.5;

    if(jumping){
        move(0, -1 * velocity * frameDelta);
        
        if(getPosition().y <= jumpPeak){
            jumping = false;
            jumpPeak = 0;
        }
        return true;
    }
    else if(!isCollidingUnder){
        move(0, velocity * frameDelta);
        return true;
    }

    return false;

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

void Player::setIsCollidingUnder(bool iCU){
    isCollidingUnder = iCU;
}

bool Player::checkCollision(CollidableObject* other){
    return getGlobalBounds().intersects(other->getGlobalBounds());
}
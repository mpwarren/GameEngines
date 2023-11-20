#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "Platform.h"
#include "CollidableObject.h"

class Player : public CollidableObject{
    public:
        Player(int id, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath);
        void movePlayer(char key, int64_t frameDelta, bool sprint);
        bool isJumping();
        void setJumping();
        void jump();
        bool gravity(int64_t frameDelta);
        void setColliding(bool isColliding);
        void setIsCollidingUnder(bool iCU);
        bool checkCollision(CollidableObject* other);


    private:
        bool jumping;
        bool colliding;
        bool isCollidingUnder;
        float jumpHeight;
        float jumpPeak;
};
#endif
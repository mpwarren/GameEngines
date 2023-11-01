#ifndef MOVINGPLATHFORM_H
#define MOVINGPLATHFORM_H

#include <SFML/Graphics.hpp>
#include "Platform.h"

enum Direction{
    vertical = 0,
    horizontal = 1
};

class MovingPlatform : public Platform{
    public:
        Direction platformDirection;
        MovingPlatform(sf::Vector2f size, sf::Vector2f position, std::string texturePath, Direction dir, float v, int dist);
        void movePosition(int64_t frameDelta);

    private:
        float velocity;
        int distance;
        int currentlyMoving = 1;
        sf::Vector2f startPoint;
        sf::Vector2f endPoint;

};
#endif
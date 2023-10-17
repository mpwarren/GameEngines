#include "MovingPlatform.h"

MovingPlatform::MovingPlatform() : Platform(){

}


MovingPlatform::MovingPlatform(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath, Direction dir, float v, int dist) : Platform(id, size, position, texturePath), platformDirection{dir}, velocity{v}, distance{dist}{
    startPoint = getPosition();
    if(dir == Direction::horizontal){
        endPoint = sf::Vector2f(startPoint.x + distance, startPoint.y);
    }
    else if(dir == Direction::vertical){
        endPoint = sf::Vector2f(startPoint.x, startPoint.y + distance);
    }
}


void MovingPlatform::movePosition(int64_t frameDelta){
    sf::Vector2f currentPosition = getPosition();
    if(platformDirection == Direction::horizontal){
        if(currentPosition.x > endPoint.x || currentPosition.x < startPoint.x){
            currentlyMoving *= -1;
        }
        move(currentlyMoving * velocity * frameDelta, 0);
    }
    else if(platformDirection == Direction::vertical){
        if(currentPosition.y > endPoint.y || currentPosition.y < startPoint.y){
            currentlyMoving *= -1;
        }
        move(0, currentlyMoving * velocity * frameDelta);
    }
}


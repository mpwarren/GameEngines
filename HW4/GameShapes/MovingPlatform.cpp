#include "MovingPlatform.h"

MovingPlatform::MovingPlatform() : Platform(){

}


MovingPlatform::MovingPlatform(int id, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath, Direction dir, float v, int dist) : Platform(id, size, position, startingPosition, texturePath), platformDirection{dir}, velocity{v}, distance{dist}{
    startPoint = getPosition();
    setEndPoint();
    objId = MOVING_PLATFORM_ID;
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

std::string MovingPlatform::toString(){
    return objId + " " + std::to_string(id) + " " + std::to_string(getSize().x) + " " + std::to_string(getSize().y)
        + " " + std::to_string(getPosition().x) + " " + std::to_string(getPosition().y) + " " + std::to_string(startingPoint.x) 
        + " " + std::to_string(startingPoint.y) + " " + classTexturePath + " " + std::to_string((int)platformDirection) + 
        " " + std::to_string(velocity) + " " + std::to_string(distance);
}

void MovingPlatform::translate(std::string dir, int64_t frameDelta){
    float distMoved = velocity * frameDelta;
    if(dir == TRANSFORM_LEFT){
        startPoint.x = startPoint.x - distMoved;
        endPoint.x = endPoint.x - distMoved;
        setPosition(getPosition().x - distMoved , getPosition().y);
    }
    else{
        startPoint.x = startPoint.x + distMoved;
        endPoint.x = endPoint.x + distMoved;
        setPosition(getPosition().x + distMoved , getPosition().y);
    }

}

void MovingPlatform::reset(){
    setPosition(startingPoint);
    startPoint = startingPoint;
    setEndPoint();

}

void MovingPlatform::setEndPoint(){
    if(platformDirection == Direction::horizontal){
        endPoint = sf::Vector2f(startPoint.x + distance, startPoint.y);
    }
    else if(platformDirection == Direction::vertical){
        endPoint = sf::Vector2f(startPoint.x, startPoint.y + distance);
    }
}

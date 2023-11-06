#include "CollidableObject.h"


CollidableObject::CollidableObject() : sf::RectangleShape(sf::Vector2f(0,0)), id{-1}  {
    setPosition(sf::Vector2f(0,0));
    classTexturePath = "n/a";
    applyTexture(classTexturePath);
}

CollidableObject::CollidableObject(int passedId, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath) : sf::RectangleShape(size), id{passedId} {
    setPosition(position);
    startingPoint = startingPosition;
    classTexturePath = texturePath == "" ? "n/a" : texturePath;
    applyTexture(classTexturePath);
    objId = "CO";
}

void CollidableObject::applyTexture(std::string filePath){
    if(filePath != "n/a"){
        if(texture.loadFromFile(filePath)){
            setTexture(&texture);
        }
        else{
            std::cout << "Failed to load texture!\n" << std::flush;
        }
    }
}

/*
    AABB collision detection
    returns true if the collision is under the called object
*/
bool CollidableObject::resolveColision(CollidableObject* other){

    sf::Vector2f otherPos = other->getPosition();
    sf::Vector2f otherSize = other->getSize();
    sf::Vector2f thisPos = getPosition();
    sf::Vector2f thisSize = getSize();

    //distance from top of player to bottom of other obj
    float colBot = otherPos.y + otherSize.y - thisPos.y;
    //distance from bottom of player to top of other obj
    float colTop = thisPos.y + thisSize.y - otherPos.y;
    //distance from right of player to left of other obj
    float colLeft = thisPos.x + thisSize.x - otherPos.x;
    //distance from left of player to right of other obj
    float colRight = otherPos.x + otherSize.x - thisPos.x;

    //find smallest distance of those 4 to find where collision is
    float min = std::min({colBot, colTop, colLeft, colRight});
    
    //move player to right outside of the colliding side
    if(min == colTop){
        setPosition(thisPos.x, otherPos.y - thisSize.y);
        return true;
    }
    else if(min == colBot){
        setPosition(thisPos.x, otherPos.y + otherSize.y);
    }
    else if(min == colRight){
        setPosition(otherPos.x + otherSize.x, thisPos.y);
    }
    else if(min == colLeft){
        setPosition(otherPos.x - thisSize.x, thisPos.y);
    }
    return false;
        
    
}

std::string CollidableObject::toString(){
    return objId + " " + std::to_string(id) + " " + std::to_string(getSize().x) + " " + std::to_string(getSize().y)
        + " " + std::to_string(getPosition().x) + " " + std::to_string(getPosition().y) + " " + std::to_string(startingPoint.x) + 
        " " + std::to_string(startingPoint.y) + " " + classTexturePath;
}

void CollidableObject::translate(std::string dir, int64_t frameDelta){
    float velocity = 0.5;
    float distMoved = velocity * frameDelta;
    if(dir == TRANSFORM_LEFT){
        setPosition(getPosition().x - distMoved , getPosition().y);
    }
    else{
        setPosition(getPosition().x + distMoved , getPosition().y);
    }
}

void CollidableObject::reset(){
    setPosition(startingPoint);
}
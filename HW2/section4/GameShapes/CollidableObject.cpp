#include "CollidableObject.h"

CollidableObject::CollidableObject(sf::Vector2f size, sf::Vector2f position, std::string texturePath) : sf::RectangleShape(size){
    setPosition(position);
    applyTexture(texturePath);
}

void CollidableObject::applyTexture(std::string filePath){
    if(!filePath.empty()){
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
*/
void CollidableObject::resolveColision(CollidableObject* other){

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
        
    
}
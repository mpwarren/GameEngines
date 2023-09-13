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
    algorithm help from:
    - https://tutorialedge.net/gamedev/aabb-collision-detection-tutorial/
    - https://stackoverflow.com/questions/5062833/detecting-the-direction-of-a-collision
*/
bool CollidableObject::CheckCollision(CollidableObject other){

    sf::Vector2f otherPos = other.getPosition();
    sf::Vector2f otherSize = other.getSize();
    sf::Vector2f thisPos = getPosition();
    sf::Vector2f thisSize = getSize();

    if(thisPos.x < otherPos.x + otherSize.x && thisPos.x + thisSize.x > otherPos.x && thisPos.y < otherPos.y + otherSize.y && thisPos.y + thisSize.y > otherPos.y){
        
        //find the closest distance from a side of the player to a side of the other object
        float botPlayer = getPosition().y + getSize().y;
        float botPlat = other.getPosition().y + other.getSize().y;
        float rightPlayer = getPosition().x + getSize().x;
        float rightPlat = other.getPosition().x + other.getSize().x;

        float colBot = botPlat - getPosition().y;
        float colTop = botPlayer - other.getPosition().y;
        float colLeft = rightPlayer - other.getPosition().x;
        float colRight = rightPlat - getPosition().x;

        float min = std::min({colBot, colTop, colLeft, colRight});

        if(min == colTop){
            setPosition(getPosition().x, other.getPosition().y - getSize().y);
        }
        else if(min == colBot){
            setPosition(getPosition().x, botPlat);
        }
        else if(min == colRight){
            setPosition(rightPlat, getPosition().y);
        }
        else if(min == colLeft){
            setPosition(other.getPosition().x - getSize().x, getPosition().y);
        }
        

        return true;
    }
    return false;
    
}
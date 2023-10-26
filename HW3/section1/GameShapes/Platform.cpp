#include "Platform.h"
#include <iostream>

Platform::Platform() : CollidableObject(-1, sf::Vector2f(0,0), sf::Vector2f(0,0), sf::Vector2f(0,0), ""){} 

Platform::Platform(int id, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath) : CollidableObject(id, size, position, startingPosition, texturePath){
    objId = PLATFORM_ID;
}



#include "Platform.h"
#include <iostream>

Platform::Platform() : CollidableObject(-1, sf::Vector2f(0,0), sf::Vector2f(0,0), ""){
}

Platform::Platform(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath) : CollidableObject(id, size, position, texturePath){
    objId = "PT";
}



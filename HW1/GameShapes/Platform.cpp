#include "Platform.h"
#include <iostream>

Platform::Platform(sf::Vector2f size, sf::Vector2f position, std::string texturePath) : sf::RectangleShape(size){
    setPosition(position);
    applyTexture(texturePath);
}

void Platform::applyTexture(std::string filePath){
    if(!filePath.empty()){
        if(texture.loadFromFile(filePath)){
            setTexture(&texture);
        }
        else{
            std::cout << "Failed to load texture!\n" << std::flush;
        }
    }
}

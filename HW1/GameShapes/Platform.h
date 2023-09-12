#ifndef PLATFORM_H
#define PLATFORM_H

#include <SFML/Graphics.hpp>
#include <cstddef>

class Platform : public sf::RectangleShape{
    public:
        Platform(sf::Vector2f size, sf::Vector2f position, std::string texturePath = NULL);
        void applyTexture(std::string filePath);
    
    private:
        sf::Texture texture;

};
#endif
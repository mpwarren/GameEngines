#ifndef COLLIDER_H
#define COLLIDER_H

#include <SFML/Graphics.hpp>
#include <iostream>

class CollidableObject : public sf::RectangleShape{

    public:

        CollidableObject(sf::Vector2f size, sf::Vector2f position, std::string texturePath);

        bool CheckCollision(CollidableObject other);
        
        sf::Vector2f GetHalfSize();

        void applyTexture(std::string filePath);
    
    private:
        sf::Texture texture;

};


#endif


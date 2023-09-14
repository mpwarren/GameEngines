#ifndef COLLIDER_H
#define COLLIDER_H

#include <SFML/Graphics.hpp>
#include <iostream>

class CollidableObject : public sf::RectangleShape{

    public:

        CollidableObject(sf::Vector2f size, sf::Vector2f position, std::string texturePath);

        bool CheckCollision(CollidableObject other);

        void applyTexture(std::string filePath);
    
    private:
        sf::Texture texture;

};


#endif


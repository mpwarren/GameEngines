#ifndef COLLIDER_H
#define COLLIDER_H

#include <SFML/Graphics.hpp>
#include <iostream>

class CollidableObject : public sf::RectangleShape{

    public:
        int id;

        CollidableObject(int passedId, sf::Vector2f size, sf::Vector2f position, std::string texturePath);

        void resolveColision(CollidableObject* other);

        void applyTexture(std::string filePath);
    
    private:
        sf::Texture texture;

};


#endif


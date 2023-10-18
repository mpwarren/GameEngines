#ifndef COLLIDER_H
#define COLLIDER_H

#include <SFML/Graphics.hpp>
#include <iostream>

class CollidableObject : public sf::RectangleShape{

    public:
        int id;

        std::string objId;

        std::string classTexturePath;

        CollidableObject();

        CollidableObject(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath);

        void resolveColision(CollidableObject* other);

        void applyTexture(std::string filePath);

        virtual std::string toString();
    
    private:
        sf::Texture texture;

};


#endif


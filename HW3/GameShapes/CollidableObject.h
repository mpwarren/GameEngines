#ifndef COLLIDER_H
#define COLLIDER_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include "../Constants.h"

class CollidableObject : public sf::RectangleShape{

    public:
        int id;

        std::string objId;

        std::string classTexturePath;

        sf::Vector2f startingPoint;

        CollidableObject();

        CollidableObject(int id, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath);

        void resolveColision(CollidableObject* other);

        void applyTexture(std::string filePath);

        virtual std::string toString();

        virtual void translate(std::string dir, int64_t frameDelta);

        virtual void reset();
    
    private:
        sf::Texture texture;

};


#endif


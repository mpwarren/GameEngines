#ifndef PLAYER_H
#define PLAYER_H

#include <SFML/Graphics.hpp>
#include "CollidableObject.h"
#include "Constants.h"

class Player : public CollidableObject{
    public:
        Player(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath);
        void movePlayer(char key, int64_t frameDelta);
};

#endif
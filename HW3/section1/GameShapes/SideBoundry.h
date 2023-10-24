#ifndef BOUNDRY_H
#define BOUNDRY_H

#include "CollidableObject.h"

class SideBoundry : public CollidableObject{
    public:
        SideBoundry(int id, sf::Vector2f position, std::string thisSide);
        std::string side;
        bool isCollidedWith;
};


#endif
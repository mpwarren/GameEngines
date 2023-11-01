#ifndef SPAWN_H
#define SPAWN_H

#include <SFML/Graphics.hpp>
#include "Constants.h"

class SpawnPoint{
    public:
        SpawnPoint(sf::Vector2f spawn, sf::Vector2f startPoint);
        sf::Vector2f getSpawnPoint();
        std::string getObjId();
        std::string toString();
        void translate(std::string dir, int64_t frameDelta);
        void reset();

    private:
        std::string objId;
        sf::Vector2f spawnPoint;
        sf::Vector2f startingPoint;
};

#endif
#ifndef SPAWN_H
#define SPAWN_H

#include <SFML/Graphics.hpp>

class SpawnPoint{
    public:
        SpawnPoint(sf::Vector2f spawn);
        sf::Vector2f getSpawnPoint();
        std::string getObjId();
        std::string toString();

    private:
        std::string objId;
        sf::Vector2f spawnPoint;
};

#endif
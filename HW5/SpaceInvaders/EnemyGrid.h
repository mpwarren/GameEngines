#ifndef ENEMY_GRID_H
#define ENEMY_GRID_H

#include "CollidableObject.h"
#include <vector>
#include <mutex>
#include "Constants.h"

class Enemy : public CollidableObject{
    public:
        Enemy(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath, int r, int c);
        bool dead;
        int row;
        int col;
};

class EnemyGrid{
    public:
        EnemyGrid(int w, int h, int s);
        int width;
        int height;
        int spacing;        
        std::vector<std::vector<Enemy*>> enemyGrid;
        std::string toString();
};

#endif
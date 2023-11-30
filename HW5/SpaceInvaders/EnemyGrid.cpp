#include "EnemyGrid.h"

Enemy::Enemy(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath, int r, int c) : CollidableObject(id, size, position, texturePath), row{r}, col{c}{
    dead = false;
}

EnemyGrid::EnemyGrid(int w, int h, int s) : width{w}, height{h}, spacing{s}{
    int enemyWidth = 50;
    int startingX = 50;
    int startingY = 50;
    int currentX = startingX;
    for(int i = 0; i < height; i++){
        enemyGrid.push_back(std::vector<Enemy*>());
        for(int j = 0; j < width; j++){
            enemyGrid[i].push_back(new Enemy(-1, sf::Vector2f(enemyWidth, 20), sf::Vector2f(currentX, startingY), "", i, j));
            currentX += enemyWidth + spacing;
        }
        currentX = startingX;
        startingY *= 2;
    }
}

std::string EnemyGrid::toString(){
    return ENEMY_GRID_ID + " " + std::to_string(width) + " " + std::to_string(height) + " " + std::to_string(spacing);
}
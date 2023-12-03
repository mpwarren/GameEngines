#include "EnemyBullet.h"

EnemyBullet::EnemyBullet(float xPosition, float yPosition) : CollidableObject(-1, sf::Vector2f(2, 15), sf::Vector2f(xPosition, yPosition), ""){

}

void EnemyBullet::moveBullet(int64_t frameDelta){
    move(0, frameDelta);
}
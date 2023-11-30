#include "Bullet.h"

Bullet::Bullet(float xPosition) : CollidableObject(-1, sf::Vector2f(2, 15), sf::Vector2f(xPosition + 9, 500), ""){

}

void Bullet::moveBullet(int64_t frameDelta){
    move(0, -frameDelta);
}
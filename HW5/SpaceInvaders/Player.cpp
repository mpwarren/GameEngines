#include "Player.h"

Player::Player(int id, sf::Vector2f size, sf::Vector2f position, std::string texturePath) : CollidableObject(id, size, position, texturePath){
    objId = PLAYER_ID;
    lives = 3;
    score = 0;
}

void Player::movePlayer(char key, int64_t frameDelta){
    if(key == 'A' && getPosition().x > 0){
        move(-1 * frameDelta, 0);
    }
    else if(key == 'D' && getPosition().x + getSize().x < SCENE_WIDTH){
        move(frameDelta, 0);
    }
}
#include "Player.h"
#include <iostream>
#include "../v8helpers.h"
#include "../ScriptManager.h"


Player::Player(int id, sf::Vector2f size, sf::Vector2f position, sf::Vector2f startingPosition, std::string texturePath) : CollidableObject(id, size, position, startingPosition, texturePath){
    setPosition(position);
    objId = PLAYER_ID;
    jumpHeight = 200;
}

void Player::movePlayer(char key, int64_t frameDelta, bool sprint){
    float xDir;

    float velocity = 1;

    int sprinting = sprint ? 2 : 1;

    if(key == 'A'){
        xDir = -1 * velocity * frameDelta * sprinting;
    }
    else if(key == 'D'){
        xDir = 1 * velocity * frameDelta * sprinting;
    }

    move(xDir, 0);

}

bool Player::gravity(int64_t frameDelta){
    float velocity = 0.5;

    if(jumping){
        move(0, -1 * velocity * frameDelta);
        
        if(getPosition().y <= jumpPeak){
            jumping = false;
            jumpPeak = 0;
        }
        return true;
    }
    else if(!isCollidingUnder){
        move(0, velocity * frameDelta);
        return true;
    }

    return false;

}

bool Player::isJumping(){
    return jumping;
}

void Player::setJumping(){
    if(!jumping && colliding){
        jumping = true;
        jumpPeak = getPosition().y - jumpHeight;
    }
}

void Player::setColliding(bool isColliding){
    colliding = isColliding;
    if(jumping && colliding){
        //end the jump if it hits something
        jumping = false;
        jumpPeak = 0;
    }
}

void Player::setIsCollidingUnder(bool iCU){
    isCollidingUnder = iCU;
}

bool Player::checkCollision(CollidableObject* other){
    return getGlobalBounds().intersects(other->getGlobalBounds());
}

v8::Local<v8::Object> Player::exposeToV8(v8::Isolate *isolate, v8::Local<v8::Context> &context, std::string context_name){
	std::vector<v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>> v;
    v.push_back(v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>("colorR", getColor, setColor));
    v.push_back(v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>("outlineColorR", getOutlineColorR, setOutlineColorR));
    v.push_back(v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>("xPos", getXPos, setXPos));
    return v8helpers::exposeToV8("thisPlayer", this, v, isolate, context, context_name);
}


void Player::setColor(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
	void* ptr = wrap->Value();
	static_cast<Player*>(ptr)->setFillColor(sf::Color(value->Int32Value(), 0, 0));
}

void Player::getColor(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int colorR = static_cast<Player*>(ptr)->getFillColor().r;
    info.GetReturnValue().Set(colorR);
}

void Player::setOutlineColorR(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
	void* ptr = wrap->Value();
	static_cast<Player*>(ptr)->setOutlineColor(sf::Color(value->Int32Value(), 255, 255));
}

void Player::getOutlineColorR(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int colorR = static_cast<Player*>(ptr)->getOutlineColor().r;
    info.GetReturnValue().Set(colorR);
}

void Player::setXPos(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
	void* ptr = wrap->Value();
	static_cast<Player*>(ptr)->setPosition(sf::Vector2f(value->Int32Value(), static_cast<Player*>(ptr)->getPosition().x));
}

void Player::getXPos(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int xPos = static_cast<Player*>(ptr)->getPosition().x;
    info.GetReturnValue().Set(xPos);
}
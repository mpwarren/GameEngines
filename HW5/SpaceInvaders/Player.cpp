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

v8::Local<v8::Object> Player::exposeToV8(v8::Isolate *isolate, v8::Local<v8::Context> &context, std::string context_name){
	std::vector<v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>> v;
    v.push_back(v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>("outlineColorR", getOutlineColorR, setOutlineColorR));
    v.push_back(v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>("colorR", getColor, setColor));
    v.push_back(v8helpers::ParamContainer<v8::AccessorGetterCallback, v8::AccessorSetterCallback>("lives", getLives, setLives));
    return v8helpers::exposeToV8("thisPlayer", this, v, isolate, context, context_name);
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

void Player::setLives(v8::Local<v8::String> property, v8::Local<v8::Value> value, const v8::PropertyCallbackInfo<void>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
	void* ptr = wrap->Value();
	static_cast<Player*>(ptr)->lives = value->Int32Value();
}

void Player::getLives(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info){
    v8::Local<v8::Object> self = info.Holder();
    v8::Local<v8::External> wrap = v8::Local<v8::External>::Cast(self->GetInternalField(0));
    void* ptr = wrap->Value();
    int lives = static_cast<Player*>(ptr)->lives;
    info.GetReturnValue().Set(lives);
}
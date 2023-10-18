#include <zmq.hpp>
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include "Timeline.h"
#include <iostream>
#include <thread>
#include <cmath>

/*
void movePlatforms(std::vector<MovingPlatform*> movingObjects){
    //Initalize socket
    zmq::context_t context (2);

    zmq::socket_t platformMovementSocket (context, zmq::socket_type::pub);

    //int conflate = 1;
    //platformMovementSocket.setsockopt(ZMQ_CONFLATE, &conflate, sizeof(conflate));

    platformMovementSocket.bind ("tcp://*:5557");

    //Create Timelines
    Timeline anchorTimeline;
    Timeline platformTime(&anchorTimeline);
    int64_t lastTime = platformTime.getTime();


    while (true){

        int64_t currentTime = platformTime.getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;

        for(MovingPlatform* obj : movingObjects){
            obj->movePosition(frameDelta);

            std::string platformPositionStr = "" + std::to_string(obj->id) + " " + std::to_string(obj->getPosition().x) + " " + std::to_string(obj->getPosition().y) + "\0";

            zmq::message_t posMessage(sizeof(platformPositionStr));
            memcpy(posMessage.data(), platformPositionStr.c_str(), sizeof(platformPositionStr));
            platformMovementSocket.send(posMessage, zmq::send_flags::none);
            
        }
    }
}
*/

int main(){

    //Create Timelines
    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);

    int id = 1;

    std::map<int, CollidableObject*> gameObjects;

    Platform platform(id, sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "");
    gameObjects[platform.id] = &platform;
    id++;

    MovingPlatform horzPlatform(id, sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.0003, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));
    gameObjects[horzPlatform.id] = &horzPlatform;
    id++;

    MovingPlatform vertPlatform(id, sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.0003, 400);
    vertPlatform.setFillColor(sf::Color(150, 50, 250));
    gameObjects[vertPlatform.id] = &vertPlatform;
    id++;

    //Initalize sockets
    zmq::context_t context (1);
    zmq::socket_t playerConnectionSocket (context, zmq::socket_type::rep);
    playerConnectionSocket.bind ("tcp://*:5555");

    while(true){
        zmq::message_t playerMessage;
        playerConnectionSocket.recv(playerMessage, zmq::recv_flags::none);

        if(playerMessage.to_string() == "np"){
            zmq::message_t newResponse(std::to_string(id).length());
            memcpy(newResponse.data(), std::to_string(id).c_str(), std::to_string(id).length());
            id++;
            playerConnectionSocket.send(newResponse, zmq::send_flags::sndmore);

            for(int i = 1; i < id - 1; i++){
                zmq::message_t platformMessage(gameObjects[i]->toString().length());
                std::cout << "SENDING: " << gameObjects[i]->toString() << "of size" << std::to_string((gameObjects[i]->toString().length())) << std::endl;
                memcpy(platformMessage.data(), gameObjects[i]->toString().c_str(), gameObjects[i]->toString().length());
                if(i == id - 2){
                    playerConnectionSocket.send(platformMessage, zmq::send_flags::none);
                }
                else{
                    playerConnectionSocket.send(platformMessage, zmq::send_flags::sndmore);
                }
            }

        }
    }
    
}
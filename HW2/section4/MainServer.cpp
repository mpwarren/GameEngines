#include <zmq.hpp>
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include "Timeline.h"
#include <iostream>
#include <thread>
#include <cmath>

void movePlatforms(std::vector<MovingPlatform*> movingObjects){
    //Initalize socket
    zmq::context_t context (1);

    zmq::socket_t platformMovementSocket (context, zmq::socket_type::pub);
    platformMovementSocket.bind ("tcp://*:5557");

    while (true){
        for(MovingPlatform* obj : movingObjects){
            obj->movePosition();

            std::string platformPositionStr = "" + std::to_string(obj->id) + " " + std::to_string(obj->getPosition().x) + " " + std::to_string(obj->getPosition().y);

            zmq::message_t posMessage(sizeof(platformPositionStr));
            memcpy(posMessage.data(), platformPositionStr.c_str(), sizeof(platformPositionStr));
            platformMovementSocket.send(posMessage, zmq::send_flags::none);
            
        }
    }
    //move platforms

    //send updates to publish socket
}


int main(){

    //Initalize socket
    zmq::context_t context (1);
    zmq::socket_t pubSocket (context, zmq::socket_type::pub);
    pubSocket.bind ("tcp://*:5555");

    //Initalize socket
    zmq::socket_t repSocket (context, zmq::socket_type::rep);
    repSocket.bind ("tcp://*:5556");



    //Create Timelines
    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);

    //Create platforms and player
    std::vector<MovingPlatform*> movingObjects;
    std::vector<CollidableObject*> collidableObjects;

    std::map<int, CollidableObject*> globalPositions;

    Platform platform(1, sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "Textures/brightgrass.png");
    collidableObjects.push_back(&platform);
    globalPositions[1] = &platform;

    MovingPlatform horzPlatform(2, sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.0003, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(&horzPlatform);
    collidableObjects.push_back(&horzPlatform);
    globalPositions[2] = &horzPlatform;

    MovingPlatform vertPlatform(3, sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.0003, 400);
    vertPlatform.setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(&vertPlatform);
    collidableObjects.push_back(&vertPlatform);
    globalPositions[3] = &vertPlatform;

    int lastPlatformId = 3;
    int playersIn = 0;
    Player player1(4, sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    Player player2(5, sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    Player player3(6, sf::Vector2f(50,50), sf::Vector2f(50, 50), "");

    std::thread platformThread(movePlatforms, movingObjects);

    while(true){


        zmq::message_t request(3);
        repSocket.recv (request, zmq::recv_flags::none);
        std::string requestStr = request.to_string();
        std::cout<< requestStr << std::endl;
        if(requestStr == "np"){
            std::cout << "NEW PLAYER" << std::endl;

            if(playersIn == 0){
                globalPositions[player1.id] = &player1;
            }
            else if(playersIn == 1){
                globalPositions[player2.id] = &player2;
            }
            else if(playersIn == 2){
                globalPositions[player3.id] = &player3;
            }

            playersIn++;

            //send joining response
            zmq::message_t playerIdMessage(1);
            memcpy(playerIdMessage.data(), std::to_string(playersIn + lastPlatformId).c_str(), 1);
            repSocket.send(playerIdMessage, zmq::send_flags::none);
        }
        else{
            int moreToRead = 1;
            while(moreToRead != 0){
                zmq::message_t cordMessage;
                repSocket.recv(cordMessage, zmq::recv_flags::none);
                float cord;
                memcpy(&cord, cordMessage.data(), sizeof(float));
                std::cout << "CORD: " << cord <<std::endl;
                
                size_t moreSize = sizeof(moreToRead);
                repSocket.getsockopt(ZMQ_RCVMORE, &moreToRead, &moreSize);
            }
            //recieving movement

            //publish position
            // std::string positionString = std::to_string(id) + " " + std::to_string((int)((Player*)globalPositions[id])->getPosition().x) + " " + std::to_string((int)((Player*)globalPositions[id])->getPosition().y);
            // std::cout << "RESPONSE MESSAGE: " << positionString << std::endl;
            // zmq::message_t positionUpdate(positionString.size());
            // memcpy(positionUpdate.data(), positionString.c_str(), positionString.size());
            // std::cout<< "Sending Message: " << positionUpdate.to_string() << std::endl;
            // pubSocket.send(positionUpdate, zmq::send_flags::none);

            zmq::message_t message(0);
            repSocket.send(message, zmq::send_flags::none);
        }
        //move entity

        //check for collisions

        //send update of moved entities
    }


    
}
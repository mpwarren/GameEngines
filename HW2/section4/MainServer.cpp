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
}

/*
void publishPlayerLocations(){
    zmq::context_t context (2);
    zmq::socket_t playerHasMovedSocket (context, zmq::socket_type::rep);
    playerHasMovedSocket.bind ("tcp://*:5558");

    zmq::socket_t publishPlayerDataSocket (context, zmq::socket_type::pub);
    publishPlayerDataSocket.bind ("tcp://*:5559");
    while(true){


        zmq::message_t playerPositionMessage(64);
        playerHasMovedSocket.recv(playerPositionMessage, zmq::recv_flags::none);
        zmq::message_t rep(0);
        playerHasMovedSocket.send(rep, zmq::send_flags::none);

        std::cout<< "PLAYER JUST MOVED: " << playerPositionMessage.to_string() << std::endl;

        publishPlayerDataSocket.send(playerPositionMessage, zmq::send_flags::none);
    }
}
*/

int main(){

    //Initalize socket
    zmq::context_t context (4);
    zmq::socket_t pubSocket (context, zmq::socket_type::rep);
    pubSocket.bind ("tcp://*:5555");

    //Initalize socket
    zmq::socket_t repSocket (context, zmq::socket_type::rep);
    repSocket.bind ("tcp://*:5556");

    zmq::socket_t playerHasMovedSocket (context, zmq::socket_type::rep);
    playerHasMovedSocket.bind ("tcp://*:5558");

    zmq::socket_t publishPlayerDataSocket (context, zmq::socket_type::pub);
    publishPlayerDataSocket.bind ("tcp://*:5559");



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
    //std::thread playerThread(publishPlayerLocations);

    zmq::pollitem_t items [] = {
        { repSocket, 0, ZMQ_POLLIN, 0 },
        { playerHasMovedSocket, 0, ZMQ_POLLIN, 0 }
    };

    std::map<int, std::string> playerPositions;
    playerPositions[4] = "4 50 50";
    playerPositions[5] = "4 50 50";
    playerPositions[6] = "4 50 50";


    while(true){
        zmq::message_t request(3);

        zmq::poll(&items [0], 2, -1);

        if(items[0].revents & ZMQ_POLLIN){
            repSocket.recv (request, zmq::recv_flags::none);

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
            memcpy(playerIdMessage.data(), std::to_string(playersIn).c_str(), 1);
            repSocket.send(playerIdMessage, zmq::send_flags::none);


        }
        if(items[1].revents & ZMQ_POLLIN){
            zmq::message_t playerPositionMessage(64);
            playerHasMovedSocket.recv(playerPositionMessage, zmq::recv_flags::none);

            zmq::message_t positionReply(64);
            if(playerPositionMessage.to_string().length() == 1){

                if(playerPositionMessage.to_string() == "4"){
                    memcpy(positionReply.data(), playerPositions[4].c_str(), playerPositions[4].length());
                    playerHasMovedSocket.send(positionReply, zmq::send_flags::none);
                }
                else if(playerPositionMessage.to_string() == "5"){
                    memcpy(positionReply.data(), playerPositions[5].c_str(), playerPositions[4].length());
                    playerHasMovedSocket.send(positionReply, zmq::send_flags::none);
                }
                else if(playerPositionMessage.to_string() == "6"){
                    memcpy(positionReply.data(), playerPositions[6].c_str(), playerPositions[4].length());
                    playerHasMovedSocket.send(positionReply, zmq::send_flags::none);
                }
            }
            else{
                zmq::message_t rep(0);
                playerHasMovedSocket.send(rep, zmq::send_flags::none);

                std::cout<< "PLAYER JUST MOVED: " << playerPositionMessage.to_string() << std::endl;

                
                if(playerPositionMessage.to_string().c_str()[0] == '4'){
                    playerPositions[4] = playerPositionMessage.to_string();
                }
                else if(playerPositionMessage.to_string().c_str()[0] == '5'){
                    playerPositions[5] = playerPositionMessage.to_string();
                }
                else if(playerPositionMessage.to_string().c_str()[0] == '6'){
                    playerPositions[6] = playerPositionMessage.to_string();

                }
                
                publishPlayerDataSocket.send(playerPositionMessage, zmq::send_flags::none);
            }

        }

    }


    
}
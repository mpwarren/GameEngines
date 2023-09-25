#include <zmq.hpp>
#include "Platform.h"
#include "MovingPlatform.h"
#include "Player.h"
#include "Timeline.h"
#include <iostream>

void publishData(){

}

void platformsThread(){
    //move platforms

    //send updates to publish socket
}

void playerMovement(){
    //wait for player movement

    //send updates to publish socket
}


int main(){

    //Initalize socket
    zmq::context_t context (1);
    zmq::socket_t pubSocket (context, zmq::socket_type::pub);
    pubSocket.bind ("tcp://*:5555");

    //Initalize socket
    zmq::socket_t repSocket (context, zmq::socket_type::rep);
    pubSocket.bind ("tcp://*:5556");

    //Create Timelines
    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);

    //Create platforms and player
    std::vector<MovingPlatform*> movingObjects;
    std::vector<CollidableObject*> collidableObjects;

    std::map<int, CollidableObject*> globalPositions;

    Platform platform(sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "../Textures/brightgrass.png");
    collidableObjects.push_back(&platform);
    globalPositions[1] = &platform;

    MovingPlatform horzPlatform(sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.1, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(&horzPlatform);
    collidableObjects.push_back(&horzPlatform);
    globalPositions[2] = &horzPlatform;

    MovingPlatform vertPlatform(sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.1, 400);
    vertPlatform.setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(&vertPlatform);
    collidableObjects.push_back(&vertPlatform);
    globalPositions[3] = &vertPlatform;


    Player player(sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    player.setFillColor(sf::Color(150, 50, 250));

    while(true){

        //wait for game update
        //"np": new player
        //"ixy": 
        //      i: object id 
        //      x: x cord
        //      y: y cord
        zmq::message_t request(3);
        repSocket.recv (request, zmq::recv_flags::none);
        if(request.to_string() == "np"){
            std::cout << "NEW PLAYER" << std::endl;
        }
        else{
            int req = stoi(request.to_string());
            int id = req % 10;
            req /= 10;
            int x = req % 10;
            req /= 10;
            int y = req;
            std::cout << "ID: " << id << " X: " << x << " Y: " << y << std::endl;
        }
        //move entity

        //check for collisions

        //send update of moved entities
    }


    
}
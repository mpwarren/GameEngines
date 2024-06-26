#include <zmq.hpp>
#include <string>
#include <iostream>
#include <thread>
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include <SFML/Graphics.hpp>
#include <mutex>


void platformMovement(std::mutex * renderMutex, std::map<int, MovingPlatform*> platformMap){

    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t subSocket (context, zmq::socket_type::sub);
    subSocket.connect ("tcp://localhost:5557");
    subSocket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true){
        zmq::message_t positionUpdate(64);
        subSocket.recv(positionUpdate, zmq::recv_flags::none);
        std::string updateString = positionUpdate.to_string();
        std::istringstream ss(updateString);
        std::istream_iterator<std::string> begin(ss), end;
        std::vector<std::string> words(begin, end);
        
        int currentId = stoi(words[0]);
        float xCord = stof(words[1]);
        float yCord = stof(words[2]);
        {
            std::lock_guard<std::mutex> lock(*renderMutex);
            platformMap[currentId]->setPosition(sf::Vector2f(xCord, yCord));
        }
    }
}

void moveOtherPlayers(std::vector<Player*> players, int myId, int * currentPlayers, std::mutex * otherPlayerMutex){
    zmq::context_t context (1);
    zmq::socket_t subSocket (context, zmq::socket_type::sub);
    subSocket.connect ("tcp://localhost:5559");
    subSocket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true){
        zmq::message_t positionUpdate(64);
        subSocket.recv(positionUpdate, zmq::recv_flags::none);
        std::string updateString = positionUpdate.to_string();
        std::istringstream ss(updateString);
        std::istream_iterator<std::string> begin(ss), end;
        std::vector<std::string> words(begin, end);
        
        {
            std::lock_guard<std::mutex> lock(*otherPlayerMutex);
            int currentId = stoi(words[0]);
            if(currentId != myId){
                if(currentId > 3 + (*currentPlayers)){
                    (*currentPlayers)++;
                }
                float xCord = stof(words[1]);
                float yCord = stof(words[2]);

                players[currentId - 4]->setPosition(sf::Vector2f(xCord, yCord));

            }

        }

    }
}

int main ()
{

    //Create platforms and player
    std::vector<CollidableObject*> collidableObjects;

    std::map<int, MovingPlatform*> movingPlatformsMap;

    sf::RenderWindow window(sf::VideoMode(800, 600), "My Window", sf::Style::Default);
    Platform platform(1, sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "Textures/brightgrass.png");
    collidableObjects.push_back(&platform);

    MovingPlatform horzPlatform(2, sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.01, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));
    movingPlatformsMap[horzPlatform.id] = &(horzPlatform);
    collidableObjects.push_back(&horzPlatform);

    MovingPlatform vertPlatform(3, sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.01, 400);
    vertPlatform.setFillColor(sf::Color(150, 50, 250));
    movingPlatformsMap[vertPlatform.id] = &(vertPlatform);
    collidableObjects.push_back(&vertPlatform);

    std::mutex renderMutex;

    //  Prepare our context and socket
    zmq::context_t context (3);

    zmq::socket_t reqSocket (context, zmq::socket_type::req);
    reqSocket.connect ("tcp://localhost:5556");

    zmq::socket_t sendPlayerDataSocket(context, zmq::socket_type::req);
    sendPlayerDataSocket.connect("tcp://localhost:5558");

    //Let server know a new player connected
    zmq::message_t newPlayerMessage (2);
    memcpy (newPlayerMessage.data (), "np", 2);
    std::cout << "Connecting To Server..."<< std::endl;
    reqSocket.send (newPlayerMessage, zmq::send_flags::none);
    
    zmq::message_t connectionResponse (1);
    reqSocket.recv(connectionResponse, zmq::recv_flags::none);
    int playersInGame = std::stoi(connectionResponse.to_string());
    int myId = 3 + playersInGame;
    std::cout << "Connected with id: "<< myId << std::endl;
    std::vector<Player*> currentPlayers;
    Player player(myId, sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    player.setFillColor(sf::Color(150, 50, 250));

    Player player2(0, sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    player2.setFillColor(sf::Color(150, 50, 250));

    Player player3(0, sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    player3.setFillColor(sf::Color(150, 50, 250));

    std::string playerPosString = "" + std::to_string(player.id) + " " + std::to_string(player.getPosition().x) + " " + std::to_string(player.getPosition().y);
    zmq::message_t posMessage(sizeof(playerPosString));
    memcpy(posMessage.data(), playerPosString.c_str(), sizeof(playerPosString));
    sendPlayerDataSocket.send(posMessage, zmq::send_flags::none);
    zmq::message_t rep(0);
    sendPlayerDataSocket.recv(rep, zmq::recv_flags::none);

    if(playersInGame == 1){
        player2.id = myId + 1;
        player3.id = myId + 2;
        currentPlayers.push_back(&player);
        currentPlayers.push_back(&player2);
        currentPlayers.push_back(&player3);
    }
    else if(playersInGame == 2){
        player2.id = myId - 1;
        player3.id = myId + 1;
        currentPlayers.push_back(&player2);
        currentPlayers.push_back(&player);
        currentPlayers.push_back(&player3);


    }
    else if(playersInGame == 3){
        player2.id = myId - 1;
        player3.id = myId - 2;
        currentPlayers.push_back(&player3);
        currentPlayers.push_back(&player2);
        currentPlayers.push_back(&player);
    }

    if(playersInGame >= 2){
        zmq::message_t playerIdMessage(1);
        memcpy(playerIdMessage.data(), std::to_string(player2.id).c_str(), 1);
        sendPlayerDataSocket.send(playerIdMessage, zmq::send_flags::none);

        //get player 2's position
        zmq::message_t player2Position(64);
        sendPlayerDataSocket.recv(player2Position, zmq::recv_flags::none);
        std::string updateString = player2Position.to_string();
        std::istringstream ss(updateString);
        std::istream_iterator<std::string> begin(ss), end;
        std::vector<std::string> words(begin, end);

        float xCord = stof(words[1]);
        float yCord = stof(words[2]);

        player2.setPosition(sf::Vector2f(xCord, yCord));

    }
    if(playersInGame == 3){
        zmq::message_t playerIdMessage(1);
        memcpy(playerIdMessage.data(), std::to_string(player3.id).c_str(), 1);
        sendPlayerDataSocket.send(playerIdMessage, zmq::send_flags::none);

        //get player 3's position
        zmq::message_t player3Position(64);
        sendPlayerDataSocket.recv(player3Position, zmq::recv_flags::none);
        std::string updateString = player3Position.to_string();
        std::istringstream ss(updateString);
        std::istream_iterator<std::string> begin(ss), end;
        std::vector<std::string> words(begin, end);

        float xCord = stof(words[1]);
        float yCord = stof(words[2]);

        player3.setPosition(sf::Vector2f(xCord, yCord));
    }



    std::thread platformThread(platformMovement, &renderMutex, movingPlatformsMap);
    
    std::mutex otherPlayerMutex;
    std::thread otherPlayerThread(moveOtherPlayers, currentPlayers, myId, &playersInGame, &otherPlayerMutex);

    //Thread to detect and send player movement
    while(window.isOpen()){

        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();

        }

        window.clear(sf::Color::Black);

        bool moved = false;
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                player.movePlayer(sf::Keyboard::W);
                moved = true;
            }
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                player.movePlayer(sf::Keyboard::A);
                moved = true;
            }
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                player.movePlayer(sf::Keyboard::S);
                moved = true;
            }
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            {
                std::lock_guard<std::mutex> lock(renderMutex);
                player.movePlayer(sf::Keyboard::D);
                moved = true;
            }
        }

        //check collisions
        for(CollidableObject* obj : collidableObjects){

            if(player.getGlobalBounds().intersects(obj->getGlobalBounds())){

                //lock mutex until method ends, stopping all other movement until collision is resolved
                {
                    std::lock_guard<std::mutex> lock(renderMutex);

                    player.resolveColision(obj);
                    moved = true;
                }


            }
        }

        if(moved){
            //update server on player's position
            std::string playerPosString = "" + std::to_string(player.id) + " " + std::to_string(player.getPosition().x) + " " + std::to_string(player.getPosition().y);
            zmq::message_t posMessage(sizeof(playerPosString));
            memcpy(posMessage.data(), playerPosString.c_str(), sizeof(playerPosString));
            sendPlayerDataSocket.send(posMessage, zmq::send_flags::none);
            zmq::message_t rep(0);
            sendPlayerDataSocket.recv(rep, zmq::recv_flags::none);
        }


        //DRAWING
        window.draw(player);
        {
            std::lock_guard<std::mutex> lock(otherPlayerMutex);
            if(playersInGame > 1){
                window.draw(player2);
            }
            if(playersInGame > 2 ){
                window.draw(player3);
            }

        }
        window.draw(platform);
        //draw platforms

        for(auto &kv : movingPlatformsMap){
            window.draw(*kv.second);
        }

        window.display();

    }



    return 0;
}
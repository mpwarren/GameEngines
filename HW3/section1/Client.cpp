#include <map>
#include "GameShapes/CollidableObject.h"
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include <zmq.hpp>
#include <mutex>


std::mutex platformMutex;

// void platformMovement(std::vector<MovingPlatform*> movingPlatforms){

// }


int main(){

    sf::Vector2f spawnPoint(50,50);

    std::map<int, CollidableObject*> gameObjects;

    //Connect to server
    zmq::context_t context (3);
    zmq::socket_t newPlayerSocket (context, zmq::socket_type::req);
    newPlayerSocket.connect ("tcp://localhost:5555");

    zmq::message_t newConnection(2);
    memcpy(newConnection.data(), "np", 2);
    newPlayerSocket.send(newConnection, zmq::send_flags::none);

    //Get Id and create player object

    zmq::message_t idMessage(2);
    newPlayerSocket.recv(idMessage, zmq::recv_flags::none);
    std::cout << "ID: " << idMessage.to_string() << std::endl;
    Player thisPlayer(stoi(idMessage.to_string()), sf::Vector2f(50,50), spawnPoint, "");
    gameObjects[thisPlayer.id] = &thisPlayer;

    int moreToRead = 1;
    while(moreToRead != 0){
        zmq::message_t objectMessage;
        newPlayerSocket.recv(objectMessage, zmq::recv_flags::none);
        std::string objectString = objectMessage.to_string();
        std::istringstream ss(objectString);
        std::istream_iterator<std::string> begin(ss), end;
        std::vector<std::string> params(begin, end);
        
        std::cout << "Message: " << objectMessage.to_string() << std::endl;
        if(params[0] == "PT"){
            int id = stoi(params[1]);
            Platform* pt = new Platform(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), params[6]);
            gameObjects[id] = pt;
        }
        else if(params[0] == "MP"){
            int id = stoi(params[1]);
            MovingPlatform* pt = new MovingPlatform(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), params[6], (Direction)stoi(params[7]), stof(params[8]), stoi(params[9]));
            gameObjects[id] = pt;
        }

        size_t moreSize = sizeof(moreToRead);
        newPlayerSocket.getsockopt(ZMQ_RCVMORE, &moreToRead, &moreSize);
    }

    for(int i = 1; i <= 3; i++){
        std::cout << gameObjects[i]->toString() << std::endl;
    }

    sf::RenderWindow window(sf::VideoMode(800, 600), "My Window", sf::Style::Default);

    while(window.isOpen()){
        
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window.close();


            window.clear(sf::Color::Black);

            for(auto const& obj : gameObjects){
                window.draw(*obj.second);
            }
            
            window.display();


        }
    }

    //
}
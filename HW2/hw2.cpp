#include <SFML/Graphics.hpp>
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include <iostream>
#include <mutex>
#include <thread>

void moveObjects(std::mutex *renderMutex, sf::RenderWindow *window, Player *player, std::vector<CollidableObject*> collidableObjects, std::vector<MovingPlatform*> movingObjects){
    window->setActive(true);
    
    while(window->isOpen()){

        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window->pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window->close();
        }

        window->clear(sf::Color::Black);

        for(MovingPlatform* obj : movingObjects){
            {
                std::unique_lock<std::mutex> lock(*renderMutex);
                obj->movePosition();
            }
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            {
                std::unique_lock<std::mutex> lock(*renderMutex);
                player->movePlayer(sf::Keyboard::W);
            }
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            {
                std::unique_lock<std::mutex> lock(*renderMutex);
                player->movePlayer(sf::Keyboard::A);
            }
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
            {
                std::unique_lock<std::mutex> lock(*renderMutex);
                player->movePlayer(sf::Keyboard::S);
            }
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            {
                std::unique_lock<std::mutex> lock(*renderMutex);
                player->movePlayer(sf::Keyboard::D);
            }
        }

        {
            //make sure nothing is moving before drawing
            std::unique_lock<std::mutex> lock(*renderMutex);
            window->draw(*player);
            for(CollidableObject* obj : collidableObjects){
                window->draw(*obj);
            }
            
            // end the current frame
            window->display();
        }

    }
}

void checkCollisions(std::mutex *renderMutex, sf::RenderWindow *window, Player *player, std::vector<CollidableObject*> collidableObjects){
    while(window->isOpen()){

        for(CollidableObject* obj : collidableObjects){

            if(player->getGlobalBounds().intersects(obj->getGlobalBounds())){

                //lock mutex until method ends, stopping all other movement until collision is resolved
                {
                    std::lock_guard<std::mutex> lock(*renderMutex);

                    player->resolveColision(obj);
                }


            }
        }
    }
}


int main()
{

    std::mutex renderMutex;

    sf::RenderWindow window(sf::VideoMode(800, 600), "My Window", sf::Style::Default);
    window.setActive(false);

    std::vector<MovingPlatform*> movingObjects;
    std::vector<CollidableObject*> collidableObjects;


    //Create platforms and player
    Platform platform(sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "Textures/brightgrass.png");
    collidableObjects.push_back(&platform);

    MovingPlatform horzPlatform(sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.5, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(&horzPlatform);
    collidableObjects.push_back(&horzPlatform);

    MovingPlatform vertPlatform(sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.5, 400);
    vertPlatform.setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(&vertPlatform);
    collidableObjects.push_back(&vertPlatform);

    Player player(sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    player.setFillColor(sf::Color(150, 50, 250));
    
    std::thread movementThread(moveObjects, &renderMutex, &window, &player, collidableObjects, movingObjects);
    std::thread collisionThread(checkCollisions, &renderMutex, &window, &player, collidableObjects);

    movementThread.join();
    collisionThread.join();

    return 0;
}
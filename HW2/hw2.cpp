#include <SFML/Graphics.hpp>
#include "GameShapes/Platform.h"
#include "GameShapes/MovingPlatform.h"
#include "GameShapes/Player.h"
#include "Timeline.h"
#include <iostream>
#include <mutex>
#include <thread>

void moveObjects(std::mutex *renderMutex, sf::RenderWindow *window, Timeline *gameTimeline, Player *player, std::vector<CollidableObject*> collidableObjects, std::vector<MovingPlatform*> movingObjects){
    window->setActive(true);
    
    int64_t lastTime = gameTimeline->getTime();
    
    bool changedTic = false;

    while(window->isOpen()){

        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window->pollEvent(event))
        {
            // "close requested" event: we close the window
            if (event.type == sf::Event::Closed)
                window->close();

            if(event.type == sf::Event::KeyReleased){
                if(event.key.code == sf::Keyboard::P){
                    if(gameTimeline->isPaused()){
                        int64_t elapsedTime = gameTimeline->unpause();
                        lastTime = gameTimeline->getTime();
                    }
                    else{
                        gameTimeline->pause(lastTime);
                    }
                }
            }
        }

        window->clear(sf::Color::Black);

        int64_t currentTime = gameTimeline->getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::F)){
            gameTimeline->changeTic(4);
            lastTime = gameTimeline->getTime();
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::G)){
            gameTimeline->changeTic(2);
            lastTime = gameTimeline->getTime();
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::H)){
            gameTimeline->changeTic(1);
            lastTime = gameTimeline->getTime();
        }




        for(MovingPlatform* obj : movingObjects){
            {
                std::lock_guard<std::mutex> lock(*renderMutex);
                obj->movePosition(frameDelta);
            }
        }

        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            {
                std::lock_guard<std::mutex> lock(*renderMutex);
                player->movePlayer(sf::Keyboard::W, frameDelta);
            }
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            {
                std::lock_guard<std::mutex> lock(*renderMutex);
                player->movePlayer(sf::Keyboard::A, frameDelta);
            }
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
            {
                std::lock_guard<std::mutex> lock(*renderMutex);
                player->movePlayer(sf::Keyboard::S, frameDelta);
            }
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            {
                std::lock_guard<std::mutex> lock(*renderMutex);
                player->movePlayer(sf::Keyboard::D, frameDelta);
            }
        }

        {
            //make sure nothing is moving before drawing
            std::lock_guard<std::mutex> lock(*renderMutex);
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

    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);

    //Create platforms and player
    Platform platform(sf::Vector2f(780.f, 15.f), sf::Vector2f(10,575), "Textures/brightgrass.png");
    collidableObjects.push_back(&platform);

    MovingPlatform horzPlatform(sf::Vector2f(60.f, 15.f), sf::Vector2f(400, 300), "", Direction::horizontal, 0.1, 200);
    horzPlatform.setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(&horzPlatform);
    collidableObjects.push_back(&horzPlatform);

    MovingPlatform vertPlatform(sf::Vector2f(100.f, 15.f), sf::Vector2f(200, 100), "", Direction::vertical, 0.1, 400);
    vertPlatform.setFillColor(sf::Color(150, 50, 250));
    movingObjects.push_back(&vertPlatform);
    collidableObjects.push_back(&vertPlatform);

    Player player(sf::Vector2f(50,50), sf::Vector2f(50, 50), "");
    player.setFillColor(sf::Color(150, 50, 250));
    
    std::thread movementThread(moveObjects, &renderMutex, &window, &gameTime, &player, collidableObjects, movingObjects);
    std::thread collisionThread(checkCollisions, &renderMutex, &window, &player, collidableObjects);

    movementThread.join();
    collisionThread.join();

    while(window.isOpen()){
        
    }

    return 0;
}
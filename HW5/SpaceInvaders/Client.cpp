#include <map>
#include "CollidableObject.h"
#include "Player.h"
#include <zmq.hpp>
#include <mutex>
#include <thread>
#include "Timeline.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventManager.h"
#include <unistd.h>
#include "Bullet.h"
#include "EnemyGrid.h"
#include "EnemyBullet.h"

std::mutex *enemyMutex;

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

void recieveEnemyInstructions(EnemyGrid* en){
    //connect to server
    zmq::context_t context (1);
    zmq::socket_t enemyListner (context, zmq::socket_type::pull);
    enemyListner.connect ("tcp://localhost:5557");


}

int main(){

    //mutexes
    enemyMutex = new std::mutex();

    //game objects and data structures
    Player * player;
    std::vector<std::shared_ptr<Bullet>> bullets;
    std::vector<std::shared_ptr<EnemyBullet>> enemyBullets;

    sf::Text livesText;
    livesText.setString("Lives:");
    sf::Font font;
    font.loadFromFile("Roboto-Black.ttf");
    livesText.setFont(font);
    livesText.setCharacterSize(20);
    livesText.setPosition(10, 560);

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(20);
    scoreText.setPosition(650, 10);

    sf::RectangleShape lifeMarker(sf::Vector2f(20, 20));
    lifeMarker.setFillColor(sf::Color(255, 0, 0));
    EnemyGrid *enemies;

    //connect to server
    zmq::context_t context (1);
    zmq::socket_t newPlayerSocket (context, zmq::socket_type::req);
    newPlayerSocket.connect ("tcp://localhost:5556");

    zmq::message_t newConnection(0);
    newPlayerSocket.send(newConnection, zmq::send_flags::none);

    //get id
    zmq::message_t idMessage(2);
    zmq::recv_result_t r =newPlayerSocket.recv(idMessage, zmq::recv_flags::none);
    std::cout << idMessage.to_string() << std::endl;
    int thisId = stoi(idMessage.to_string());
    std::cout << "Connected with ID: " << idMessage.to_string() << std::endl;

    //get game objects from server
    int moreToRead = 1;
    while(moreToRead != 0){
        zmq::message_t objectMessage;
        zmq::recv_result_t r =newPlayerSocket.recv(objectMessage, zmq::recv_flags::none);
        std::vector<std::string> params = parseMessage(objectMessage.to_string());

        if(params[0] == PLAYER_ID){
            int id = stoi(params[1]);
            player = new Player(id, sf::Vector2f(stof(params[2]), stof(params[3])), sf::Vector2f(stof(params[4]), stof(params[5])), params[6]);
        }
        else if(params[0] == ENEMY_GRID_ID){
            enemies = new EnemyGrid(stoi(params[1]), stoi(params[2]), stoi(params[3]));
        }
        else{
            std::cout << "ERROR: UNKNOWN OBJECT TYPE RECIEVED FROM SERVER" << std::endl;
        }

        size_t moreSize = sizeof(moreToRead);
        newPlayerSocket.getsockopt(ZMQ_RCVMORE, &moreToRead, &moreSize);
    }

    //create event manager and handlers
    EventManager *eventManager = new EventManager();
    PlayerHandler * playerHandler = new PlayerHandler(player);
    eventManager->addHandler(std::vector<EventType>{MOVEMENT_EV, ENEMY_DEATH_EV}, playerHandler);

    EnemyHandler * enemyHandler = new EnemyHandler(enemies, enemyMutex);
    eventManager->addHandler(std::vector<EventType>{ENEMY_DEATH_EV}, enemyHandler);

    //create sfml window
    sf::RenderWindow window(sf::VideoMode(SCENE_WIDTH, SCENE_HEIGHT), "Space Invaders Clone", sf::Style::Default);

    //create timelines
    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);
    int64_t lastTime = gameTime.getTime();
    while(window.isOpen()){
    
        // check all the window's events that were triggered since the last iteration of the loop
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed){
                if(event.key.code == sf::Keyboard::Space){
                    std::shared_ptr<Bullet> b = std::make_shared<Bullet>(player->getPosition().x);
                    bullets.push_back(b);
                }
            }
        }
        window.clear();

        int64_t currentTime = gameTime.getTime();
        int64_t frameDelta = currentTime - lastTime;
        lastTime = currentTime;

        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            std::shared_ptr<MovementEvent> e = std::make_shared<MovementEvent>(currentTime, HIGH, 'A', frameDelta);
            eventManager->addToQueue(e);
        }
        if(window.hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            std::shared_ptr<MovementEvent> e = std::make_shared<MovementEvent>(currentTime, HIGH, 'D', frameDelta);
            eventManager->addToQueue(e);
        }

        //move bullets and check for collisions with enemies
        for(auto it = bullets.begin(); it != bullets.end();){
            if((*it)->getPosition().y < 0){
                it = bullets.erase(it);
            }
            else{
                bool removed = false;
                std::lock_guard<std::mutex> lock(*enemyMutex);
                for(std::vector<Enemy*> row : enemies->enemyGrid){
                    for(Enemy* e : row){
                        if(!e->dead && (*it)->getGlobalBounds().intersects(e->getGlobalBounds())){
                            int row = e->row;
                            int col = e->col;
                            std::shared_ptr<EnemyDeathEvent> e = std::make_shared<EnemyDeathEvent>(currentTime, HIGH, row, col);
                            eventManager->addToQueue(e);
                            it = bullets.erase(it);
                            removed = true;
                        }
                    }
                }
                if(!removed){
                    (*it)->moveBullet(frameDelta);
                    ++it;
                }
            }
        }

        

        //process events
        {
            std::lock_guard<std::mutex> lock(eventManager->mutex);
            while(!eventManager->eventQueueHigh.empty() && eventManager->eventQueueHigh.top()->timeStamp <= gameTime.getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueHigh.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueHigh.pop();
            }
            while(!eventManager->eventQueueMedium.empty() && eventManager->eventQueueMedium.top()->timeStamp <= gameTime.getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueMedium.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueMedium.pop();
            }
            while(!eventManager->eventQueueLow.empty() && eventManager->eventQueueLow.top()->timeStamp <= gameTime.getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueLow.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueLow.pop();
            }
        }

        window.draw(*player);
        for(std::shared_ptr<Bullet> b : bullets){
            window.draw(*b);
        }

        //draw enemies
        {
            std::lock_guard<std::mutex> lock(*enemyMutex);
            for(std::vector<Enemy*> row : enemies->enemyGrid){
                for(Enemy* e : row){
                    if(!e->dead){
                        window.draw(*e);
                    }
                }
            }
        }


        //draw lives 
        window.draw(livesText);
        lifeMarker.setPosition(sf::Vector2f(70, 570));
        for(int i = 0; i < player->lives; i++){
            window.draw(lifeMarker);
            lifeMarker.setPosition(lifeMarker.getPosition().x + 40, lifeMarker.getPosition().y);
        }

        scoreText.setString("Score: " + std::to_string(player->score));
        window.draw(scoreText);

        window.display();

    }

}
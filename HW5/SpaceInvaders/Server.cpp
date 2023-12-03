#include <zmq.hpp>
#include "Timeline.h"
#include "CollidableObject.h"
#include "Player.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventManager.h"
#include "EnemyGrid.h"
#include <thread>
#include <random>
#include <unistd.h> 

std::mutex *enemyMutex;

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

void enemyAI(EnemyGrid *enemies){
    zmq::context_t context (1);
    zmq::socket_t enemySender (context, zmq::socket_type::push);
    enemySender.bind("tcp://*:5557");
    
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> distr(0, enemies->width - 1);
    
    while(true){
        int colToShoot = distr(gen);
        int rowToShoot = enemies->height-1;
        while(rowToShoot != -1 && enemies->enemyGrid[rowToShoot][colToShoot]->dead){
            rowToShoot--;
        }

        //try again with a different column
        if(rowToShoot == -1){
            continue;
        }

        std::string shootString = "S " + std::to_string(rowToShoot) + " " + std::to_string(colToShoot);
        zmq::message_t shootMessage(shootString.length());
        memcpy(shootMessage.data(), shootString.c_str(), shootString.length());
        enemySender.send(shootMessage, zmq::send_flags::none);

        sleep(1);
    }

}


void eventListner(EventManager *em, Timeline* timeline){
    zmq::context_t context(1);
    zmq::socket_t serverEventListner(context, zmq::socket_type::pull);
    serverEventListner.bind("tcp://*:5558");

    while(true){
        zmq::message_t eventMessage;
        zmq::recv_result_t r = serverEventListner.recv(eventMessage, zmq::recv_flags::none);

        //add it to server event manager
        std::vector<std::string> params = parseMessage(eventMessage.to_string());
        EventType t = (EventType)stoi(params[0]);
        if(t == ENEMY_DEATH_EV){
            std::shared_ptr<EnemyDeathEvent> e = std::make_shared<EnemyDeathEvent>(timeline->getTime(), (Priority)stoi(params[2]), stoi(params[3]), stoi(params[4]));
            em->addToQueue(e);
        }
    }
}

void eventProcessor(EventManager *eventManager, Timeline* gameTime){
    while(true){
        {
            std::lock_guard<std::mutex> lock(eventManager->mutex);
            while(!eventManager->eventQueueHigh.empty() && eventManager->eventQueueHigh.top()->timeStamp <= gameTime->getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueHigh.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueHigh.pop();
            }
            while(!eventManager->eventQueueMedium.empty() && eventManager->eventQueueMedium.top()->timeStamp <= gameTime->getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueMedium.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueMedium.pop();
            }
            while(!eventManager->eventQueueLow.empty() && eventManager->eventQueueLow.top()->timeStamp <= gameTime->getTime()){
                std::shared_ptr<Event> ev = eventManager->eventQueueLow.top();
                for(EventHandler* h : eventManager->handlers[ev->eventType]){
                    h->onEvent(ev);
                }
                eventManager->eventQueueLow.pop();
            }
        }
    }
}


int main(){
    enemyMutex = new std::mutex();
    //set up server connection
    zmq::context_t context (1);
    zmq::socket_t playerConnectionSocket (context, zmq::socket_type::rep);
    playerConnectionSocket.bind ("tcp://*:5556");

    int id = 1;

    Player player(id, sf::Vector2f(20, 40), sf::Vector2f(390, 500), "");
    EnemyGrid *enemies = new EnemyGrid(4, 2, 50);

    int highScore = 0;

    //create event manager and handlers
    EventManager *eventManager = new EventManager();

    ServerHandler * serverHandler = new ServerHandler(enemies, enemyMutex);
    eventManager->addHandler(std::vector<EventType>{ENEMY_DEATH_EV}, serverHandler);

    Timeline anchorTimeline;
    Timeline gameTime(&anchorTimeline);

    std::thread eventProcessorThread(eventProcessor, eventManager, &gameTime);
    std::thread eventListnerThread(eventListner, eventManager, &gameTime);
    std::thread enemyAIThread(enemyAI, enemies);


    while(true){
        //wait for client request
        zmq::message_t pm;
        zmq::recv_result_t r = playerConnectionSocket.recv(pm, zmq::recv_flags::none);

        //send world to player
        zmq::message_t newResponse(std::to_string(player.id).length());
        memcpy(newResponse.data(), std::to_string(player.id).c_str(), std::to_string(player.id).length());
        playerConnectionSocket.send(newResponse, zmq::send_flags::sndmore);

        zmq::message_t enemiesMsg(enemies->toString().length());
        memcpy(enemiesMsg.data(), enemies->toString().c_str(), enemies->toString().length());
        playerConnectionSocket.send(enemiesMsg, zmq::send_flags::sndmore);

        zmq::message_t playerMsg(player.toString().length());
        memcpy(playerMsg.data(), player.toString().c_str(), player.toString().length());
        playerConnectionSocket.send(playerMsg, zmq::send_flags::none);
    }



}
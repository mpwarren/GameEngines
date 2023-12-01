#include <zmq.hpp>
#include "Timeline.h"
#include "CollidableObject.h"
#include "Player.h"
#include "Event.h"
#include "EventHandler.h"
#include "EventManager.h"
#include "EnemyGrid.h"

std::vector<std::string> parseMessage(std::string strToParse){
    std::istringstream ss(strToParse);
    std::istream_iterator<std::string> begin(ss), end;
    std::vector<std::string> words(begin, end);
    return words;
}

void enemyAI(){
    zmq::context_t context (1);
    zmq::socket_t enemySender (context, zmq::socket_type::push);
    enemySender.bind("tcp://*:5557");



}

int main(){
    
    //set up server connection
    zmq::context_t context (1);
    zmq::socket_t playerConnectionSocket (context, zmq::socket_type::rep);
    playerConnectionSocket.bind ("tcp://*:5556");

    int id = 1;

    Player player(id, sf::Vector2f(20, 40), sf::Vector2f(390, 500), "");
    EnemyGrid enemies(4, 2, 50);

    int highScore = 0;

    while(true){
        //wait for client request
        zmq::message_t pm;
        zmq::recv_result_t r = playerConnectionSocket.recv(pm, zmq::recv_flags::none);

        //send world to player
        zmq::message_t newResponse(std::to_string(player.id).length());
        memcpy(newResponse.data(), std::to_string(player.id).c_str(), std::to_string(player.id).length());
        playerConnectionSocket.send(newResponse, zmq::send_flags::sndmore);

        zmq::message_t enemiesMsg(enemies.toString().length());
        memcpy(enemiesMsg.data(), enemies.toString().c_str(), enemies.toString().length());
        playerConnectionSocket.send(enemiesMsg, zmq::send_flags::sndmore);

        zmq::message_t playerMsg(player.toString().length());
        memcpy(playerMsg.data(), player.toString().c_str(), player.toString().length());
        playerConnectionSocket.send(playerMsg, zmq::send_flags::none);
    }



}
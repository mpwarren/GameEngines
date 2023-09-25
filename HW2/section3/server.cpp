#include <zmq.hpp>
#include <string>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)	Sleep(n)
#endif

std::mutex dataManager;

std::map<int, int> iterationTracker;
int n = 1;


int getN(){
    std::lock_guard<std::mutex> lock(dataManager);
    return n;
}

void addToMap(){
    std::lock_guard<std::mutex> lock(dataManager);
    iterationTracker[n] = 1;
    n++;
}

std::map<int, int> getMap(){
    std::lock_guard<std::mutex> lock(dataManager);
    return iterationTracker;
}

void updateIncrement(int clientNum){
    std::lock_guard<std::mutex> lock(dataManager);
    iterationTracker[clientNum]++;
}

void publishData(){

    zmq::context_t context1 (1);
    zmq::socket_t pubSocket (context1, zmq::socket_type::pub);
    pubSocket.bind("tcp://*:5556");

    while(true){
        int currentN = getN();
        for(auto& kv : getMap()){
            zmq::message_t message(30);
            snprintf ((char *) message.data(), 30 ,"Client: %d Iteration: %d", kv.first, kv.second);
            updateIncrement(kv.first);
            if(kv.first == currentN - 1){
                pubSocket.send(message, zmq::send_flags::none);
                break;
            }
            else{
                pubSocket.send(message, ZMQ_SNDMORE);
            }
        }


        sleep(1);
    }

}

int main () {
    //  Prepare our context and socket
    std::thread publishThread(publishData);

    zmq::context_t context (2);
    zmq::socket_t repSocket (context, zmq::socket_type::rep);
    repSocket.bind ("tcp://*:5555");

    zmq::socket_t internalPub (context, zmq::socket_type::pub);
    internalPub.bind ("tcp://*:5557");


    int n = 0;
    while(true){
        zmq::message_t request;
        repSocket.recv (request, zmq::recv_flags::none);
        std::cout << "Received New Client" << std::endl;

        addToMap();

        //  Send reply back to client
        zmq::message_t reply (0);
        memcpy (reply.data (), "", 0);
        repSocket.send (reply, zmq::send_flags::none);
    }

    publishThread.join();

    return 0;
}
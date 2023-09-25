#include <zmq.hpp>
#include <string>
#include <iostream>
#include <map>
#include <thread>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)	Sleep(n)
#endif

std::map<int, int> iterationTracker;
int n = 1;

void publishData(){

    zmq::context_t context1 (1);
    zmq::socket_t pubSocket (context1, zmq::socket_type::pub);
    pubSocket.bind("tcp://*:5556");

    while(true){
        for(auto& kv : iterationTracker){
            zmq::message_t message(30);
            snprintf ((char *) message.data(), 30 ,"Client: %d Iteration: %d", kv.first, kv.second);
            kv.second++;
            if(kv.first == n - 1){
                pubSocket.send(message, zmq::send_flags::none);
            }
            else{
                pubSocket.send(message, ZMQ_SNDMORE);
            }
        }

        // std::string test = "";
        // zmq::message_t testMessage(test.size());
        // memcpy(testMessage.data(), test.c_str(), test.size());
        // pubSocket.send(testMessage, zmq::send_flags::none);


        sleep(1);
    }

}

int main () {
    //  Prepare our context and socket
    std::thread publishThread(publishData);

    zmq::context_t context (2);
    zmq::socket_t repSocket (context, zmq::socket_type::rep);
    repSocket.bind ("tcp://*:5555");


    while(true){
        zmq::message_t request;
        repSocket.recv (request, zmq::recv_flags::none);
        std::cout << "Received New Client" << std::endl;

        iterationTracker[n] = 1;
        n++;

        //  Send reply back to client
        zmq::message_t reply (0);
        memcpy (reply.data (), "", 0);
        repSocket.send (reply, zmq::send_flags::none);
    }

    publishThread.join();

    return 0;
}
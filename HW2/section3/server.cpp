//
//  Hello World server in C++
//  Binds REP socket to tcp://*:5555
//  Expects "Hello" from client, replies with "World"
//
#include <zmq.hpp>
#include <string>
#include <iostream>
#include <string>
#include <map>
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>

#define sleep(n)	Sleep(n)
#endif

int main () {
    //  Prepare our context and socket
    zmq::context_t context (2);
    zmq::socket_t socket (context, zmq::socket_type::rep);
    socket.bind ("tcp://*:5555");

    //keep track of connected clients
    int n = 1;
    std::map<int, int> m;

    while (true) {
        zmq::message_t request;

        //  Wait for next request from client
        socket.recv (request, zmq::recv_flags::none);
        if(request.to_string() == "N"){
            m[n] = 1;
            n++;
        }

        //  Do some 'work'
        sleep(1);
        

        std::string replyStr = "";
        for(const auto& kv : m){

            zmq::message_t reply (30);
            snprintf((char *) reply.data(), 30, "Client: %d Iteration: %d\n", kv.first, kv.second);

            if(kv.first == n - 1){
                socket.send (reply, zmq::send_flags::none);
            }
            else{
                socket.send (reply, ZMQ_SNDMORE);
            }

            m[kv.first]++;
        }

        //  Send reply back to client
    }
    return 0;
}
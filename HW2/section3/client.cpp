//
//  Hello World client in C++
//  Connects REQ socket to tcp://localhost:5555
//  Sends "Hello" to server, expects "World" back
//
#include <zmq.hpp>
#include <string>
#include <iostream>

int main ()
{
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t reqSocket (context, zmq::socket_type::req);
    zmq::socket_t subSocket (context, zmq::socket_type::sub);

    std::cout << "Connecting to hello world server..." << std::endl;
    reqSocket.connect ("tcp://localhost:5555");
    subSocket.connect ("tcp://localhost:5556");

    subSocket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::message_t request (0);
    memcpy (request.data (), "", 0);
    reqSocket.send (request, zmq::send_flags::none);

    //  Get the reply.
    zmq::message_t reply;
    reqSocket.recv (reply, zmq::recv_flags::none);
    std::cout << "Received World " << request_nbr << std::endl;

    //  Do 10 requests, waiting each time for a response
    for (int request_nbr = 0; request_nbr != 10; request_nbr++) {



        //OTHER SOCKET
        zmq::message_t subMessage;
        subSocket.recv(subMessage, zmq::recv_flags::none);
        std::cout << "subMessage: " << subMessage.to_string() << std::endl;

    }
    return 0;
}
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
    zmq::socket_t socket (context, zmq::socket_type::req);

    socket.connect ("tcp://localhost:5555");


    zmq::message_t firstRequest (1);
    memcpy (firstRequest.data (), "N", 1);
    socket.send (firstRequest, zmq::send_flags::none);


    //  Get the reply.
    zmq::message_t reply;
    socket.recv (reply, zmq::recv_flags::none);
    std::cout << reply.to_string();


    while(1) {
        zmq::message_t request (0);
        socket.send (request, zmq::send_flags::none);

        //  Get the reply.
        int64_t moreMessages = 1;
        while(moreMessages){

            zmq::message_t reply;
            socket.recv (reply, zmq::recv_flags::none);

            std::cout << reply.to_string() << std::endl;


            size_t size = sizeof(moreMessages);

            socket.getsockopt(ZMQ_RCVMORE, &moreMessages, &size);

        }

    }
    return 0;
}
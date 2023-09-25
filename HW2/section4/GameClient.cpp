#include <zmq.hpp>
#include <string>
#include <iostream>

int main ()
{
    //  Prepare our context and socket
    zmq::context_t context (1);
    zmq::socket_t reqSocket (context, zmq::socket_type::req);

    reqSocket.connect ("tcp://localhost:5556");


    while(true) {

        for(int i = 100; i < 10000000; i += 15;){
            zmq::message_t request (3);
            memcpy (request.data (), std::to_string(i).c_str(), 3);
            reqSocket.send (request, zmq::send_flags::none);

            //  Get the reply.
            zmq::message_t reply;
            reqSocket.recv (reply, zmq::recv_flags::none);
        }


    }
    return 0;
}
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>

class GameThread{
    public:
        int i; // an identifier
        ThreadExample *other; // a reference to the "other" thread in this example
        std::mutex *_mutex; // the object for mutual exclusion of execution
        std::condition_variable *_condition_variable; // for messaging between threads

        GameThread(int i, ThreadExample *other, std::mutex *_mutex, std::condition_variable *_condition_variable);

        bool isBusy();

        void run();
}
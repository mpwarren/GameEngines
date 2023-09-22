#include "Timeline.h"
#include <chrono>
#include <cstdint>
#include <iostream>



Timeline::Timeline(Timeline* anchorParam){
    anchor = anchorParam;
    tic = 2;
    startTime = anchor->getTime(); 
    type = 1;
    paused = false;
}

Timeline::Timeline(){
    tic = 1;
    type = 0;
    startTime = getRealTime();
    paused = false;
}

int64_t Timeline::getTime(){
    std::lock_guard<std::mutex> lock(timemutex);
    if(paused){
        return lastPausedTime;
    }
    if (type == 0){
        return getRealTime() - startTime;
    }
    return (anchor->getTime() - startTime) / tic;

}

void Timeline::pause(int64_t pauseTime){
    std::lock_guard<std::mutex> lock(timemutex);
    std::cout << "Pausing!\n";
    lastPausedTime = pauseTime;
    paused = true;
}

int64_t Timeline::unpause(){
    std::lock_guard<std::mutex> lock(timemutex);
    std::cout << "Unpausing!\n";
    paused = false;
    return lastPausedTime;
}

void Timeline::changeTic(int64_t ticRate){
    if(ticRate != tic){
        std::lock_guard<std::mutex> lock(timemutex);
        tic = ticRate;
    }

}

bool Timeline::isPaused(){
    std::lock_guard<std::mutex> lock(timemutex);
    return paused;
}

int64_t Timeline::getRealTime(){
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
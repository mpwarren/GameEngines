#include "Timeline.h"
#include <chrono>
#include <cstdint>



Timeline::Timeline(Timeline* anchorParam, int64_t ticParam){
    anchor = anchorParam;
    tic = ticParam;
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
    if (type == 0){
        return getRealTime() - startTime;
    }
    return (anchor->getTime() - startTime) / tic;
}

void Timeline::pause(){
    std::lock_guard<std::mutex> lock(timemutex);
    lastPausedTime = getTime();
    paused = true;
}

void Timeline::unpause(){
    std::lock_guard<std::mutex> lock(timemutex);
    int64_t elapsedTime = getTime() - lastPausedTime;
    paused = false;
}

void Timeline::changeTic(int64_t newTic){
    std::lock_guard<std::mutex> lock(timemutex);
    tic = newTic;
}

bool Timeline::isPaused(){
    std::lock_guard<std::mutex> lock(timemutex);
    return paused;
}

int64_t Timeline::getRealTime(){
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
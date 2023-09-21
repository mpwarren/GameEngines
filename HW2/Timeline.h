#ifndef TIMELINE_H
#define TIMELINE_H

#include <mutex>

class Timeline{
    public:
        Timeline(Timeline* anchor, int64_t tic);

        Timeline();

        int64_t getTime();

        void pause();

        void unpause();

        void changeTic(int64_t newTic);

        bool isPaused();

    private:
        std::mutex timemutex;
        int64_t startTime;
        int64_t lastPausedTime;
        int64_t tic;
        bool paused;
        Timeline *anchor;
        //0 for base, 1 for other
        int type;

        int64_t getRealTime();
        
};
#endif
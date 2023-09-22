#ifndef TIMELINE_H
#define TIMELINE_H

#include <mutex>
#include <vector>

class Timeline{
    public:
        Timeline(Timeline* anchor);

        Timeline();

        int64_t getTime();

        void pause(int64_t pauseTime);

        int64_t unpause();

        void changeTic(int64_t ticRate);

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
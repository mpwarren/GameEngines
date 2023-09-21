#ifndef TIMELINE_H
#define TIMELINE_H

#include <mutex>

class Timeline{
    public:
        Timeline(Timeline* anchor, int64_t tic);

        Timeline(int type);

        int64_t getTime();

        void pause();

        void unpause();

        void changeTic(int tic);

        bool isPaused();

    private:
        std::Mutex timemutex;
        int64_t startTime;
        int64_t lastPausedTime;
        int64_t tic;
        bool paused;
        Timeline *anchor;
};
#endif
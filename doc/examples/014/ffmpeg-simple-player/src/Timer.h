#ifndef TIMER_H
#define TIMER_H

#include "SDL.h"

typedef void (*TimerOutCb)();

class Timer
{
public:
    Timer();

    void start(void* cb, int interval);

    void stop();

private:
    SDL_TimerID m_timerId = 0;
};

#endif // TIMER_H

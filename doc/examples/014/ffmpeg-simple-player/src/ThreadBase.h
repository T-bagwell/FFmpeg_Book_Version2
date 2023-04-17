#ifndef THREADBASE_H
#define THREADBASE_H

#include <thread>
#include <atomic>

class ThreadBase
{
public:
    ThreadBase();

    virtual void run() = 0;

    void stop();

    void start();

private:
    std::thread *m_th = nullptr;

protected:
    std::atomic<bool> m_stop = false;
};

#endif // THREADBASE_H

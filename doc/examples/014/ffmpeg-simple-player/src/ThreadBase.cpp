#include "ThreadBase.h"

static void ThreadEntry(void *arg)
{
    ThreadBase *th = (ThreadBase*)arg;
    th->run();
}

ThreadBase::ThreadBase()
{

}

void ThreadBase::stop()
{
    m_stop = true;
    if (m_th) {
        m_th->join();
    }
}

void ThreadBase::start()
{
    m_stop = false;
    if (m_th) {
        return;
    }

    m_th = new std::thread(ThreadEntry, this);
}

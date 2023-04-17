#include "PacketQueue.h"

PacketQueue::PacketQueue()
{
    mutex = SDL_CreateMutex();
    cond = SDL_CreateCond();
}

int PacketQueue::packetPut(AVPacket *pkt)
{
    SDL_LockMutex(mutex);

    pkts.push_back(*pkt);
    size += pkt->size;

    SDL_CondSignal(cond);
    SDL_UnlockMutex(mutex);
    return 0;
}

int PacketQueue::packetGet(AVPacket *pkt, std::atomic<bool> &quit)
{
    int ret = 0;

    SDL_LockMutex(mutex);

    for(;;) {
        if (!pkts.empty()) {
            AVPacket &firstPkt = pkts.front();

            size -= firstPkt.size;
            *pkt = firstPkt;

            // remove this packet
            pkts.erase(pkts.begin());

            ret = 1;
            break;
        } else {
            // use SDL_CondWaitTimeout for non-block player
            SDL_CondWaitTimeout(cond, mutex, 500);
        }

        if (quit) {
            ret = -1;
            break;
        }
    }

    SDL_UnlockMutex(mutex);
    return ret;
}

void PacketQueue::packetFlush()
{
    SDL_LockMutex(mutex);

    std::list<AVPacket>::iterator iter;
    for (iter = pkts.begin(); iter != pkts.end(); ++iter) {
        AVPacket &pkt = *iter;
        av_packet_unref(&pkt);
    }
    pkts.clear();

    size = 0;
    SDL_UnlockMutex(mutex);
}

int PacketQueue::packetSize()
{
    return size;
}

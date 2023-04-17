#include "SDLApp.h"

#include <functional>

#include "SDL.h"

#define SDL_APP_EVENT_TIMEOUT (1)

static SDLApp* globalInstance = nullptr;

SDLApp::SDLApp()
{
    SDL_Init(SDL_INIT_EVERYTHING);

    if (!globalInstance) {
        globalInstance = this;
    } else {
        fprintf(stderr, "only one instance allowed\n");
        exit(1);
    }
}

int SDLApp::exec()
{
    SDL_Event event;
    for (;;) {
        SDL_WaitEventTimeout(&event, SDL_APP_EVENT_TIMEOUT);
        switch(event.type) {
        case SDL_QUIT:
            SDL_Quit();
            return 0;
        case SDL_USEREVENT:
        {
            std::function<void()> cb = *(std::function<void()>*)event.user.data1;
            cb();
        }
            break;
        default:
            auto iter = m_userEventMaps.find(event.type);
            if (iter != m_userEventMaps.end()) {
                auto onEventCb = iter->second;
                onEventCb(&event);
            }
            break;
        }
    }
}

void SDLApp::quit()
{
    SDL_Event event;
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
}

void SDLApp::registerEvent(int type, const std::function<void (SDL_Event *)> &cb)
{
    m_userEventMaps[type] = cb;
}

SDLApp *SDLApp::instance()
{
    return globalInstance;
}

#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <SDL.h>
#include <list>
#include <mutex>
#include <libavcodec/avcodec.h>

using namespace std;

struct RenderItem
{
    SDL_Texture *texture;
    SDL_Rect srcRect;
    SDL_Rect dstRect;
};

class RenderView
{
public:
    explicit RenderView();

    void setNativeHandle(void *handle);

    int initSDL();

    /* this method is deprecated */
    RenderItem* createRGB24Texture(int w, int h);

    RenderItem* createYUV420PTexture(int w, int h);

    void updateTexture(RenderItem*item, void *pixelData, int rows);

    void onRefresh();

    void setVideoPicture(AVFrame *frame);

private:
    SDL_Window* m_sdlWindow = nullptr;
    SDL_Renderer* m_sdlRender = nullptr;
    void* m_nativeHandle = nullptr;
    std::list<RenderItem *> m_items;
    mutex m_updateMutex;
};

#endif // RENDERVIEW_H

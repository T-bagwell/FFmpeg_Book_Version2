#include "AudioPlay.h"

AudioPlay::AudioPlay()
{

}

int AudioPlay::openDevice(const SDL_AudioSpec *spec)
{
    m_devId = SDL_OpenAudioDevice(NULL, 0, spec, NULL, 0);
    return m_devId;
}

void AudioPlay::start()
{
    SDL_PauseAudioDevice(m_devId, 0);
}

void AudioPlay::stop()
{
    SDL_PauseAudioDevice(m_devId, 1);
}

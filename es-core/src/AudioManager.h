//
//  AudioManager.h
//
//  Low-level audio functions (using SDL).
//

#pragma once
#ifndef ES_CORE_AUDIO_MANAGER_H
#define ES_CORE_AUDIO_MANAGER_H

#ifdef __linux__
#include <SDL2/SDL_audio.h>
#else
#include "SDL_audio.h"
#endif

#include <memory>
#include <vector>

class Sound;

class AudioManager
{
    static SDL_AudioSpec sAudioFormat;
    static std::vector<std::shared_ptr<Sound>> sSoundVector;
    static std::shared_ptr<AudioManager> sInstance;

    static void mixAudio(void *unused, Uint8 *stream, int len);

    AudioManager();

public:
    static std::shared_ptr<AudioManager> & getInstance();

    void init();
    void deinit();

    void registerSound(std::shared_ptr<Sound> & sound);
    void unregisterSound(std::shared_ptr<Sound> & sound);

    void play();
    void stop();

    virtual ~AudioManager();
};

#endif // ES_CORE_AUDIO_MANAGER_H

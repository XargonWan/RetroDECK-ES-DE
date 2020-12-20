//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  AudioManager.h
//
//  Low-level audio functions (using SDL2).
//

#ifndef ES_CORE_AUDIO_MANAGER_H
#define ES_CORE_AUDIO_MANAGER_H

#include <SDL2/SDL_audio.h>
#include <memory>
#include <vector>

class Sound;

class AudioManager
{
    static SDL_AudioSpec sAudioFormat;
    static std::vector<std::shared_ptr<Sound>> sSoundVector;
    static std::shared_ptr<AudioManager> sInstance;

    static void mixAudio(void* unused, Uint8* stream, int len);

    AudioManager();

public:
    static SDL_AudioDeviceID sAudioDevice;
    static std::shared_ptr<AudioManager>& getInstance();

    void init();
    void deinit();

    void registerSound(std::shared_ptr<Sound>& sound);
    void unregisterSound(std::shared_ptr<Sound>& sound);

    void play();
    void stop();

    virtual ~AudioManager();
};

#endif // ES_CORE_AUDIO_MANAGER_H

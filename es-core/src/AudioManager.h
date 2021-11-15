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
#include <mutex>
#include <vector>

class Sound;

class AudioManager
{
public:
    virtual ~AudioManager();
    static AudioManager& getInstance();

    void init();
    void deinit();

    void registerSound(std::shared_ptr<Sound> sound);
    void unregisterSound(std::shared_ptr<Sound> sound);

    void play();
    void stop();

    // Used for streaming audio from videos.
    void setupAudioStream(int sampleRate);
    void processStream(const void* samples, unsigned count);
    void clearStream();

    void muteStream()
    {
        std::unique_lock<std::mutex> audioLock{mAudioLock};
        sMuteStream = true;
    }
    void unmuteStream()
    {
        std::unique_lock<std::mutex> audioLock{mAudioLock};
        sMuteStream = false;
    }

    bool getHasAudioDevice() { return sHasAudioDevice; }

    inline static SDL_AudioDeviceID sAudioDevice = 0;
    inline static SDL_AudioSpec sAudioFormat;

private:
    AudioManager();

    static void mixAudio(void* unused, Uint8* stream, int len);
    static void mixAudio2(void* unused, Uint8* stream, int len);

    inline static std::mutex mAudioLock;
    inline static SDL_AudioStream* sConversionStream;
    inline static std::vector<std::shared_ptr<Sound>> sSoundVector;
    inline static bool sMuteStream = false;
    inline static bool sHasAudioDevice = true;
};

#endif // ES_CORE_AUDIO_MANAGER_H

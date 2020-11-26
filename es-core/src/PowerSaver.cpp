//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  PowerSaver.cpp
//
//  Power saving functions.
//

#include "PowerSaver.h"

#include "AudioManager.h"
#include "Settings.h"

#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__)
#include <sstream>
#endif

bool PowerSaver::mState = false;
bool PowerSaver::mRunningScreensaver = false;

int PowerSaver::mWakeupTimeout = -1;
int PowerSaver::mScreensaverTimeout = -1;
PowerSaver::mode PowerSaver::mMode = PowerSaver::DISABLED;

void PowerSaver::init()
{
    setState(true);
    updateMode();
}

int PowerSaver::getTimeout()
{
    if (SDL_GetAudioStatus() == SDL_AUDIO_PAUSED)
        AudioManager::getInstance()->deinit();

    // Used only for SDL_WaitEventTimeout. Use `getMode()` for modes.
    return mRunningScreensaver ? mWakeupTimeout : mScreensaverTimeout;
}

void PowerSaver::loadWakeupTime()
{
    // TODO : Move this to Screensaver Class.
    std::string behaviour = Settings::getInstance()->getString("ScreensaverType");
    if (behaviour == "video")
        mWakeupTimeout = Settings::getInstance()->getInt("ScreensaverSwapVideoTimeout") - getMode();
    else if (behaviour == "slideshow")
        mWakeupTimeout = Settings::getInstance()->getInt("ScreensaverSwapImageTimeout") - getMode();
    else // Dim and Blank.
        mWakeupTimeout = -1;
}

void PowerSaver::updateTimeouts()
{
    mScreensaverTimeout = static_cast<unsigned int>(Settings::getInstance()->
            getInt("ScreensaverTimer"));
    mScreensaverTimeout = mScreensaverTimeout > 0 ? mScreensaverTimeout - getMode() : -1;
    loadWakeupTime();
}

PowerSaver::mode PowerSaver::getMode()
{
    return mMode;
}

void PowerSaver::updateMode()
{
    std::string mode = Settings::getInstance()->getString("PowerSaverMode");

    if (mode == "disabled") {
        mMode = DISABLED;
    }
    else if (mode == "instant") {
        mMode = INSTANT;
    }
    else if (mode == "enhanced") {
        mMode = ENHANCED;
    }
    else {
        mMode = DEFAULT;
    }
    updateTimeouts();
}

bool PowerSaver::getState()
{
    return mState;
}

void PowerSaver::setState(bool state)
{
    bool ps_enabled = Settings::getInstance()->getString("PowerSaverMode") != "disabled";
    mState = ps_enabled && state;
}

void PowerSaver::runningScreensaver(bool state)
{
    mRunningScreensaver = state;
    if (mWakeupTimeout < mMode) {
        // Disable PS if wake up time is less than mode as PS will never trigger.
        setState(!state);
    }
}

bool PowerSaver::isScreensaverActive()
{
    return mRunningScreensaver;
}

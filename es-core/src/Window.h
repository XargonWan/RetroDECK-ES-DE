//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  Window.h
//
//  Window management, screensaver management, and help prompts.
//  The input stack starts here as well, as this is the first instance called by InputManager.
//

#ifndef ES_CORE_WINDOW_H
#define ES_CORE_WINDOW_H

#include "resources/TextureResource.h"
#include "HelpPrompt.h"
#include "InputConfig.h"
#include "Settings.h"

#include <memory>

class FileData;
class Font;
class GuiComponent;
class HelpComponent;
class ImageComponent;
class InputConfig;
class TextCache;
class Transform4x4f;
struct HelpStyle;

class Window
{
public:
    class Screensaver
    {
    public:
        virtual bool allowSleep() = 0;
        virtual bool isScreensaverActive() = 0;

        virtual void startScreensaver(bool generateMediaList) = 0;
        virtual void stopScreensaver() = 0;
        virtual void nextGame() = 0;
        virtual void launchGame() = 0;
        virtual void goToGame() = 0;

        virtual void renderScreensaver() = 0;
        virtual void update(int deltaTime) = 0;

        virtual FileData* getCurrentGame() = 0;
        virtual void triggerNextGame() = 0;
    };

    class InfoPopup
    {
    public:
        virtual void render(const Transform4x4f& parentTrans) = 0;
        virtual void stop() = 0;
        virtual ~InfoPopup() {};
    };

    Window();
    ~Window();

    void pushGui(GuiComponent* gui);
    void removeGui(GuiComponent* gui);
    GuiComponent* peekGui();
    inline int getGuiStackSize() { return static_cast<int>(mGuiStack.size()); }

    void textInput(const char* text);
    void input(InputConfig* config, Input input);
    void logInput(InputConfig * config, Input input);
    void update(int deltaTime);
    void render();

    bool init();
    void deinit();

    void normalizeNextUpdate();

    inline bool isSleeping() const { return mSleeping; }
    bool getAllowSleep();
    void setAllowSleep(bool sleep);

    void renderLoadingScreen(std::string text);

    void renderHelpPromptsEarly(); // Used to render HelpPrompts before a fade.
    void setHelpPrompts(const std::vector<HelpPrompt>& prompts, const HelpStyle& style);

    void setScreensaver(Screensaver* screensaver) { mScreensaver = screensaver; }
    void setInfoPopup(InfoPopup* infoPopup) { delete mInfoPopup; mInfoPopup = infoPopup; }
    inline void stopInfoPopup() { if (mInfoPopup) mInfoPopup->stop(); };

    bool isScreensaverActive() { return mRenderScreensaver; };
    void startScreensaver();
    bool stopScreensaver();
    void renderScreensaver();
    void screensaverTriggerNextGame() { mScreensaver->triggerNextGame(); };

    void setLaunchedGame();
    void unsetLaunchedGame();
    bool getGameLaunchedState() { return mGameLaunchedState; };
    void setAllowTextScrolling(bool setting) { mAllowTextScrolling = setting; };
    bool getAllowTextScrolling() { return mAllowTextScrolling; };

    void invalidateCachedBackground()
            { mCachedBackground = false; mInvalidatedCachedBackground = true;};

private:
    void onSleep();
    void onWake();

    // Returns true if at least one component on the stack is processing.
    bool isProcessing();

    HelpComponent* mHelp;
    ImageComponent* mBackgroundOverlay;
    unsigned char mBackgroundOverlayOpacity;
    Screensaver* mScreensaver;
    InfoPopup* mInfoPopup;
    std::vector<GuiComponent*> mGuiStack;
    std::vector<std::shared_ptr<Font>> mDefaultFonts;
    std::unique_ptr<TextCache> mFrameDataText;

    bool mNormalizeNextUpdate;
    int mFrameTimeElapsed;
    int mFrameCountElapsed;
    int mAverageDeltaTime;
    bool mAllowSleep;
    bool mSleeping;
    unsigned int mTimeSinceLastInput;

    bool mRenderScreensaver;
    bool mGameLaunchedState;
    bool mAllowTextScrolling;
    bool mCachedBackground;
    bool mInvalidatedCachedBackground;

    unsigned char mTopOpacity;
    float mTopScale;
    bool mRenderedHelpPrompts;
};

#endif // ES_CORE_WINDOW_H

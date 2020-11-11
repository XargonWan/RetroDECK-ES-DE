//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  SystemScreensaver.cpp
//
//  Screensaver, supporting the following types:
//  Dim, black, slideshow, video.
//

#include "SystemScreensaver.h"

#if defined(_RPI_)
#include "components/VideoPlayerComponent.h"
#endif
#include "components/VideoVlcComponent.h"
#include "resources/Font.h"
#include "utils/FileSystemUtil.h"
#include "utils/StringUtil.h"
#include "views/gamelist/IGameListView.h"
#include "views/ViewController.h"
#include "FileData.h"
#include "Log.h"
#include "PowerSaver.h"
#include "SystemData.h"

#include <random>
#include <time.h>
#include <unordered_map>

#if defined(_WIN64)
#include <cstring>
#endif

#define FADE_TIME 300

SystemScreensaver::SystemScreensaver(
        Window* window)
        : mWindow(window),
        mState(STATE_INACTIVE),
        mImageScreensaver(nullptr),
        mVideoScreensaver(nullptr),
        mCurrentGame(nullptr),
        mPreviousGame(nullptr),
        mTimer(0),
        mVideoChangeTime(30000),
        mHasMediaFiles(false),
        mOpacity(0.0f),
        mDimValue(1.0),
        mRectangleFadeIn(50),
        mTextFadeIn(0),
        mSaturationAmount(1.0)
{
    mWindow->setScreensaver(this);
}

SystemScreensaver::~SystemScreensaver()
{
    mCurrentGame = nullptr;
    delete mVideoScreensaver;
    delete mImageScreensaver;
}

bool SystemScreensaver::allowSleep()
{
    return ((mVideoScreensaver == nullptr) && (mImageScreensaver == nullptr));
}

bool SystemScreensaver::isScreensaverActive()
{
    return (mState != STATE_INACTIVE);
}

void SystemScreensaver::startScreensaver(bool generateMediaList)
{
    std::string path = "";
    std::string screensaverType = Settings::getInstance()->getString("ScreensaverType");
    mHasMediaFiles = false;
    mOpacity = 0.0f;

    // Keep a reference to the default fonts, so they don't keep getting destroyed/recreated.
    if (mGameOverlayFont.empty()) {
        mGameOverlayFont.push_back(Font::get(FONT_SIZE_SMALL));
        mGameOverlayFont.push_back(Font::get(FONT_SIZE_MEDIUM));
        mGameOverlayFont.push_back(Font::get(FONT_SIZE_LARGE));
    }

    // Set mPreviousGame which will be used to avoid showing the same game again during
    // the random selection.
    if ((screensaverType == "slideshow" || screensaverType == "video") && mCurrentGame != nullptr)
        mPreviousGame = mCurrentGame;

    if (screensaverType == "slideshow") {
        if (generateMediaList) {
            mImageFiles.clear();
            mImageCustomFiles.clear();
        }

        // This creates a fade transition between the images.
        mState = STATE_FADE_OUT_WINDOW;

        mVideoChangeTime = Settings::getInstance()->getInt("ScreensaverSwapImageTimeout");

        // Load a random image.
        if (Settings::getInstance()->getBool("ScreensaverSlideshowCustomImages")) {
            if (generateMediaList)
                generateCustomImageList();
            pickRandomCustomImage(path);

            if (mImageCustomFiles.size() > 0)
                mHasMediaFiles = true;
            // Custom images are not tied to the game list.
            mCurrentGame = nullptr;
        }
        else {
            if (generateMediaList)
                generateImageList();
            pickRandomImage(path);
        }

        if (mImageFiles.size() > 0)
            mHasMediaFiles = true;

        // Don't attempt to render the screensaver if there are no images available, but
        // do flag it as running. This way render() will fade to a black screen, i.e. it
        // will activate the 'Black' screensaver type.
        if (mImageFiles.size() > 0 || mImageCustomFiles.size() > 0) {
            if (Settings::getInstance()->getBool("ScreensaverSlideshowGameInfo"))
                generateOverlayInfo();

            if (!mImageScreensaver)
                mImageScreensaver = new ImageComponent(mWindow, false, false);

            mTimer = 0;

            mImageScreensaver->setImage(path);
            mImageScreensaver->setOrigin(0.5f, 0.5f);
            mImageScreensaver->setPosition(Renderer::getScreenWidth() / 2.0f,
                    Renderer::getScreenHeight() / 2.0f);

            if (Settings::getInstance()->getBool("ScreensaverStretchImages"))
                mImageScreensaver->setResize(static_cast<float>(Renderer::getScreenWidth()),
                        static_cast<float>(Renderer::getScreenHeight()));
            else
                mImageScreensaver->setMaxSize(static_cast<float>(Renderer::getScreenWidth()),
                        static_cast<float>(Renderer::getScreenHeight()));
        }
        PowerSaver::runningScreensaver(true);
        mTimer = 0;
        return;
    }
    else if (!mVideoScreensaver && (screensaverType == "video")) {
        if (generateMediaList)
            mVideoFiles.clear();

        // This creates a fade transition between the videos.
        mState = STATE_FADE_OUT_WINDOW;

        mVideoChangeTime = Settings::getInstance()->getInt("ScreensaverSwapVideoTimeout");

        // Load a random video.
        if (generateMediaList)
            generateVideoList();
        pickRandomVideo(path);

        if (mVideoFiles.size() > 0)
            mHasMediaFiles = true;

        if (!path.empty() && Utils::FileSystem::exists(path)) {
            if (Settings::getInstance()->getBool("ScreensaverVideoGameInfo"))
                generateOverlayInfo();

            #if defined(_RPI_)
            // Create the correct type of video component.
            if (Settings::getInstance()->getBool("ScreensaverOmxPlayer"))
                mVideoScreensaver = new VideoPlayerComponent(mWindow);
            else
                mVideoScreensaver = new VideoVlcComponent(mWindow);
            #else
            mVideoScreensaver = new VideoVlcComponent(mWindow);
            #endif

            mVideoScreensaver->topWindow(true);
            mVideoScreensaver->setOrigin(0.5f, 0.5f);
            mVideoScreensaver->setPosition(Renderer::getScreenWidth() / 2.0f,
                    Renderer::getScreenHeight() / 2.0f);

            if (Settings::getInstance()->getBool("ScreensaverStretchVideos"))
                mVideoScreensaver->setResize(static_cast<float>(Renderer::getScreenWidth()),
                        static_cast<float>(Renderer::getScreenHeight()));
            else
                mVideoScreensaver->setMaxSize(static_cast<float>(Renderer::getScreenWidth()),
                        static_cast<float>(Renderer::getScreenHeight()));

            mVideoScreensaver->setVideo(path);
            mVideoScreensaver->setScreensaverMode(true);
            mVideoScreensaver->onShow();
            PowerSaver::runningScreensaver(true);
            mTimer = 0;
            return;
        }
    }
    // No videos or images, just use a standard screensaver.
    mState = STATE_SCREENSAVER_ACTIVE;
    mCurrentGame = nullptr;
}

void SystemScreensaver::stopScreensaver()
{
    delete mVideoScreensaver;
    mVideoScreensaver = nullptr;
    delete mImageScreensaver;
    mImageScreensaver = nullptr;

    mState = STATE_INACTIVE;
    PowerSaver::runningScreensaver(false);

    mDimValue = 1.0;
    mRectangleFadeIn = 50;
    mTextFadeIn = 0;
    mSaturationAmount = 1.0;

    if (mGameOverlay)
        mGameOverlay.release();
}

void SystemScreensaver::nextGame() {
    stopScreensaver();
    startScreensaver(false);
}

void SystemScreensaver::launchGame()
{
    if (mCurrentGame != nullptr) {
        // Launching game
        ViewController::get()->goToGameList(mCurrentGame->getSystem());
        IGameListView* view = ViewController::get()->
                getGameListView(mCurrentGame->getSystem()).get();
        view->setCursor(mCurrentGame);
        ViewController::get()->resetMovingCamera();
        ViewController::get()->launch(mCurrentGame);
    }
}

void SystemScreensaver::renderScreensaver()
{
    std::string screensaverType = Settings::getInstance()->getString("ScreensaverType");
    if (mVideoScreensaver && screensaverType == "video") {
        // Render a black background below the video.
        Renderer::setMatrix(Transform4x4f::Identity());
        Renderer::drawRect(0.0f, 0.0f, Renderer::getScreenWidth(),
                Renderer::getScreenHeight(), 0x000000FF, 0x000000FF);

        // Only render the video if the state requires it.
        if (static_cast<int>(mState) >= STATE_FADE_IN_VIDEO) {
            Transform4x4f transform = Transform4x4f::Identity();
            mVideoScreensaver->render(transform);
        }
    }
    else if (mImageScreensaver && screensaverType == "slideshow") {
        // Render a black background below the image.
        Renderer::setMatrix(Transform4x4f::Identity());
        Renderer::drawRect(0.0f, 0.0f, Renderer::getScreenWidth(),
                Renderer::getScreenHeight(), 0x000000FF, 0x000000FF);

        // Only render the image if the state requires it.
        if (static_cast<int>(mState) >= STATE_FADE_IN_VIDEO) {
            if (mImageScreensaver->hasImage()) {
                mImageScreensaver->setOpacity(255 - static_cast<unsigned char>(mOpacity * 255));

                Transform4x4f transform = Transform4x4f::Identity();
                mImageScreensaver->render(transform);
            }
        }
    }

    if (isScreensaverActive()) {
        Renderer::setMatrix(Transform4x4f::Identity());
        if (Settings::getInstance()->getString("ScreensaverType") == "slideshow") {
            if (mHasMediaFiles) {
                #if defined(USE_OPENGL_21)
                if (Settings::getInstance()->getBool("ScreensaverSlideshowScanlines"))
                    Renderer::shaderPostprocessing(Renderer::SHADER_SCANLINES);
                #endif
                if (Settings::getInstance()->getBool("ScreensaverSlideshowGameInfo") &&
                        mGameOverlay) {
                    if (mGameOverlayRectangleCoords.size() == 4) {
                        Renderer::drawRect(mGameOverlayRectangleCoords[0],
                                mGameOverlayRectangleCoords[1], mGameOverlayRectangleCoords[2],
                                mGameOverlayRectangleCoords[3], 0x00000000 | mRectangleFadeIn,
                                0x00000000 | mRectangleFadeIn );
                    }
                    if (mRectangleFadeIn < 180)
                        mRectangleFadeIn = Math::clamp(mRectangleFadeIn + 4, 0, 255);

                    mGameOverlay.get()->setColor(0xFFFFFF00 | mTextFadeIn);
                    mGameOverlayFont.at(0)->renderTextCache(mGameOverlay.get());
                    if (mTextFadeIn < 255)
                        mTextFadeIn = Math::clamp(mTextFadeIn + 8, 0, 255);
                }
            }
            else {
                // If there are no images, fade in a black screen.
                #if defined(USE_OPENGL_21)
                Renderer::shaderParameters blackParameters;
                blackParameters.fragmentDimValue = mDimValue;
                Renderer::shaderPostprocessing(Renderer::SHADER_DIM, blackParameters);
                #else
                Renderer::drawRect(0.0f, 0.0f, Renderer::getScreenWidth(),
                        Renderer::getScreenHeight(), 0x000000FF, 0x000000FF, mDimValue);
                #endif
                if (mDimValue > 0.0)
                    mDimValue = Math::clamp(mDimValue-0.045, 0.0, 1.0);
            }
        }
        if (Settings::getInstance()->getString("ScreensaverType") == "video") {
            if (mHasMediaFiles) {
                #if defined(USE_OPENGL_21)
                if (Settings::getInstance()->getBool("ScreensaverVideoBlur"))
                    Renderer::shaderPostprocessing(Renderer::SHADER_BLUR_HORIZONTAL);
                if (Settings::getInstance()->getBool("ScreensaverVideoScanlines"))
                    Renderer::shaderPostprocessing(Renderer::SHADER_SCANLINES);
                #endif
                if (Settings::getInstance()->getBool("ScreensaverVideoGameInfo") && mGameOverlay) {
                    if (mGameOverlayRectangleCoords.size() == 4) {
                        #if defined(USE_OPENGL_21)
                        Renderer::shaderPostprocessing(Renderer::SHADER_OPACITY);
                        #endif
                        Renderer::drawRect(mGameOverlayRectangleCoords[0],
                                mGameOverlayRectangleCoords[1], mGameOverlayRectangleCoords[2],
                                mGameOverlayRectangleCoords[3], 0x00000000 | mRectangleFadeIn,
                                0x00000000 | mRectangleFadeIn );
                    }
                    if (mRectangleFadeIn < 180)
                        mRectangleFadeIn = Math::clamp(mRectangleFadeIn + 4, 0, 255);

                    mGameOverlay.get()->setColor(0xFFFFFF00 | mTextFadeIn);
                    mGameOverlayFont.at(0)->renderTextCache(mGameOverlay.get());
                    if (mTextFadeIn < 255)
                        mTextFadeIn = Math::clamp(mTextFadeIn + 8, 0, 255);
                }
            }
            else {
                // If there are no videos, fade in a black screen.
                #if defined(USE_OPENGL_21)
                Renderer::shaderParameters blackParameters;
                blackParameters.fragmentDimValue = mDimValue;
                Renderer::shaderPostprocessing(Renderer::SHADER_DIM, blackParameters);
                #else
                Renderer::drawRect(0.0f, 0.0f, Renderer::getScreenWidth(),
                        Renderer::getScreenHeight(), 0x000000FF, 0x000000FF, mDimValue);
                #endif
                if (mDimValue > 0.0)
                    mDimValue = Math::clamp(mDimValue-0.045, 0.0, 1.0);
            }
        }

        else if (Settings::getInstance()->getString("ScreensaverType") == "dim") {
            #if defined(USE_OPENGL_21)
            Renderer::shaderParameters dimParameters;
            dimParameters.fragmentDimValue = mDimValue;
            Renderer::shaderPostprocessing(Renderer::SHADER_DIM, dimParameters);
            if (mDimValue > 0.4)
                mDimValue = Math::clamp(mDimValue-0.021, 0.4, 1.0);
            dimParameters.fragmentSaturation = mSaturationAmount;
            Renderer::shaderPostprocessing(Renderer::SHADER_DESATURATE, dimParameters);
            if (mSaturationAmount > 0.0)
                mSaturationAmount = Math::clamp(mSaturationAmount-0.035, 0.0, 1.0);
            #else
            Renderer::drawRect(0.0f, 0.0f, Renderer::getScreenWidth(),
                    Renderer::getScreenHeight(), 0x000000A0, 0x000000A0);
            #endif
        }
        else if (Settings::getInstance()->getString("ScreensaverType") == "black") {
            #if defined(USE_OPENGL_21)
            Renderer::shaderParameters blackParameters;
            blackParameters.fragmentDimValue = mDimValue;
            Renderer::shaderPostprocessing(Renderer::SHADER_DIM, blackParameters);
            if (mDimValue > 0.0)
                mDimValue = Math::clamp(mDimValue-0.045, 0.0, 1.0);
            #else
            Renderer::drawRect(0.0f, 0.0f, Renderer::getScreenWidth(),
                    Renderer::getScreenHeight(), 0x000000FF, 0x000000FF);
            #endif
        }
    }
}

void SystemScreensaver::update(int deltaTime)
{
    // Use this to update the fade value for the current fade stage.
    if (mState == STATE_FADE_OUT_WINDOW) {
        mOpacity += static_cast<float>(deltaTime) / FADE_TIME;
        if (mOpacity >= 1.0f) {
            mOpacity = 1.0f;

            // Update to the next state.
            mState = STATE_FADE_IN_VIDEO;
        }
    }
    else if (mState == STATE_FADE_IN_VIDEO) {
        mOpacity -= static_cast<float>(deltaTime) / FADE_TIME;
        if (mOpacity <= 0.0f) {
            mOpacity = 0.0f;
            // Update to the next state.
            mState = STATE_SCREENSAVER_ACTIVE;
        }
    }
    else if (mState == STATE_SCREENSAVER_ACTIVE) {
        // Update the timer that swaps the videos.
        mTimer += deltaTime;
        if (mTimer > mVideoChangeTime)
            nextGame();
    }

    // If we have a loaded video then update it.
    if (mVideoScreensaver)
        mVideoScreensaver->update(deltaTime);
    if (mImageScreensaver)
        mImageScreensaver->update(deltaTime);
}

void SystemScreensaver::generateImageList()
{
    for (auto it = SystemData::sSystemVector.cbegin();
            it != SystemData::sSystemVector.cend(); ++it) {
        // We only want nodes from game systems that are not collections.
        if (!(*it)->isGameSystem() || (*it)->isCollection())
            continue;

        std::vector<FileData*> allFiles = (*it)->getRootFolder()->getFilesRecursive(GAME, true);
        for (auto it = allFiles.begin(); it != allFiles.end(); it++) {
            std::string imagePath = (*it)->getImagePath();
            if (imagePath != "")
                mImageFiles.push_back((*it));
        }
    }
}

void SystemScreensaver::generateVideoList()
{
    for (auto it = SystemData::sSystemVector.cbegin();
            it != SystemData::sSystemVector.cend(); ++it) {
        // We only want nodes from game systems that are not collections.
        if (!(*it)->isGameSystem() || (*it)->isCollection())
            continue;

        std::vector<FileData*> allFiles = (*it)->getRootFolder()->getFilesRecursive(GAME, true);
        for (auto it = allFiles.begin(); it != allFiles.end(); it++) {
            std::string videoPath = (*it)->getVideoPath();
            if (videoPath != "")
                mVideoFiles.push_back((*it));
        }
    }
}

void SystemScreensaver::generateCustomImageList()
{
    std::string imageDir = Utils::FileSystem::expandHomePath(
            Settings::getInstance()->getString("ScreensaverSlideshowImageDir"));

    if (imageDir != "" && Utils::FileSystem::isDirectory(imageDir)) {
        std::string imageFilter = ".jpg, .JPG, .png, .PNG";
        Utils::FileSystem::stringList dirContent = Utils::FileSystem::getDirContent(
                imageDir, Settings::getInstance()->getBool("ScreensaverSlideshowRecurse"));

        for (auto it = dirContent.begin(); it != dirContent.end(); it++) {
            if (Utils::FileSystem::isRegularFile(*it)) {
                if (imageFilter.find(Utils::FileSystem::getExtension(*it)) != std::string::npos)
                    mImageCustomFiles.push_back(*it);
            }
        }
    }
    else {
        LOG(LogWarning) << "Custom screensaver image directory '" <<
                imageDir << "' does not exist.";
    }
}

void SystemScreensaver::pickRandomImage(std::string& path)
{
    mCurrentGame = nullptr;

    if (mImageFiles.size() == 0)
        return;

    if (mImageFiles.size() == 1) {
        mPreviousGame = nullptr;
        mCurrentGame = mImageFiles.front();
        path = mImageFiles.front()->getImagePath();
        return;
    }

    unsigned int index;
    do {
        // Get a random number in range.
        std::random_device randDev;
        //  Mersenne Twister pseudorandom number generator.
        std::mt19937 engine{randDev()};
        std::uniform_int_distribution<int> uniform_dist(0, mImageFiles.size() - 1);
        index = uniform_dist(engine);
    }
    while (mPreviousGame && mImageFiles.at(index) == mPreviousGame);

    path = mImageFiles.at(index)->getImagePath();
    mGameName = mImageFiles.at(index)->getName();
    mSystemName = mImageFiles.at(index)->getSystem()->getFullName();
    mCurrentGame = mImageFiles.at(index);
}

void SystemScreensaver::pickRandomVideo(std::string& path)
{
    mCurrentGame = nullptr;

    if (mVideoFiles.size() == 0)
        return;

    if (mVideoFiles.size() == 1) {
        mPreviousGame = nullptr;
        mCurrentGame = mVideoFiles.front();
        path = mVideoFiles.front()->getVideoPath();
        return;
    }

    unsigned int index;
    do {
        // Get a random number in range.
        std::random_device randDev;
        //  Mersenne Twister pseudorandom number generator.
        std::mt19937 engine{randDev()};
        std::uniform_int_distribution<int> uniform_dist(0, mVideoFiles.size() - 1);
        index = uniform_dist(engine);
    }
    while (mPreviousGame && mVideoFiles.at(index) == mPreviousGame);

    path = mVideoFiles.at(index)->getVideoPath();
    mGameName = mVideoFiles.at(index)->getName();
    mSystemName = mVideoFiles.at(index)->getSystem()->getFullName();
    mCurrentGame = mVideoFiles.at(index);
}

void SystemScreensaver::pickRandomCustomImage(std::string& path)
{
    if (mImageCustomFiles.size() == 0)
        return;

    if (mVideoFiles.size() == 1) {
        mPreviousCustomImage = mImageCustomFiles.front();
        path = mImageCustomFiles.front();
        return;
    }

    unsigned int index;
    do {
        // Get a random number in range.
        std::random_device randDev;
        //  Mersenne Twister pseudorandom number generator.
        std::mt19937 engine{randDev()};
        std::uniform_int_distribution<int> uniform_dist(0, mImageCustomFiles.size() - 1);
        index = uniform_dist(engine);
    }
    while (mPreviousCustomImage != "" && mImageCustomFiles.at(index) == mPreviousCustomImage);

    path = mImageCustomFiles.at(index);
    mPreviousCustomImage = path;
    mGameName = "";
    mSystemName = "";
}

void SystemScreensaver::generateOverlayInfo()
{
    if (mGameName == "" || mSystemName == "")
        return;

    float posX = static_cast<float>(Renderer::getWindowWidth()) * 0.03;
    float posY = static_cast<float>(Renderer::getWindowHeight()) * 0.87;

    const std::string gameName = Utils::String::toUpper(mGameName);
    const std::string systemName = Utils::String::toUpper(mSystemName);
    const std::string overlayText = gameName + "\n" + systemName;

    mGameOverlay = std::unique_ptr<TextCache>(mGameOverlayFont.at(0)->
            buildTextCache(overlayText, posX, posY, 0xFFFFFFFF));

    float textSizeX;
    float textSizeY = mGameOverlayFont[0].get()->sizeText(overlayText).y();

    // There is a weird issue with sizeText() where the X size value is returned
    // as too large if there are two rows in a string and the second row is longer
    // than the first row. Possibly it's the newline character that is somehow
    // injected in the size calculation. Regardless, this workaround is working
    // fine for the time being.
    if (mGameOverlayFont[0].get()->sizeText(gameName).x() >
            mGameOverlayFont[0].get()->sizeText(systemName).x())
        textSizeX = mGameOverlayFont[0].get()->sizeText(gameName).x();
    else
        textSizeX = mGameOverlayFont[0].get()->sizeText(systemName).x();

    float marginX = Renderer::getWindowWidth() * 0.01;

    mGameOverlayRectangleCoords.clear();
    mGameOverlayRectangleCoords.push_back(posX - marginX);
    mGameOverlayRectangleCoords.push_back(posY);
    mGameOverlayRectangleCoords.push_back(textSizeX + marginX * 2);
    mGameOverlayRectangleCoords.push_back(textSizeY);
}

//
//  VideoPlayerComponent.h
//
//  OMXPlayer video playing for Raspberry Pi.
//

#ifdef _RPI_
#pragma once
#ifndef ES_CORE_COMPONENTS_VIDEO_PLAYER_COMPONENT_H
#define ES_CORE_COMPONENTS_VIDEO_PLAYER_COMPONENT_H

#include "components/VideoComponent.h"

void catch_child(int sig_num);

class VideoPlayerComponent : public VideoComponent
{
public:
    VideoPlayerComponent(Window* window, std::string path);
    virtual ~VideoPlayerComponent();

    void render(const Transform4x4f& parentTrans) override;

    // Resize the video to fit this size. If one axis is zero, scale that axis to maintain
    // aspect ratio. If both are non-zero, potentially break the aspect ratio. If both are
    // zero, no resizing. This can be set before or after a video is loaded.
    // setMaxSize() and setResize() are mutually exclusive.
    void setResize(float width, float height) override;

    // Resize the video to be as large as possible but fit within a box of this size.
    // This can be set before or after a video is loaded.
    // Never breaks the aspect ratio. setMaxSize() and setResize() are mutually exclusive.
    void setMaxSize(float width, float height) override;

private:
    // Start the video Immediately.
    virtual void startVideo() override;
    // Stop the video.
    virtual void stopVideo() override;

private:
    pid_t mPlayerPid;
    std::string subtitlePath;
};

#endif // ES_CORE_COMPONENTS_VIDEO_PLAYER_COMPONENT_H
#endif // _RPI_

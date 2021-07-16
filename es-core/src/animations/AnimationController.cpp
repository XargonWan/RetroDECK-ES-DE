//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  AnimationController.cpp
//
//  Basic animation controls.
//

#include "animations/AnimationController.h"

#include "animations/Animation.h"

AnimationController::AnimationController(Animation* anim,
                                         int delay,
                                         std::function<void()> finishedCallback,
                                         bool reverse)
    : mAnimation(anim)
    , mFinishedCallback(finishedCallback)
    , mReverse(reverse)
    , mTime(-delay)
    , mDelay(delay)
{
}

AnimationController::~AnimationController()
{
    if (mFinishedCallback)
        mFinishedCallback();

    delete mAnimation;
}

bool AnimationController::update(int deltaTime)
{
    mTime += deltaTime;

    if (mTime < 0) // Are we still in delay?
        return false;

    float t = static_cast<float>(mTime) / mAnimation->getDuration();

    if (t > 1.0f)
        t = 1.0f;
    else if (t < 0.0f)
        t = 0.0f;

    mAnimation->apply(mReverse ? 1.0f - t : t);

    if (t == 1.0f)
        return true;

    return false;
}

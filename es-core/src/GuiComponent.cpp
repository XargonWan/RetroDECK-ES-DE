//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  GuiComponent.cpp
//
//  Basic GUI component handling such as placement, rotation, Z-order, rendering and animation.
//

#include "GuiComponent.h"

#include "Log.h"
#include "ThemeData.h"
#include "Window.h"
#include "animations/Animation.h"
#include "renderers/Renderer.h"

#include <algorithm>

GuiComponent::GuiComponent(Window* window)
    : mWindow(window)
    , mParent(nullptr)
    , mColor(0)
    , mColorShift(0)
    , mColorShiftEnd(0)
    , mOpacity(255)
    , mSaturation(1.0f)
    , mPosition({})
    , mOrigin(Vector2f::Zero())
    , mRotationOrigin(0.5f, 0.5f)
    , mSize(Vector2f::Zero())
    , mTransform(Renderer::getIdentity())
    , mIsProcessing(false)
    , mVisible(true)
    , mEnabled(true)
{
    for (unsigned char i = 0; i < MAX_ANIMATIONS; i++)
        mAnimationMap[i] = nullptr;
}

GuiComponent::~GuiComponent()
{
    mWindow->removeGui(this);

    cancelAllAnimations();

    if (mParent)
        mParent->removeChild(this);

    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->setParent(nullptr);
}

bool GuiComponent::input(InputConfig* config, Input input)
{
    for (unsigned int i = 0; i < getChildCount(); i++) {
        if (getChild(i)->input(config, input))
            return true;
    }

    return false;
}

void GuiComponent::updateSelf(int deltaTime)
{
    for (unsigned char i = 0; i < MAX_ANIMATIONS; i++)
        advanceAnimation(i, deltaTime);
}

void GuiComponent::updateChildren(int deltaTime)
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->update(deltaTime);
}

void GuiComponent::update(int deltaTime)
{
    updateSelf(deltaTime);
    updateChildren(deltaTime);
}

void GuiComponent::render(const glm::mat4& parentTrans)
{
    if (!isVisible())
        return;

    glm::mat4 trans = parentTrans * getTransform();
    renderChildren(trans);
}

void GuiComponent::renderChildren(const glm::mat4& transform) const
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->render(transform);
}

void GuiComponent::setPosition(float x, float y, float z)
{
    mPosition = glm::vec3(x, y, z);
    onPositionChanged();
}

void GuiComponent::setOrigin(float x, float y)
{
    mOrigin = Vector2f(x, y);
    onOriginChanged();
}

void GuiComponent::setSize(float w, float h)
{
    mSize = Vector2f(w, h);
    onSizeChanged();
}

Vector2f GuiComponent::getCenter() const
{
    return Vector2f(mPosition.x - (getSize().x() * mOrigin.x()) + getSize().x() / 2.0f,
                    mPosition.y - (getSize().y() * mOrigin.y()) + getSize().y() / 2.0f);
}

void GuiComponent::addChild(GuiComponent* cmp)
{
    mChildren.push_back(cmp);

    if (cmp->getParent())
        cmp->getParent()->removeChild(cmp);

    cmp->setParent(this);
}

void GuiComponent::removeChild(GuiComponent* cmp)
{
    if (!cmp->getParent())
        return;

    if (cmp->getParent() != this) {
        LOG(LogError) << "Tried to remove child from incorrect parent!";
    }

    cmp->setParent(nullptr);

    for (auto i = mChildren.cbegin(); i != mChildren.cend(); i++) {
        if (*i == cmp) {
            mChildren.erase(i);
            return;
        }
    }
}

void GuiComponent::sortChildren()
{
    std::stable_sort(mChildren.begin(), mChildren.end(), [](GuiComponent* a, GuiComponent* b) {
        return b->getZIndex() > a->getZIndex();
    });
}

int GuiComponent::getChildIndex() const
{
    std::vector<GuiComponent*>::iterator it =
        std::find(getParent()->mChildren.begin(), getParent()->mChildren.end(), this);

    if (it != getParent()->mChildren.end())
        return static_cast<int>(std::distance(getParent()->mChildren.begin(), it));
    else
        return -1;
}

void GuiComponent::setOpacity(unsigned char opacity)
{
    mOpacity = opacity;
    for (auto it = mChildren.cbegin(); it != mChildren.cend(); it++)
        (*it)->setOpacity(opacity);
}

const glm::mat4& GuiComponent::getTransform()
{
    mTransform = Renderer::getIdentity();
    mTransform = glm::translate(mTransform, mPosition);

    if (mScale != 1.0f)
        mTransform = glm::scale(mTransform, glm::vec3(mScale));

    if (mRotation != 0.0f) {
        // Calculate offset as difference between origin and rotation origin.
        Vector2f rotationSize = getRotationSize();
        float xOff = (mOrigin.x() - mRotationOrigin.x()) * rotationSize.x();
        float yOff = (mOrigin.y() - mRotationOrigin.y()) * rotationSize.y();

        // Transform to offset point.
        if (xOff != 0.0f || yOff != 0.0f)
            mTransform = glm::translate(mTransform, glm::vec3(xOff * -1.0f, yOff * -1.0f, 0.0f));

        // Apply rotation transform.
        mTransform = glm::rotate(mTransform, mRotation, glm::vec3(0.0f, 0.0f, 1.0f));

        // Transform back to original point.
        if (xOff != 0.0f || yOff != 0.0f)
            mTransform = glm::translate(mTransform, glm::vec3(xOff, yOff, 0.0f));
    }
    mTransform = glm::translate(mTransform, glm::vec3(mOrigin.x() * mSize.x() * -1.0f,
                                                      mOrigin.y() * mSize.y() * -1.0f, 0.0f));

    return mTransform;
}

void GuiComponent::textInput(const std::string& text)
{
    for (auto iter = mChildren.cbegin(); iter != mChildren.cend(); iter++)
        (*iter)->textInput(text);
}

void GuiComponent::setAnimation(Animation* anim,
                                int delay,
                                std::function<void()> finishedCallback,
                                bool reverse,
                                unsigned char slot)
{
    assert(slot < MAX_ANIMATIONS);

    AnimationController* oldAnim = mAnimationMap[slot];
    mAnimationMap[slot] = new AnimationController(anim, delay, finishedCallback, reverse);

    if (oldAnim)
        delete oldAnim;
}

bool GuiComponent::stopAnimation(unsigned char slot)
{
    assert(slot < MAX_ANIMATIONS);
    if (mAnimationMap[slot]) {
        delete mAnimationMap[slot];
        mAnimationMap[slot] = nullptr;
        return true;
    }
    else {
        return false;
    }
}

bool GuiComponent::cancelAnimation(unsigned char slot)
{
    assert(slot < MAX_ANIMATIONS);
    if (mAnimationMap[slot]) {
        mAnimationMap[slot]->removeFinishedCallback();
        delete mAnimationMap[slot];
        mAnimationMap[slot] = nullptr;
        return true;
    }
    else {
        return false;
    }
}

bool GuiComponent::finishAnimation(unsigned char slot)
{
    assert(slot < MAX_ANIMATIONS);
    AnimationController* anim = mAnimationMap[slot];
    if (anim) {
        // Skip to animation's end.
        const bool done = anim->update(anim->getAnimation()->getDuration() - anim->getTime());
        if (done) {
            mAnimationMap[slot] = nullptr;
            delete anim; // Will also call finishedCallback.
        }
        return true;
    }
    else {
        return false;
    }
}

bool GuiComponent::advanceAnimation(unsigned char slot, unsigned int time)
{
    assert(slot < MAX_ANIMATIONS);
    AnimationController* anim = mAnimationMap[slot];
    if (anim) {
        bool done = anim->update(time);
        if (done) {
            mAnimationMap[slot] = nullptr;
            delete anim; // Will also call finishedCallback.
        }
        return true;
    }
    else {
        return false;
    }
}

void GuiComponent::stopAllAnimations()
{
    for (unsigned char i = 0; i < MAX_ANIMATIONS; i++)
        stopAnimation(i);
}

void GuiComponent::cancelAllAnimations()
{
    for (unsigned char i = 0; i < MAX_ANIMATIONS; i++)
        cancelAnimation(i);
}

void GuiComponent::applyTheme(const std::shared_ptr<ThemeData>& theme,
                              const std::string& view,
                              const std::string& element,
                              unsigned int properties)
{
    Vector2f scale = getParent() ? getParent()->getSize() :
                                   Vector2f(static_cast<float>(Renderer::getScreenWidth()),
                                            static_cast<float>(Renderer::getScreenHeight()));

    const ThemeData::ThemeElement* elem = theme->getElement(view, element, "");
    if (!elem)
        return;

    using namespace ThemeFlags;
    if (properties & POSITION && elem->has("pos")) {
        Vector2f denormalized = elem->get<Vector2f>("pos") * scale;
        setPosition(glm::vec3(denormalized.x(), denormalized.y(), 0.0f));
    }

    if (properties & ThemeFlags::SIZE && elem->has("size"))
        setSize(elem->get<Vector2f>("size") * scale);

    // Position + size also implies origin.
    if ((properties & ORIGIN || (properties & POSITION && properties & ThemeFlags::SIZE)) &&
        elem->has("origin")) {
        setOrigin(elem->get<Vector2f>("origin"));
    }

    if (properties & ThemeFlags::ROTATION) {
        if (elem->has("rotation"))
            setRotationDegrees(elem->get<float>("rotation"));
        if (elem->has("rotationOrigin"))
            setRotationOrigin(elem->get<Vector2f>("rotationOrigin"));
    }

    if (properties & ThemeFlags::Z_INDEX && elem->has("zIndex"))
        setZIndex(elem->get<float>("zIndex"));
    else
        setZIndex(getDefaultZIndex());

    if (properties & ThemeFlags::VISIBLE && elem->has("visible"))
        setVisible(elem->get<bool>("visible"));
    else
        setVisible(true);
}

void GuiComponent::updateHelpPrompts()
{
    if (getParent()) {
        getParent()->updateHelpPrompts();
        return;
    }

    std::vector<HelpPrompt> prompts = getHelpPrompts();

    if (mWindow->peekGui() == this)
        mWindow->setHelpPrompts(prompts, getHelpStyle());
}

void GuiComponent::onShow()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onShow();
}

void GuiComponent::onHide()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onHide();
}

void GuiComponent::onStopVideo()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onStopVideo();
}

void GuiComponent::onPauseVideo()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onPauseVideo();
}

void GuiComponent::onUnpauseVideo()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onUnpauseVideo();
}

void GuiComponent::onScreensaverActivate()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onScreensaverActivate();
}

void GuiComponent::onScreensaverDeactivate()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onScreensaverDeactivate();
}

void GuiComponent::onGameLaunchedActivate()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onGameLaunchedActivate();
}

void GuiComponent::onGameLaunchedDeactivate()
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->onGameLaunchedDeactivate();
}

void GuiComponent::topWindow(bool isTop)
{
    for (unsigned int i = 0; i < getChildCount(); i++)
        getChild(i)->topWindow(isTop);
}

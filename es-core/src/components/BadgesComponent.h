//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  BadgesComponent.h
//
//  Game badges icons.
//  Used by the gamelist views.
//

#ifndef ES_CORE_COMPONENTS_BADGES_COMPONENT_H
#define ES_CORE_COMPONENTS_BADGES_COMPONENT_H

#include "FlexboxComponent.h"
#include "GuiComponent.h"

struct GameControllers {
    std::string shortName;
    std::string displayName;
    std::string fileName;
};

class BadgesComponent : public GuiComponent
{
public:
    BadgesComponent(Window* window);

    struct BadgeInfo {
        std::string badgeType;
        std::string gameController;
    };

    static void populateGameControllers();
    std::vector<std::string> getBadgeTypes() { return mBadgeTypes; }
    void setBadges(const std::vector<BadgeInfo>& badges);
    static const std::vector<GameControllers>& getGameControllers()
    {
        if (sGameControllers.empty())
            populateGameControllers();
        return sGameControllers;
    }

    static const std::string getShortName(const std::string& displayName);
    static const std::string getDisplayName(const std::string& shortName);

    void render(const glm::mat4& parentTrans) override;
    void onSizeChanged() override { mFlexboxComponent.onSizeChanged(); }

    virtual void applyTheme(const std::shared_ptr<ThemeData>& theme,
                            const std::string& view,
                            const std::string& element,
                            unsigned int properties) override;

private:
    static std::vector<GameControllers> sGameControllers;

    std::vector<FlexboxComponent::FlexboxItem> mFlexboxItems;
    FlexboxComponent mFlexboxComponent;

    std::vector<std::string> mBadgeTypes;
    std::map<std::string, std::string> mBadgeIcons;
};

#endif // ES_CORE_COMPONENTS_BADGES_COMPONENT_H
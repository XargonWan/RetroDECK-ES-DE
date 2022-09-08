//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  BadgeComponent.cpp
//
//  Game badges icons.
//  Used by the gamelist views.
//

#define SLOT_COLLECTION "collection"
#define SLOT_FOLDER "folder"
#define SLOT_FAVORITE "favorite"
#define SLOT_COMPLETED "completed"
#define SLOT_KIDGAME "kidgame"
#define SLOT_BROKEN "broken"
#define SLOT_CONTROLLER "controller"
#define SLOT_ALTEMULATOR "altemulator"

#include "components/BadgeComponent.h"

#include "Log.h"
#include "Settings.h"
#include "ThemeData.h"
#include "resources/TextureResource.h"
#include "utils/StringUtil.h"

namespace
{
    // clang-format off
    // The "unknown" controller entry has to be placed last.
    GameControllers sControllerDefinitions [] {
        // shortName                        displayName                                    fileName
        {"gamepad_generic",                 "Gamepad (Generic)",                           ":/graphics/controllers/gamepad_generic.svg"},
        {"gamepad_nintendo_nes",            "Gamepad (Nintendo NES)",                      ":/graphics/controllers/gamepad_nintendo_nes.svg"},
        {"gamepad_nintendo_snes",           "Gamepad (Nintendo SNES)",                     ":/graphics/controllers/gamepad_nintendo_snes.svg"},
        {"gamepad_nintendo_64",             "Gamepad (Nintendo 64)",                       ":/graphics/controllers/gamepad_nintendo_64.svg"},
        {"gamepad_playstation",             "Gamepad (PlayStation)",                       ":/graphics/controllers/gamepad_playstation.svg"},
        {"gamepad_sega_md_3_buttons",       "Gamepad (Sega Mega Drive/Genesis 3 Buttons)", ":/graphics/controllers/gamepad_sega_md_3_buttons.svg"},
        {"gamepad_sega_md_6_buttons",       "Gamepad (Sega Mega Drive/Genesis 6 Buttons)", ":/graphics/controllers/gamepad_sega_md_6_buttons.svg"},
        {"gamepad_xbox",                    "Gamepad (Xbox)",                              ":/graphics/controllers/gamepad_xbox.svg"},
        {"joystick_generic",                "Joystick (Generic)",                          ":/graphics/controllers/joystick_generic.svg"},
        {"joystick_arcade_no_buttons",      "Joystick (Arcade No Buttons)",                ":/graphics/controllers/joystick_arcade_no_buttons.svg"},
        {"joystick_arcade_1_button",        "Joystick (Arcade 1 Button)",                  ":/graphics/controllers/joystick_arcade_1_button.svg"},
        {"joystick_arcade_2_buttons",       "Joystick (Arcade 2 Buttons)",                 ":/graphics/controllers/joystick_arcade_2_buttons.svg"},
        {"joystick_arcade_3_buttons",       "Joystick (Arcade 3 Buttons)",                 ":/graphics/controllers/joystick_arcade_3_buttons.svg"},
        {"joystick_arcade_4_buttons",       "Joystick (Arcade 4 Buttons)",                 ":/graphics/controllers/joystick_arcade_4_buttons.svg"},
        {"joystick_arcade_5_buttons",       "Joystick (Arcade 5 Buttons)",                 ":/graphics/controllers/joystick_arcade_5_buttons.svg"},
        {"joystick_arcade_6_buttons",       "Joystick (Arcade 6 Buttons)",                 ":/graphics/controllers/joystick_arcade_6_buttons.svg"},
        {"keyboard_generic",                "Keyboard (Generic)",                          ":/graphics/controllers/keyboard_generic.svg"},
        {"keyboard_and_mouse_generic",      "Keyboard and Mouse (Generic)",                ":/graphics/controllers/keyboard_and_mouse_generic.svg"},
        {"mouse_generic",                   "Mouse (Generic)",                             ":/graphics/controllers/mouse_generic.svg"},
        {"mouse_amiga",                     "Mouse (Amiga)",                               ":/graphics/controllers/mouse_amiga.svg"},
        {"lightgun_generic",                "Lightgun (Generic)",                          ":/graphics/controllers/lightgun_generic.svg"},
        {"lightgun_nintendo",               "Lightgun (Nintendo)",                         ":/graphics/controllers/lightgun_nintendo.svg"},
        {"steering_wheel_generic",          "Steering Wheel (Generic)",                    ":/graphics/controllers/steering_wheel_generic.svg"},
        {"flight_stick_generic",            "Flight Stick (Generic)",                      ":/graphics/controllers/flight_stick_generic.svg"},
        {"spinner_generic",                 "Spinner (Generic)",                           ":/graphics/controllers/spinner_generic.svg"},
        {"trackball_generic",               "Trackball (Generic)",                         ":/graphics/controllers/trackball_generic.svg"},
        {"wii_remote_nintendo",             "Wii Remote (Nintendo)",                       ":/graphics/controllers/wii_remote_nintendo.svg"},
        {"wii_remote_and_nunchuk_nintendo", "Wii Remote and Nunchuk (Nintendo)",           ":/graphics/controllers/wii_remote_and_nunchuk_nintendo.svg"},
        {"joycon_left_or_right_nintendo",   "Joy-Con Left or Right (Nintendo)",            ":/graphics/controllers/joycon_left_or_right_nintendo.svg"},
        {"joycon_pair_nintendo",            "Joy-Con Pair (Nintendo)",                     ":/graphics/controllers/joycon_pair_nintendo.svg"},
        {"xbox_kinect",                     "Xbox Kinect",                                 ":/graphics/controllers/xbox_kinect.svg"},
        {"unknown",                         "Unknown Controller",                          ":/graphics/controllers/unknown.svg"}
    };
    // clang-format on
} // namespace

BadgeComponent::BadgeComponent()
    : mFlexboxItems {}
    , mFlexboxComponent {mFlexboxItems}
    , mBadgeTypes {{SLOT_COLLECTION, SLOT_FOLDER, SLOT_FAVORITE, SLOT_COMPLETED, SLOT_KIDGAME,
                    SLOT_BROKEN, SLOT_CONTROLLER, SLOT_ALTEMULATOR}}
{
    mBadgeIcons[SLOT_COLLECTION] = ":/graphics/badge_collection.svg";
    mBadgeIcons[SLOT_FOLDER] = ":/graphics/badge_folder.svg";
    mBadgeIcons[SLOT_FAVORITE] = ":/graphics/badge_favorite.svg";
    mBadgeIcons[SLOT_COMPLETED] = ":/graphics/badge_completed.svg";
    mBadgeIcons[SLOT_KIDGAME] = ":/graphics/badge_kidgame.svg";
    mBadgeIcons[SLOT_BROKEN] = ":/graphics/badge_broken.svg";
    mBadgeIcons[SLOT_CONTROLLER] = ":/graphics/badge_controller.svg";
    mBadgeIcons[SLOT_ALTEMULATOR] = ":/graphics/badge_altemulator.svg";
}

void BadgeComponent::populateGameControllers()
{
    sGameControllers.clear();
    for (auto controller : sControllerDefinitions)
        sGameControllers.emplace_back(controller);
}

void BadgeComponent::setBadges(const std::vector<BadgeInfo>& badges)
{
    std::map<std::string, bool> prevVisibility;
    std::map<std::string, std::string> prevPlayers;
    std::map<std::string, std::string> prevController;

    // Save the visibility status to know whether any badges changed.
    for (auto& item : mFlexboxItems) {
        prevVisibility[item.label] = item.visible;
        if (item.overlayImage.getTexture() != nullptr)
            prevController[item.label] = item.overlayImage.getTexture()->getTextureFilePath();
        item.visible = false;
    }

    for (auto& badge : badges) {
        auto it = std::find_if(
            mFlexboxItems.begin(), mFlexboxItems.end(),
            [badge](FlexboxComponent::FlexboxItem item) { return item.label == badge.badgeType; });

        if (it != mFlexboxItems.end()) {
            // Don't show the alternative emulator badge if the corresponding setting has been
            // disabled.
            if (badge.badgeType == "altemulator" &&
                !Settings::getInstance()->getBool("AlternativeEmulatorPerGame"))
                continue;

            it->visible = true;

            std::string texturePath;
            if (it->overlayImage.getTexture() != nullptr)
                texturePath = it->overlayImage.getTexture()->getTextureFilePath();

            if (badge.badgeType == "folder") {
                if (badge.folderLink)
                    it->overlayImage.setVisible(true);
                else
                    it->overlayImage.setVisible(false);
            }

            if (badge.gameController != "" && badge.gameController != texturePath) {

                auto it2 = std::find_if(sGameControllers.begin(), sGameControllers.end(),
                                        [badge](GameControllers gameController) {
                                            return gameController.shortName == badge.gameController;
                                        });

                if (it2 != sGameControllers.cend()) {
                    it->overlayImage.setImage((*it2).fileName);
                    // This is done to keep the texture cache entry from expiring.
                    mOverlayMap[it2->shortName] =
                        std::make_unique<ImageComponent>(it->overlayImage);
                }
                else if (badge.gameController != "")
                    it->overlayImage.setImage(sGameControllers.back().fileName);
            }
        }
    }

    // Only recalculate the flexbox if any badges changed.
    for (auto& item : mFlexboxItems) {
        if (prevVisibility[item.label] != item.visible ||
            prevController[item.label] != item.label) {
            mFlexboxComponent.onSizeChanged();
            break;
        }
    }
}

const std::string BadgeComponent::getShortName(const std::string& displayName)
{
    auto it = std::find_if(sGameControllers.begin(), sGameControllers.end(),
                           [displayName](GameControllers gameController) {
                               return gameController.displayName == displayName;
                           });
    if (it != sGameControllers.end())
        return (*it).shortName;
    else
        return "unknown";
}

const std::string BadgeComponent::getDisplayName(const std::string& shortName)
{
    auto it = std::find_if(sGameControllers.begin(), sGameControllers.end(),
                           [shortName](GameControllers gameController) {
                               return gameController.shortName == shortName;
                           });
    if (it != sGameControllers.end())
        return (*it).displayName;
    else
        return "unknown";
}

void BadgeComponent::render(const glm::mat4& parentTrans)
{
    if (!isVisible() || mThemeOpacity == 0.0f)
        return;

    if (mOpacity * mThemeOpacity == 1.0f) {
        mFlexboxComponent.render(parentTrans);
    }
    else {
        mFlexboxComponent.setOpacity(mOpacity * mThemeOpacity);
        mFlexboxComponent.render(parentTrans);
        mFlexboxComponent.setOpacity(1.0f);
    }
}

void BadgeComponent::applyTheme(const std::shared_ptr<ThemeData>& theme,
                                const std::string& view,
                                const std::string& element,
                                unsigned int properties)
{
    populateGameControllers();

    using namespace ThemeFlags;

    const ThemeData::ThemeElement* elem {theme->getElement(view, element, "badges")};
    if (!elem)
        return;

    if (elem->has("horizontalAlignment")) {
        const std::string horizontalAlignment {elem->get<std::string>("horizontalAlignment")};
        if (horizontalAlignment != "left" && horizontalAlignment != "right") {
            LOG(LogWarning) << "BadgeComponent: Invalid theme configuration, <horizontalAlignment> "
                               "property defined as \""
                            << horizontalAlignment << "\"";
        }
        else {
            mFlexboxComponent.setAlignment(horizontalAlignment);
        }
    }
    // Legacy themes only.
    else if (elem->has("alignment")) {
        const std::string alignment {elem->get<std::string>("alignment")};
        if (alignment != "left" && alignment != "right") {
            LOG(LogWarning)
                << "BadgeComponent: Invalid theme configuration, <alignment> property defined as \""
                << alignment << "\"";
        }
        else {
            mFlexboxComponent.setAlignment(alignment);
        }
    }

    if (elem->has("direction")) {
        const std::string direction {elem->get<std::string>("direction")};
        if (direction != "row" && direction != "column") {
            LOG(LogWarning)
                << "BadgeComponent: Invalid theme configuration, <direction> property defined as \""
                << direction << "\"";
        }
        else {
            mFlexboxComponent.setDirection(direction);
        }
    }

    mFlexboxComponent.setLines(3);
    if (elem->has("lines")) {
        const unsigned int lines {elem->get<unsigned int>("lines")};
        if (lines < 1 || lines > 10) {
            LOG(LogWarning)
                << "BadgeComponent: Invalid theme configuration, <lines> property defined as \""
                << lines << "\"";
        }
        else {
            mFlexboxComponent.setLines(lines);
        }
    }

    mFlexboxComponent.setItemsPerLine(4);
    if (elem->has("itemsPerLine")) {
        const unsigned int itemsPerLine {elem->get<unsigned int>("itemsPerLine")};
        if (itemsPerLine < 1 || itemsPerLine > 10) {
            LOG(LogWarning) << "BadgeComponent: Invalid theme configuration, <itemsPerLine> "
                               "property defined as \""
                            << itemsPerLine << "\"";
        }
        else {
            mFlexboxComponent.setItemsPerLine(itemsPerLine);
        }
    }

    if (elem->has("itemMargin")) {
        glm::vec2 itemMargin = elem->get<glm::vec2>("itemMargin");
        if ((itemMargin.x != -1.0 && itemMargin.y != -1.0) &&
            (itemMargin.x < 0.0f || itemMargin.x > 0.2f || itemMargin.y < 0.0f ||
             itemMargin.y > 0.2f)) {
            LOG(LogWarning) << "BadgeComponent: Invalid theme configuration, <itemMargin> property "
                               "defined as \""
                            << itemMargin.x << " " << itemMargin.y << "\"";
        }
        else {
            mFlexboxComponent.setItemMargin(itemMargin);
        }
    }

    if (elem->has("slots")) {
        // Replace possible whitespace separators with commas.
        std::string slotsTag = Utils::String::toLower(elem->get<std::string>("slots"));
        for (auto& character : slotsTag) {
            if (std::isspace(character))
                character = ',';
        }
        slotsTag = Utils::String::replace(slotsTag, ",,", ",");
        std::vector<std::string> slots {Utils::String::delimitedStringToVector(slotsTag, ",")};

        // If the "all" value has been set, then populate all badges not already defined.
        if (std::find(slots.begin(), slots.end(), "all") != slots.end()) {
            for (auto& badge : mBadgeTypes) {
                if (std::find(slots.begin(), slots.end(), badge) == slots.end())
                    slots.emplace_back(badge);
            }
        }

        for (auto slot : slots) {
            if (std::find(mBadgeTypes.cbegin(), mBadgeTypes.cend(), slot) != mBadgeTypes.end()) {
                // The "badge_" string is required as ThemeData adds this as a prefix to avoid
                // name collisions when using XML attributes.
                if (properties & PATH && elem->has("badge_" + slot))
                    mBadgeIcons[slot] = elem->get<std::string>("badge_" + slot);

                FlexboxComponent::FlexboxItem item;
                item.label = slot;

                ImageComponent badgeImage {false, false};
                badgeImage.setImage(mBadgeIcons[slot]);
                item.baseImage = badgeImage;
                item.overlayImage = ImageComponent {false, false};

                if (slot == "folder") {
                    if (elem->has("customFolderLinkIcon"))
                        item.overlayImage.setImage(elem->get<std::string>("customFolderLinkIcon"));
                    else
                        item.overlayImage.setImage(":/graphics/badge_folderlink_overlay.svg");

                    if (elem->has("folderLinkPos")) {
                        glm::vec2 folderLinkPos {elem->get<glm::vec2>("folderLinkPos")};
                        folderLinkPos.x = glm::clamp(folderLinkPos.x, -1.0f, 2.0f);
                        folderLinkPos.y = glm::clamp(folderLinkPos.y, -1.0f, 2.0f);
                        item.overlayPosition = folderLinkPos;
                    }

                    if (elem->has("folderLinkSize")) {
                        item.overlaySize =
                            glm::clamp(elem->get<float>("folderLinkSize"), 0.1f, 1.0f);
                    }
                }
                else if (slot == "controller") {
                    if (elem->has("controllerPos")) {
                        glm::vec2 controllerPos {elem->get<glm::vec2>("controllerPos")};
                        controllerPos.x = glm::clamp(controllerPos.x, -1.0f, 2.0f);
                        controllerPos.y = glm::clamp(controllerPos.y, -1.0f, 2.0f);
                        item.overlayPosition = controllerPos;
                    }

                    if (elem->has("controllerSize"))
                        item.overlaySize =
                            glm::clamp(elem->get<float>("controllerSize"), 0.1f, 2.0f);
                }
                mFlexboxItems.emplace_back(std::move(item));
            }
            else {
                LOG(LogError) << "Invalid badge slot \"" << slot << "\" defined";
            }
        }

        for (auto& gameController : sGameControllers) {
            if (properties & PATH && elem->has("controller_" + gameController.shortName))
                gameController.fileName =
                    elem->get<std::string>("controller_" + gameController.shortName);
        }

        GuiComponent::applyTheme(theme, view, element, properties);

        mFlexboxComponent.setPosition(mPosition);
        mFlexboxComponent.setSize(mSize);
        mFlexboxComponent.setOrigin(mOrigin);
        mFlexboxComponent.setRotation(mRotation);
        mFlexboxComponent.setRotationOrigin(mRotationOrigin);
        mFlexboxComponent.setVisible(mVisible);
        mFlexboxComponent.setDefaultZIndex(mDefaultZIndex);
        mFlexboxComponent.setZIndex(mZIndex);
    }
}

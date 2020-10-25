//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  GuiCollectionSystemsOptions.h
//
//  User interface for the game collection settings.
//  Submenu to the GuiMenu main menu.
//

#ifndef ES_APP_GUIS_GUI_COLLECTION_SYSTEM_OPTIONS_H
#define ES_APP_GUIS_GUI_COLLECTION_SYSTEM_OPTIONS_H

#include "components/MenuComponent.h"

template<typename T>
class OptionListComponent;
class SwitchComponent;
class SystemData;

class GuiCollectionSystemsOptions : public GuiComponent
{
public:
    GuiCollectionSystemsOptions(Window* window);
    ~GuiCollectionSystemsOptions();
    bool input(InputConfig* config, Input input) override;

    virtual std::vector<HelpPrompt> getHelpPrompts() override;
    HelpStyle getHelpStyle() override;

private:
    void initializeMenu();
    void applySettings();
    void addSystemsToMenu();
    void addEntry(const char* name, unsigned int color,
            bool add_arrow, const std::function<void()>& func);
    void updateSettings(std::string newAutoSettings, std::string newCustomSettings);
    void createCollection(std::string inName);
    void exitEditMode();
    std::shared_ptr<OptionListComponent<std::string>> autoOptionList;
    std::shared_ptr<OptionListComponent<std::string>> customOptionList;
    std::shared_ptr<SwitchComponent> bundleCustomCollections;
    std::shared_ptr<SwitchComponent> sortFavFirstCustomSwitch;
    std::shared_ptr<SwitchComponent> favoriteStarCustomSwitch;
    std::shared_ptr<SwitchComponent> toggleSystemNameInCollections;
    MenuComponent mMenu;
    SystemData* mSystem;
};

#endif // ES_APP_GUIS_GUI_COLLECTION_SYSTEM_OPTIONS_H

//
//  GuiComplexTextEditPopup.h
//
//  Text edit popup with a title, two text strings, a text input box and buttons
//  to load the second text string and to clear the input field.
//  Intended for updating settings for configuration files and similar.
//

#pragma once
#ifndef ES_CORE_GUIS_GUI_COMPLEX_TEXT_EDIT_POPUP_H
#define ES_CORE_GUIS_GUI_COMPLEX_TEXT_EDIT_POPUP_H

#include "components/ComponentGrid.h"
#include "components/NinePatchComponent.h"
#include "GuiComponent.h"

class TextComponent;
class TextEditComponent;

class GuiComplexTextEditPopup : public GuiComponent
{
public:
    GuiComplexTextEditPopup(
            Window* window,
            const HelpStyle& helpstyle,
            const std::string& title,
            const std::string& infoString1,
            const std::string& infoString2,
            const std::string& initValue,
            const std::function<void(const std::string&)>& okCallback,
            bool multiLine,
            const char* acceptBtnText = "OK",
            const char* saveConfirmationText = "SAVE CHANGES?");

    bool input(InputConfig* config, Input input) override;
    void onSizeChanged() override;

    std::vector<HelpPrompt> getHelpPrompts() override;
    HelpStyle getHelpStyle() override { return mHelpStyle; };

private:
    NinePatchComponent mBackground;
    ComponentGrid mGrid;

    std::shared_ptr<TextComponent> mTitle;
    std::shared_ptr<TextComponent> mInfoString1;
    std::shared_ptr<TextComponent> mInfoString2;
    std::shared_ptr<TextEditComponent> mText;
    std::shared_ptr<ComponentGrid> mButtonGrid;

    HelpStyle mHelpStyle;
    bool mMultiLine;
    std::string mInitValue;
    std::function<void(const std::string&)> mOkCallback;
    std::string mSaveConfirmationText;
};

#endif // ES_CORE_GUIS_GUI_COMPLEX_TEXT_EDIT_POPUP_H

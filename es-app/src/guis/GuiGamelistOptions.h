//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  GuiGamelistOptions.h
//
//  Gamelist options menu for the 'Jump to...' quick selector,
//  game sorting, game filters, and metadata edit.
//
//  The filter interface is covered by GuiGamelistFilter and the
//  metadata edit interface is covered by GuiMetaDataEd.
//

#ifndef ES_APP_GUIS_GUI_GAME_LIST_OPTIONS_H
#define ES_APP_GUIS_GUI_GAME_LIST_OPTIONS_H

#include "FileData.h"
#include "GuiComponent.h"
#include "components/MenuComponent.h"
#include "components/OptionListComponent.h"
#include "utils/StringUtil.h"

class IGameListView;
class SystemData;

class GuiGamelistOptions : public GuiComponent
{
public:
    GuiGamelistOptions(Window* window, SystemData* system);
    virtual ~GuiGamelistOptions();

    virtual bool input(InputConfig* config, Input input) override;
    virtual std::vector<HelpPrompt> getHelpPrompts() override;
    virtual HelpStyle getHelpStyle() override;

private:
    void openGamelistFilter();
    void openMetaDataEd();
    void startEditMode();
    void exitEditMode();

    void jumpToLetter();
    void jumpToFirstRow();

    MenuComponent mMenu;

    using LetterList = OptionListComponent<std::string>;
    std::shared_ptr<LetterList> mJumpToLetterList;

    using SortList = OptionListComponent<const FileData::SortType*>;
    std::shared_ptr<SortList> mListSort;

    SystemData* mSystem;
    IGameListView* getGamelist();
    bool mFoldersOnTop;
    bool mFavoritesSorting;
    bool mOnlyHasFolders;
    bool mFromPlaceholder;
    bool mFiltersChanged;
    bool mCancelled;
    bool mIsCustomCollection;
    bool mIsCustomCollectionGroup;
    SystemData* mCustomCollectionSystem;
    std::vector<std::string> mFirstLetterIndex;
    std::string mCurrentFirstCharacter;
};

#endif // ES_APP_GUIS_GUI_GAME_LIST_OPTIONS_H

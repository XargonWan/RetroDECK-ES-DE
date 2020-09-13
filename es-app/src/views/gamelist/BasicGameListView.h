//
//  BasicGameListView.h
//
//  Interface that defines a GameListView of the type 'basic'.
//

#ifndef ES_APP_VIEWS_GAME_LIST_BASIC_GAME_LIST_VIEW_H
#define ES_APP_VIEWS_GAME_LIST_BASIC_GAME_LIST_VIEW_H

#include "components/TextListComponent.h"
#include "views/gamelist/ISimpleGameListView.h"

class BasicGameListView : public ISimpleGameListView
{
public:
    BasicGameListView(Window* window, FileData* root);

    // Called when a FileData* is added, has its metadata changed, or is removed.
    virtual void onFileChanged(FileData* file, FileChangeType change) override;

    virtual void onThemeChanged(const std::shared_ptr<ThemeData>& theme) override;

    virtual FileData* getCursor() override;
    virtual void setCursor(FileData* file) override;
    virtual FileData* getFirstEntry() override;
    virtual FileData* getLastEntry() override;

    virtual const char* getName() const override { return "basic"; }

    virtual std::vector<HelpPrompt> getHelpPrompts() override;
    virtual void launch(FileData* game) override;

    virtual bool isListScrolling() override { return mList.isScrolling(); };
    virtual void stopListScrolling() override { mList.stopScrolling(); };

protected:
    virtual std::string getQuickSystemSelectRightButton() override;
    virtual std::string getQuickSystemSelectLeftButton() override;
    virtual void populateList(const std::vector<FileData*>& files) override;
    virtual void remove(FileData* game, bool deleteFile) override;
    virtual void removeMedia(FileData* game) override;
    virtual void addPlaceholder();

    TextListComponent<FileData*> mList;

    const std::string FAVORITE_GAME_CHAR = "\uF005";
    const std::string FAVORITE_FOLDER_CHAR = "\uF07C";
};

#endif // ES_APP_VIEWS_GAME_LIST_BASIC_GAME_LIST_VIEW_H

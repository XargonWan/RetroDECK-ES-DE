//  SPDX-License-Identifier: MIT
//
//  EmulationStation Desktop Edition
//  FileData.h
//
//  Provides game file data structures and functions to access and sort this information.
//  Also provides functions to look up paths to media files and for launching games
//  (launching initiated in ViewController).
//

#ifndef ES_APP_FILE_DATA_H
#define ES_APP_FILE_DATA_H

#include "utils/FileSystemUtil.h"
#include "MetaData.h"

#include <unordered_map>

class SystemData;
class Window;
struct SystemEnvironmentData;

enum FileType {
    GAME = 1, // Cannot have children.
    FOLDER = 2,
    PLACEHOLDER = 3
};

// A tree node that holds information for a file.
class FileData
{
public:
    FileData(FileType type, const std::string& path,
            SystemEnvironmentData* envData, SystemData* system);

    virtual ~FileData();

    virtual const std::string& getName();
    const std::string& getSortName();
    const bool getFavorite();
    const bool getKidgame();
    const bool getHidden();
    const bool getCountAsGame();
    const std::pair<unsigned int, unsigned int> getGameCount() { return mGameCount; };
    const bool getExcludeFromScraper();
    const std::vector<FileData*> getChildrenRecursive() const;
    inline FileType getType() const { return mType; }
    inline const std::string& getPath() const { return mPath; }
    inline FileData* getParent() const { return mParent; }
    inline const std::unordered_map<std::string, FileData*>& getChildrenByFilename() const
            { return mChildrenByFilename; }
    inline const std::vector<FileData*>& getChildren() const { return mChildren; }
    inline SystemData* getSystem() const { return mSystem; }
    inline SystemEnvironmentData* getSystemEnvData() const { return mEnvData; }
    const bool getOnlyFoldersFlag() { return mOnlyFolders; }
    const bool getHasFoldersFlag() { return mHasFolders; }
    static const std::string getROMDirectory();
    static const std::string getMediaDirectory();
    const std::string getMediafilePath(std::string subdirectory, std::string mediatype) const;
    const std::string getImagePath() const;
    const std::string get3DBoxPath() const;
    const std::string getCoverPath() const;
    const std::string getMarqueePath() const;
    const std::string getMiximagePath() const;
    const std::string getScreenshotPath() const;
    const std::string getThumbnailPath() const;
    const std::string getVideoPath() const;

    bool getDeletionFlag() { return mDeletionFlag; };
    void setDeletionFlag(bool setting) { mDeletionFlag = setting; };

    const std::vector<FileData*>& getChildrenListToDisplay();
    std::vector<FileData*> getFilesRecursive(unsigned int typeMask,
            bool displayedOnly = false, bool countAllGames = true) const;
    std::vector<FileData*> getScrapeFilesRecursive(bool includeFolders, bool excludeRecursively,
            bool respectExclusions) const;

    void addChild(FileData* file); // Error if mType != FOLDER
    void removeChild(FileData* file); //Error if mType != FOLDER

    inline bool isPlaceHolder() { return mType == PLACEHOLDER; };

    virtual inline void refreshMetadata() { return; };

    virtual std::string getKey();
    const bool isArcadeAsset();
    const bool isArcadeGame();
    inline std::string getFullPath() { return getPath(); };
    inline std::string getFileName() { return Utils::FileSystem::getFileName(getPath()); };
    virtual FileData* getSourceFileData();
    inline std::string getSystemName() const { return mSystemName; };

    // Returns our best guess at the "real" name for this file.
    std::string getDisplayName() const;

    // As above, but also remove parenthesis.
    std::string getCleanName() const;

    void launchGame(Window* window);
    std::string findEmulatorPath(std::string& command);

    typedef bool ComparisonFunction(const FileData* a, const FileData* b);
    struct SortType {
        ComparisonFunction* comparisonFunction;
        std::string description;
        SortType(ComparisonFunction* sortFunction, const std::string& sortDescription)
                : comparisonFunction(sortFunction), description(sortDescription) {}
    };

    void sort(ComparisonFunction& comparator, std::pair<unsigned int, unsigned int>& gameCount);
    void sortFavoritesOnTop(ComparisonFunction& comparator,
            std::pair<unsigned int, unsigned int>& gameCount);
    void sort(const SortType& type, bool mFavoritesOnTop = false);
    MetaDataList metadata;
    // Only count the games, a cheaper alternative to a full sort when that is not required.
    void countGames(std::pair<unsigned int, unsigned int>& gameCount);

    inline void setSortTypeString(std::string typestring) { mSortTypeString = typestring; }
    inline std::string getSortTypeString() { return mSortTypeString; }
    FileData::SortType getSortTypeFromString(std::string desc);

protected:
    FileData* mSourceFileData;
    FileData* mParent;
    std::string mSystemName;
    std::string mSortTypeString = "";

private:
    FileType mType;
    std::string mPath;
    SystemEnvironmentData* mEnvData;
    SystemData* mSystem;
    std::unordered_map<std::string,FileData*> mChildrenByFilename;
    std::vector<FileData*> mChildren;
    std::vector<FileData*> mFilteredChildren;
    // The pair includes all games, and favorite games.
    std::pair<unsigned int, unsigned int> mGameCount;
    bool mOnlyFolders;
    bool mHasFolders;
    // Used for flagging a game for deletion from its gamelist.xml file.
    bool mDeletionFlag;
};

class CollectionFileData : public FileData
{
public:
    CollectionFileData(FileData* file, SystemData* system);
    ~CollectionFileData();
    const std::string& getName();
    void refreshMetadata();
    FileData* getSourceFileData();
    std::string getKey();

private:
    // Needs to be updated when metadata changes.
    std::string mCollectionFileName;
    bool mDirty;
};

#endif // ES_APP_FILE_DATA_H


#ifndef FILELOOKER_H_INCLUDED
#define FILELOOKER_H_INCLUDED

class AdeptIDE;
class ExplorerNode;

#include "INTERFACE/TextBar.h"
#include "INTERFACE/Explorer.h"

class FileSearchEntry {
public:
    std::string withPath;
    std::string withoutPath;
    std::string shortPath;
    size_t deviance;

    FileSearchEntry(const std::string withPath, const std::string withoutPath, const std::string shortPath);
    bool operator<(const FileSearchEntry& other) const;
    void calculateDeviance(const std::string& input);
};

class FileLooker : public TextBar {
    std::vector<FileSearchEntry> possibilities;
    std::vector<FileSearchEntry> matches;
    void addFiles(ExplorerNode *node, std::string root = "");
    void clearFiles();
    void sortFiles();
    void recalculateMatches(const std::string& input);
    FileSearchEntry *getBestMatch(const std::string& input);

public:
    FileLooker();
    void setFiles(ExplorerNode *rootNode);
    std::string look();
    void onType();
};

#endif // EXPLORER_H_INCLUDED

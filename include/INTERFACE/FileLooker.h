
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

    FileSearchEntry(const std::string withPath, const std::string withoutPath);
    bool operator<(const FileSearchEntry& other) const;
};

class FileLooker : public TextBar {
    std::vector<FileSearchEntry> possibilities;
    void addFiles(ExplorerNode *node);
    void clearFiles();
    void sortFiles();

public:
    FileLooker();
    void setFiles(ExplorerNode *rootNode);
    std::string look();
};

#endif // EXPLORER_H_INCLUDED

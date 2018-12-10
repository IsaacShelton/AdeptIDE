
#ifndef FOLDER_WATCHER_H
#define FOLDER_WATCHER_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <string>

class FolderWatcher {
private:
    bool hasHandle = false;
    HANDLE notificationHandle;

public:
    ~FolderWatcher();
    void target(const std::string& folder);
    bool changeOccured();
};

#endif // FOLDER_WATCHER_H

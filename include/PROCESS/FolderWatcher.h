
#ifndef FOLDER_WATCHER_H
#define FOLDER_WATCHER_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/event.h>
#endif

#include <string>

class FolderWatcher {
private:
    #ifdef _WIN32
    bool hasHandle = false;
    HANDLE notificationHandle;
    #elif defined(__APPLE__)
    int kq, dirfd;
    struct kevent direvent, sigevent;
    #endif

  public:
    ~FolderWatcher();
    void target(const std::string& folder);
    bool changeOccured();
};

#endif // FOLDER_WATCHER_H


#ifndef FOLDER_WATCHER_H
#define FOLDER_WATCHER_H

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(__APPLE__)
#include <sys/event.h>
#include <atomic>
#include <thread>
#endif

#include <string>

class FolderWatcher {
private:
    #ifdef _WIN32
    bool hasHandle;
    HANDLE notificationHandle;
    #elif defined(__APPLE__)
    int kq, dirfd;
    struct kevent direvent, sigevent;
    std::thread thread;
    std::atomic_bool shouldStopWatching;
    std::atomic_bool changed;
    #endif
    
    bool isWatching;
    void stopWatching();

public:
    FolderWatcher();
    ~FolderWatcher();
    void target(const std::string& folder);
    bool changeOccured();
};

#endif // FOLDER_WATCHER_H

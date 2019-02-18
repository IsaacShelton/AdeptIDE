
#include <iostream>
#include "PROCESS/FolderWatcher.h"

#ifdef _WIN32
#elif defined(__APPLE__)
#include <errno.h>     // for errno
#include <fcntl.h>     // for O_RDONLY
#include <stdio.h>     // for fprintf()
#include <stdlib.h>    // for EXIT_SUCCESS
#include <string.h>    // for strerror()
#include <sys/event.h> // for kqueue() etc.
#include <unistd.h>    // for close()
#endif

void FolderWatcher::target(const std::string& folder){
    #ifdef _WIN32
    if(hasHandle) FindCloseChangeNotification(notificationHandle);
    notificationHandle = FindFirstChangeNotificationA(folder.c_str(), false, FILE_NOTIFY_CHANGE_LAST_WRITE);
    #elif defined(__APPLE__)
    kq = kqueue();
    dirfd = open(folder.c_str(), O_RDONLY);

    EV_SET(&direvent, dirfd, EVFILT_VNODE, EV_ADD | EV_CLEAR | EV_ENABLE,
           NOTE_WRITE | NOTE_ATTRIB | NOTE_DELETE | NOTE_EXTEND, 0, (void *) "");

    kevent(kq, &direvent, 1, NULL, 0, NULL);

    // Register interest in SIGINT with the queue.  The user data
    // is NULL, which is how we'll differentiate between
    // a directory-modification event and a SIGINT-received event.
    EV_SET(&sigevent, SIGINT, EVFILT_SIGNAL, EV_ADD | EV_ENABLE, 0, 0, NULL);
    
    // kqueue event handling happens after the legacy API, so make
    // sure it doesn eat the signal before the kqueue can see it.
    signal(SIGINT, SIG_IGN);

    // Register the signal event.
    struct timespec timeout = {0, 500};
    kevent(kq, &sigevent, 1, NULL, 0, &timeout);

    shouldStopWatching.store(0);
    changed.store(0);

    thread = std::thread([this]() -> void {
        struct kevent change;
        struct timespec timeout = {0, 500000};

        while(true){
            int res = kevent(kq, NULL, 0, &change, 1, &timeout);

            if(res == -1){
                std::cerr << "kevent failed" << std::endl;
                return;
            }
            if(res != 0 && change.udata != NULL){
                changed.store(1);
            }

            if(shouldStopWatching.load()) return;
        }
    });

    #endif
}

FolderWatcher::~FolderWatcher(){
    #ifdef _WIN32
    if(hasHandle) FindCloseChangeNotification(notificationHandle);
    #elif defined(__APPLE__)
    shouldStopWatching.store(1);
    thread.join();
    close(kq);
    #endif
}

bool FolderWatcher::changeOccured(){
#ifdef _WIN32
    if(WaitForSingleObject(notificationHandle, 1) == WAIT_OBJECT_0){
        FindNextChangeNotification(notificationHandle);
        return true;
    }
#elif defined(__APPLE__)
    if(changed.load()){
        changed.store(0);
        return true;
    }
#endif

    return false;
}

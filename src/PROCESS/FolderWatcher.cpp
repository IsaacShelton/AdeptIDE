
#include "PROCESS/FolderWatcher.h"

void FolderWatcher::target(const std::string& folder){
    if(hasHandle) FindCloseChangeNotification(notificationHandle);
    notificationHandle = FindFirstChangeNotificationA(folder.c_str(), false, FILE_NOTIFY_CHANGE_LAST_WRITE);
}

FolderWatcher::~FolderWatcher(){
    if(hasHandle) FindCloseChangeNotification(notificationHandle);
}

bool FolderWatcher::changeOccured(){
    if(WaitForSingleObject(notificationHandle, 1) == WAIT_OBJECT_0){
        FindNextChangeNotification(notificationHandle);
        return true;
    }

    return false;
}

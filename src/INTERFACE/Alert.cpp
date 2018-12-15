
#include "INTERFACE/Alert.h"

#ifdef _WIN32

#elif defined(__APPLE__)
void macMessageBox(const char *title, const char *text){
    SInt32 nRes = 0;
    CFUserNotificationRef pDlg = NULL;
    const void *keys[] = {kCFUserNotificationAlertHeaderKey, kCFUserNotificationAlertMessageKey};
    CFStringRef cfTitle = CFStringCreateWithCString(NULL, title, kCFStringEncodingWindowsLatin1);
    CFStringRef cfText = CFStringCreateWithCString(NULL, text, kCFStringEncodingWindowsLatin1);
    const void *vals[] = {cfTitle, cfText};
    CFDictionaryRef dict = CFDictionaryCreate(0, keys, vals, sizeof(keys) / sizeof(*keys), &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    pDlg = CFUserNotificationCreate(kCFAllocatorDefault, 0, kCFUserNotificationPlainAlertLevel, &nRes, dict);
    CFRelease(cfTitle);
    CFRelease(cfText);
}
#endif

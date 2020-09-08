
#include <ctype.h>
#include "UTIL/util.h" // (from insight)
#include "UTIL/ground.h" // (from insight)
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

#elif defined(__linux__)
void linuxSanitize(char *inout_text){
    length_t length = strlen(inout_text);
    for(length_t i = 0; i < length; i++){
        char c = inout_text[i];
        
        // Whitelist
        if(isalnum(c)) continue;
        if(c == '_' || c == ' ') continue;
        if(c == '.' || c == '?') continue;

        // Don't allow anything else
        // Move everything afterwards forward one character (include \0)
        memmove(&inout_text[i], &inout_text[i + 1], sizeof(char) * (length - i));

        // Reduce length by one and prevent loop from going to next character
        i--;
        length--;
    }
}

void linuxMessageBox(const char *raw_title, const char *raw_text){
    length_t title_length = strlen(raw_title);
    length_t text_length = strlen(raw_text);

    strong_cstr_t title = static_cast<strong_cstr_t>(malloc(title_length + 1));
    strong_cstr_t text = static_cast<strong_cstr_t>(malloc(text_length + 1));

    linuxSanitize(title);
    linuxSanitize(text);

    // TODO: Not use system?
    strong_cstr_t command = mallocandsprintf("zenity --info --title=\"%s\" --text=\"%s\"", title, text);
    system(command);
    free(command);
    free(title);
    free(text);
}
#endif

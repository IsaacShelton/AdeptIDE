
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

char* filename_path(const char *filename){
    size_t i;
    size_t filename_length = strlen(filename);
    char *path;

    if(filename_length == 0){
        path = static_cast<char*>(malloc(1));
        path[0] = '\0';
        return path;
    }

    for(i = filename_length - 1 ; i != 0; i--){
        if(filename[i] == '/' || filename[i] == '\\'){
            path = static_cast<char*>(malloc(i + 2));
            memcpy(path, filename, i + 1);
            path[i + 1] = '\0';
            return path;
        }
    }

    path = static_cast<char*>(malloc(1));
    path[0] = '\0';
    return path;
}

char* filename_absolute(const char *filename){
    #if defined(_WIN32) || defined(_WIN64)

    char *buffer = static_cast<char*>(malloc(512));
    if(GetFullPathName(filename, 512, buffer, NULL) == 0){
        free(buffer);
        return NULL;
    }
    return buffer;
    #else
    return realpath(filename, NULL);
    #endif

    return NULL;
}

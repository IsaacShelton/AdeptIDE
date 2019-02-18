
#ifndef FILENAME_UTIL_H
#define FILENAME_UTIL_H

#include <string>

char* filename_path(const char *filename);
char* filename_absolute(const char *filename);

std::string filename_get_extension(const std::string &filename);

#endif // FILENAME_UTIL_H

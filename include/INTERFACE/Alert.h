
#ifndef ALERT_H_INCLUDED
#define ALERT_H_INCLUDED

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define alertError(t, m) MessageBox(NULL, t, m, MB_OK | MB_ICONERROR)
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#define alertError(t, m) macMessageBox(t, m)
void macMessageBox(const char *title, const char *text);
#endif

#endif // ALERT_H_INCLUDED

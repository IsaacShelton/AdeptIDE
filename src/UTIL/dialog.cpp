
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "UTIL/dialog.h"

#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define DIALOG_FILTER "Source Code\0*.adept;*.java;*.html\0Adept Source Code (*.adept)\0*.adept\0Java Source Code (*.java)\0*.java\0HTML Document (*.html)\0*.html\0All files (*.*)\0*.*\0"

bool openFileDialog(GLFWwindow* window, std::string& output){
    char *filename = static_cast<char*>(malloc(sizeof(char) * 65546));

    OPENFILENAME open_filename_info;
    ZeroMemory(&filename, sizeof( filename ));
    ZeroMemory(&open_filename_info, sizeof( open_filename_info ));
    open_filename_info.lStructSize = sizeof( open_filename_info );
    open_filename_info.hwndOwner = glfwGetWin32Window(window);
    open_filename_info.lpstrFilter = DIALOG_FILTER;
    open_filename_info.lpstrFile = filename;
    open_filename_info.nMaxFile = 65546;
    open_filename_info.lpstrTitle = "Open file";
    open_filename_info.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_EXPLORER;

    if (GetOpenFileNameA( &open_filename_info )){
        output = filename;
        free(filename);
        return true;
    } else {
        switch (CommDlgExtendedError()){
        case CDERR_DIALOGFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_DIALOGFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_FINDRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_FINDRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_INITIALIZATION: MessageBox(NULL, "FATAL ERROR: CDERR_INITIALIZATION",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_LOADRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOADRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_LOADSTRFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOADSTRFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_LOCKRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOCKRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_MEMALLOCFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_MEMALLOCFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_MEMLOCKFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_MEMLOCKFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_NOHINSTANCE: MessageBox(NULL, "FATAL ERROR: CDERR_NOHINSTANCE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_NOHOOK: MessageBox(NULL, "FATAL ERROR: CDERR_NOHOOK",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_NOTEMPLATE: MessageBox(NULL, "FATAL ERROR: CDERR_NOTEMPLATE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case CDERR_STRUCTSIZE: MessageBox(NULL, "FATAL ERROR: CDERR_STRUCTSIZE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case FNERR_BUFFERTOOSMALL: MessageBox(NULL, "FATAL ERROR: Filename is too long",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case FNERR_INVALIDFILENAME: MessageBox(NULL, "FATAL ERROR: Invalid filename",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        case FNERR_SUBCLASSFAILURE: MessageBox(NULL, "FATAL ERROR: FNERR_SUBCLASSFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); free(filename); return false;
        default: free(filename); return false;
        }
    }

    // We should never get here
    free(filename);
    return false;
}

bool openMultipleFileDialog(GLFWwindow* window, std::vector<std::string>& output){
    char filename[1024];

    OPENFILENAME open_filename_info;
    ZeroMemory(&filename, sizeof( filename ));
    ZeroMemory(&open_filename_info, sizeof( open_filename_info ));
    open_filename_info.lStructSize = sizeof( open_filename_info );
    open_filename_info.hwndOwner = glfwGetWin32Window(window);
    open_filename_info.lpstrFilter = DIALOG_FILTER;
    open_filename_info.lpstrFile = filename;
    open_filename_info.nMaxFile = MAX_PATH;
    open_filename_info.lpstrTitle = "Open file";
    open_filename_info.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    if (GetOpenFileNameA( &open_filename_info )){
        char* str = open_filename_info.lpstrFile;
        std::string directory = str;
        str += directory.length() + 1;

        if(*str){
            while(*str){
              std::string filename = str;
              str += filename.length() + 1;
              output.push_back(directory + "\\" + filename);
            }
        } else {
            // Directory isn't really a directory lol, it's actually a single filename
            output.push_back(directory);
        }
        return true;
    } else {
        switch (CommDlgExtendedError()){
        case CDERR_DIALOGFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_DIALOGFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_FINDRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_FINDRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_INITIALIZATION: MessageBox(NULL, "FATAL ERROR: CDERR_INITIALIZATION",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_LOADRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOADRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_LOADSTRFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOADSTRFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_LOCKRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOCKRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_MEMALLOCFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_MEMALLOCFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_MEMLOCKFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_MEMLOCKFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_NOHINSTANCE: MessageBox(NULL, "FATAL ERROR: CDERR_NOHINSTANCE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_NOHOOK: MessageBox(NULL, "FATAL ERROR: CDERR_NOHOOK",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_NOTEMPLATE: MessageBox(NULL, "FATAL ERROR: CDERR_NOTEMPLATE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_STRUCTSIZE: MessageBox(NULL, "FATAL ERROR: CDERR_STRUCTSIZE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case FNERR_BUFFERTOOSMALL: MessageBox(NULL, "FATAL ERROR: Filename is too long",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case FNERR_INVALIDFILENAME: MessageBox(NULL, "FATAL ERROR: Invalid filename",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case FNERR_SUBCLASSFAILURE: MessageBox(NULL, "FATAL ERROR: FNERR_SUBCLASSFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        default: return false;
        }
    }

    // We should never get here
    return false;
}

bool saveFileDialog(GLFWwindow* window, std::string& output){
    char filename[1024];

    OPENFILENAME open_filename_info;
    ZeroMemory(&filename, sizeof( filename ));
    ZeroMemory(&open_filename_info, sizeof( open_filename_info ));
    open_filename_info.lStructSize = sizeof( open_filename_info );
    open_filename_info.hwndOwner = glfwGetWin32Window(window);
    open_filename_info.lpstrFilter = DIALOG_FILTER;
    open_filename_info.lpstrFile = filename;
    open_filename_info.nMaxFile = MAX_PATH;
    open_filename_info.lpstrTitle = "Save file";
    open_filename_info.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_ALLOWMULTISELECT | OFN_EXPLORER;

    if (GetSaveFileNameA( &open_filename_info )){
        output = filename;
        return true;
    } else {
        switch (CommDlgExtendedError()){
        case CDERR_DIALOGFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_DIALOGFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_FINDRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_FINDRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_INITIALIZATION: MessageBox(NULL, "FATAL ERROR: CDERR_INITIALIZATION",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_LOADRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOADRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_LOADSTRFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOADSTRFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_LOCKRESFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_LOCKRESFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_MEMALLOCFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_MEMALLOCFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_MEMLOCKFAILURE: MessageBox(NULL, "FATAL ERROR: CDERR_MEMLOCKFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_NOHINSTANCE: MessageBox(NULL, "FATAL ERROR: CDERR_NOHINSTANCE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_NOHOOK: MessageBox(NULL, "FATAL ERROR: CDERR_NOHOOK",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_NOTEMPLATE: MessageBox(NULL, "FATAL ERROR: CDERR_NOTEMPLATE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case CDERR_STRUCTSIZE: MessageBox(NULL, "FATAL ERROR: CDERR_STRUCTSIZE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case FNERR_BUFFERTOOSMALL: MessageBox(NULL, "FATAL ERROR: Filename is too long",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case FNERR_INVALIDFILENAME: MessageBox(NULL, "FATAL ERROR: Invalid filename",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        case FNERR_SUBCLASSFAILURE: MessageBox(NULL, "FATAL ERROR: FNERR_SUBCLASSFAILURE",  "Fatal Error", MB_OK | MB_ICONERROR); return false;
        default: return false;
        }
    }

    // We should never get here
    return false;
}
#elif !defined(__APPLE__)
// Non-apple dialog
#endif

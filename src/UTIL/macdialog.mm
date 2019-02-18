
#include "UTIL/dialog.h"

#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#import <Cocoa/Cocoa.h>

bool openFileDialog(GLFWwindow *window, std::string &output){
    NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];

    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    [openDlg setCanChooseFiles:YES];
    [openDlg setCanChooseDirectories:NO];
    
    if([openDlg runModal] == NSModalResponseOK){
        NSURL* url = [openDlg URL];
        output = std::string([[url path] UTF8String]);

        [keyWindow makeKeyAndOrderFront:nil];
        return true;
    }

    [keyWindow makeKeyAndOrderFront:nil];
    return false;
}

bool openFolderDialog(GLFWwindow *window, std::string &output){
    NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];

    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    [openDlg setCanChooseFiles:NO];
    [openDlg setCanChooseDirectories:YES];
    
    if([openDlg runModal] == NSModalResponseOK){
        NSURL* url = [openDlg URL];
        output = std::string([[url path] UTF8String]);

        [keyWindow makeKeyAndOrderFront:nil];
        return true;
    }

    [keyWindow makeKeyAndOrderFront:nil];
    return false;
}

bool openMultipleFileDialog(GLFWwindow *window, std::vector<std::string> &output){
    NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];

    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    [openDlg setCanChooseFiles:YES];
    [openDlg setAllowsMultipleSelection:YES];
    [openDlg setCanChooseDirectories:NO];
    
    if([openDlg runModal] == NSModalResponseOK){
        NSArray* urls = [openDlg URLs];
        
        for(int i = 0; i < [urls count]; i++ ){
            NSURL* url = [urls objectAtIndex:i];
            output.push_back(std::string([[url path] UTF8String]));
        }

        [keyWindow makeKeyAndOrderFront:nil];
        return true;
    }

    [keyWindow makeKeyAndOrderFront:nil];
    return false;
}

bool saveFileDialog(GLFWwindow *window, std::string &output){
    NSWindow *keyWindow = [[NSApplication sharedApplication] keyWindow];

    NSSavePanel *save = [NSSavePanel savePanel];
    [save setParentWindow:glfwGetCocoaWindow(window)];
    [save setAllowedFileTypes:nil];
    [save setAllowsOtherFileTypes:YES];
    
    NSInteger result = [save runModal];
    
    if (result == NSModalResponseOK){
        NSString *selectedFile = [[save URL] path];
        output = std::string([selectedFile UTF8String]);
        [keyWindow makeKeyAndOrderFront:nil];
        return true;
    }

    [keyWindow makeKeyAndOrderFront:nil];
    return false;
}


#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include "UTIL/strings.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"
#include "INTERFACE/AdeptIDE.h"
#include "INTERFACE/CommandRunner.h"

CommandRunner::CommandRunner(){
    this->container = NULL;
    this->constantText = "Run Command: ";
}

CommandResult CommandRunner::run(void *adeptide_ref){
    AdeptIDE *adeptide = (AdeptIDE*) adeptide_ref;
    std::string command = string_flatten(this->getInput());

    if(command == "cdhere"){
        bool success = adeptide->cdFile();
        return CommandResult(success, success ? "" : "No active file or folder to change directory to");
    }

    if(command == "new" || command == "newadept"){
        adeptide->newFile(FileType::ADEPT);
        return CommandResult(true, "");
    }

    if(command == "newplain" || command == "newplaintext" || command == "newtext"){
        adeptide->newFile(FileType::PLAIN_TEXT);
        return CommandResult(true, "");
    }

    if(command == "newjava"){
        adeptide->newFile(FileType::JAVA);
        return CommandResult(true, "");
    }

    if(command == "newhtml"){
        adeptide->newFile(FileType::HTML);
        return CommandResult(true, "");
    }

    if(command == "insight" || command == "updateinsight"){
        adeptide->updateInsight(true);
        return CommandResult(true, "");
    }

    if(command == "openfolder"){
        adeptide->openFolder();
        return CommandResult(true, "");
    }

    if(command == "open" || command == "openfile"){
        adeptide->openFile();
        return CommandResult(true, "");
    }

    if(command == "explorer" || command == "togglexplorer"){
        if(adeptide->explorer){
            adeptide->explorer->toggleVisibility();
            return CommandResult(true, "");
        }
        return CommandResult(false, "Explorer doesn't exist");
    }

    if(command == "terminal" || command == "toggleterminal"){
        if(adeptide->terminal){
            adeptide->terminal->toggleVisibility();
            return CommandResult(true, "");
        }
        return CommandResult(false, "Terminal doesn't exist");
    }

    if(command == "save" || command == "savefile"){
        adeptide->saveFile();
        return CommandResult(true, "");
    }

    if(command == "run" || command == "runfile"){
        adeptide->runFile(); // Error is handled inside
        return CommandResult(true, "");
    }

    if(command == "settings"){
        settings_menu(adeptide);
        return CommandResult(true, "");
    }

    if(command == "maximize"){
        glfwMaximizeWindow(adeptide->window);
        return CommandResult(true, "");
    }
    
    return CommandResult(false, "Unknown Command");
}

CommandResult::CommandResult(bool successful, const std::string& message){
    this->successful = successful;
    this->message = message;
}

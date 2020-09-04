
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include "UTIL/strings.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"
#include "INTERFACE/AdeptIDE.h"
#include "INTERFACE/CommandRunner.h"
#include "UTIL/levenshtein.h" // NOTE: From insight

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

    if(command == "newjson"){
        adeptide->newFile(FileType::JSON);
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
    
    return CommandResult(false, "Unknown Command \"" + command + "\"");
}

void CommandRunner::onType(){
    std::string input = this->getInput();
    std::string result = "";

    // Lazy command suggestions
    const static std::vector<std::string> command_list = {
        "cdhere", "new", "newadept", "newplain", "newplaintext", "newtext", "newjava", "newhtml", "newjson",
        "insight", "updateinsight", "openfolder", "open", "openfile", "explorer", "toggleexplorer", "terminal", "toggleterminal",
        "save", "savefile", "run", "runfile", "settings", "maximize"
    };

    std::vector<SymbolWeight> suggestions;

    for(size_t i = 0; i != command_list.size(); i++){
        if(command_list[i].length() < input.length()) continue;
        int distance = levenshtein_overlapping(input.c_str(), command_list[i].c_str());
        suggestions.push_back(SymbolWeight(command_list[i], command_list[i], distance, SymbolWeight::Kind::NONE, NULL_SOURCE));
    }
    
    std::stable_sort(suggestions.begin(), suggestions.end());

    size_t max_matches = 10;
    for(const SymbolWeight& suggestion : suggestions){
        result += suggestion.label + "\n";
        if(--max_matches == 0) break;
    }

    // Trim ending \n from result
    if(result.length() != 0) result = result.substr(0, result.length() - 1);

    // Set additional text bar text
    this->setAdditionalText(result);
}

CommandResult::CommandResult(bool successful, const std::string& message){
    this->successful = successful;
    this->message = message;
}

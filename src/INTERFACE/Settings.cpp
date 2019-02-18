
#include <fstream>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef _WIN32
   #include <io.h> 
   #define access    _access_s
#else
   #include <unistd.h>
#endif

#include "INTERFACE/Alert.h"
#include "INTERFACE/Settings.h"
#include "UTIL/nlohmann_json.hpp"

void HiddenState::defaults(){
    delta = 1.0;
    fastForward = false;
}

void invalidSettingValueType(const std::string& name, const std::string& expectedType){
    alertError(("Invalid value type for setting '" + name + "'!\n\nExpected type '" + expectedType + "'").c_str(), "Invalid settings.json file");
}

void unknownSetting(const std::string& name){
    alertError(("Unknown setting '" + name + "'").c_str(),  "Invalid settings.json file");
}

bool setting(nlohmann::json::iterator it, const std::string& name, bool *out){
    // Returns whether or not key was found
    if(it.key() != name) return false;

    if(it.value().is_boolean()){
        *out = it.value();
    } else {
        invalidSettingValueType(name, "boolean");
    }

    return true;
}

bool setting(nlohmann::json::iterator it, const std::string& name, int *out){
    // Returns whether or not key was found
    if(it.key() != name) return false;

    if(it.value().is_number_integer()){
        *out = it.value();
    } else {
        invalidSettingValueType(name, "integer");
    }

    return true;
}

bool setting(nlohmann::json::iterator it, const std::string& name, double *out){
    // Returns whether or not key was found
    if(it.key() != name) return false;

    if(it.value().is_number_float()){
        *out = it.value();
    } else {
        invalidSettingValueType(name, "number");
    }

    return true;
}

bool setting(nlohmann::json::iterator it, const std::string& name, std::string *out){
    // Returns whether or not key was found
    if(it.key() != name) return false;

    if(it.value().is_string()){
        *out = it.value();
    } else {
        invalidSettingValueType(name, "string");
    }

    return true;
}

bool setting(nlohmann::json::iterator it, const std::string& name, size_t *out){
    // Returns whether or not key was found
    if(it.key() != name) return false;

    if(it.value().is_number_integer()){
        *out = it.value();
    } else {
        invalidSettingValueType(name, "integer");
    }

    return true;
}

bool setting(nlohmann::json::iterator it, const std::string& name, std::vector<std::string> *out){
    out->clear();

    // Returns whether or not key was found
    if(it.key() != name) return false;

    if(!it.value().is_array()){
        invalidSettingValueType(name, "array of strings");
        return false;
    }

    for(size_t i = 0; i != it.value().size(); i++){
       if(it.value()[i].is_string()){
           out->push_back(it.value()[i].get<std::string>());
       } else {
           invalidSettingValueType(name, "array of strings");
       }
    }

    return true;
}

SyntaxColorPalette::Defaults themeFromString(std::string text){
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);

    if(text == "visual studio")   return SyntaxColorPalette::Defaults::VISUAL_STUDIO;
    if(text == "tropical ocean")  return SyntaxColorPalette::Defaults::TROPICAL_OCEAN;
    if(text == "island campfire") return SyntaxColorPalette::Defaults::ISLAND_CAMPFIRE;
    if(text == "one dark")        return SyntaxColorPalette::Defaults::ONE_DARK;
    if(text == "fruit smoothie")  return SyntaxColorPalette::Defaults::FRUIT_SMOOTHIE;
    return SyntaxColorPalette::Defaults::VISUAL_STUDIO;
}

FileType languageFromString(std::string text){
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);

    if(text == "adept")         return FileType::ADEPT;
    if(text == "java")          return FileType::JAVA;
    if(text == "html")          return FileType::HTML;

    if(text == /*prefered*/ "plain text" || text == "plain-text" || text == "plaintext" || text == "plain" || text == "text")
        return FileType::PLAIN_TEXT;
    
    return FileType::PLAIN_TEXT;
}

void Settings::defaults(){
    // ide.*
    this->ide_default_maximized = true;
    this->ide_default_width = 1024;
    this->ide_default_height = 720;
    this->ide_default_fps = 60;
    this->ide_suggestions = true;
    this->ide_debug_fps = false;
    this->ide_emblem = true;
    this->ide_scroll_fixed = true;
    this->ide_scroll_multiplier = 1.0;

    // editor.*
    this->editor_default_theme = SyntaxColorPalette::Defaults::VISUAL_STUDIO;
    this->editor_default_language = FileType::ADEPT;
    this->editor_default_text = "";
    this->editor_default_position = 0;
    this->editor_icons = true;
    this->editor_caret_animation = true;

    // adept.*
    this->adept_root = "C:\\Adept\\2.0\\";
    this->adept_compiler = "adept";

    this->explorer_default_show = true;
    this->explorer_default_collapse = true;
    this->explorer_default_folder = "";
    this->explorer_show_hidden = false;
    this->explorer_show_icons = true;
    this->explorer_show_folders = true;
    this->explorer_show_files = true;
    this->explorer_prefer_folders = true;
    this->explorer_prefer_code = true;

    this->terminal_show = false;
    this->terminal_transparent = false;
    #if _WIN32
    this->terminal_shell = "cmd.exe";
    #else
    this->terminal_shell = "/bin/bash";
    #endif
    this->terminal_shell_arguments.clear();
    this->terminal_environment_term = "xterm-256color";

    // Hidden values accessable via Settings
    this->hidden.defaults();
}

void Settings::loadFromFile(const std::string& filename){
    this->defaults();

    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    if(access(filename.c_str(), 0) != 0){
        return;
    }

    try {
        nlohmann::json settings = nlohmann::json::parse(text);
        std::string tmp;

        if(settings.is_object()){
            for(auto it = settings.begin(); it != settings.end(); it++){
                if(setting(it, "ide.default.maximized", &this->ide_default_maximized)) continue;
                if(setting(it, "ide.default.width", &this->ide_default_width)) continue;
                if(setting(it, "ide.default.height", &this->ide_default_height)) continue;
                if(setting(it, "ide.default.fps", &this->ide_default_fps)){
                    glfwSwapInterval(this->ide_default_fps <= 0 ? 0 : 60 / this->ide_default_fps);
                    continue;
                }
                if(setting(it, "ide.quicktype", &this->ide_quicktype)) continue;
                if(setting(it, "ide.suggestions", &this->ide_suggestions)) continue;
                if(setting(it, "ide.debug.fps", &this->ide_debug_fps)) continue;
                if(setting(it, "ide.emblem", &this->ide_emblem)) continue;
                if(setting(it, "ide.scroll.fixed", &this->ide_scroll_fixed)) continue;
                if(setting(it, "ide.scroll.multiplier", &this->ide_scroll_multiplier)) continue;

                if(setting(it, "editor.default.theme", &tmp)){
                    this->editor_default_theme = themeFromString(tmp);
                    continue;
                }

                if(setting(it, "editor.default.language", &tmp)){
                    this->editor_default_language = languageFromString(tmp);
                    continue;
                }

                if(setting(it, "editor.default.text", &this->editor_default_text)) continue;
                if(setting(it, "editor.default.position", &this->editor_default_position)) continue;
                if(setting(it, "editor.icons", &this->editor_icons)) continue;
                if(setting(it, "editor.caret.animation", &this->editor_caret_animation)) continue;

                if(setting(it, "adept.root", &this->adept_root)) continue;
                if(setting(it, "adept.compiler", &this->adept_compiler)) continue;

                if(setting(it, "explorer.default.show", &this->explorer_default_show)) continue;
                if(setting(it, "explorer.default.collapse", &this->explorer_default_collapse)) continue;
                if(setting(it, "explorer.default.folder", &this->explorer_default_folder)) continue;
                if(setting(it, "explorer.show.hidden", &this->explorer_show_hidden)) continue;
                if(setting(it, "explorer.show.icons", &this->explorer_show_icons)) continue;
                if(setting(it, "explorer.show.folders", &this->explorer_show_folders)) continue;
                if(setting(it, "explorer.show.files", &this->explorer_show_files)) continue;
                if(setting(it, "explorer.prefer.folders", &this->explorer_prefer_folders)) continue;
                if(setting(it, "explorer.prefer.code", &this->explorer_prefer_code)) continue;

                if(setting(it, "terminal.show", &this->terminal_show)) continue;
                if(setting(it, "terminal.transparent", &this->terminal_transparent)) continue;
                if(setting(it, "terminal.shell", &this->terminal_shell)) continue;
                if(setting(it, "terminal.shell.arguments", &this->terminal_shell_arguments)) continue;
                if(setting(it, "terminal.environment.term", &this->terminal_environment_term)) continue;

                unknownSetting(it.key());
            }
        } else {
            alertError("Expected settings.json to be an object", "Invalid settings.json file");
        }
    } catch(...){
        alertError("Failed to parse settings.json file", "Invalid settings.json file");
    }
}

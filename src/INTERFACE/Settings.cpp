
#include <fstream>
#include <sstream>
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
    // Returns whether or not key was found
    if(it.key() != name) return false;

    if(!it.value().is_array()){
        invalidSettingValueType(name, "array of strings");
        return false;
    }

    out->clear();
    
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
    if(text == "json")          return FileType::JSON;
    
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
    this->ide_quicktype = true;
    this->ide_suggestions = true;
    this->ide_suggestions_max = 5;
    this->ide_debug_fps = false;
    this->ide_emblem = true;
    this->ide_scroll_fixed = true;
    this->ide_scroll_multiplier = 1.0;
    this->ide_error_underline = true;
    this->ide_warning_underline = true;

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

    this->insight_passive_rate = 1.0;

    // Hidden values accessable via Settings
    this->hidden.defaults();
}

bool objectIntoSettings(Settings *settings, nlohmann::json object){
    if(!object.is_object()) return false;

    std::string tmp;

    for(auto it = object.begin(); it != object.end(); it++){
        if(setting(it, "ide.default.maximized", &settings->ide_default_maximized)) continue;
        if(setting(it, "ide.default.width", &settings->ide_default_width)) continue;
        if(setting(it, "ide.default.height", &settings->ide_default_height)) continue;
        if(setting(it, "ide.default.fps", &settings->ide_default_fps)){
            glfwSwapInterval(settings->ide_default_fps <= 0 ? 0 : 60 / settings->ide_default_fps);
            continue;
        }
        if(setting(it, "ide.quicktype", &settings->ide_quicktype)) continue;
        if(setting(it, "ide.suggestions", &settings->ide_suggestions)) continue;
        if(setting(it, "ide.suggestions.max", &settings->ide_suggestions_max)) continue;
        if(setting(it, "ide.debug.fps", &settings->ide_debug_fps)) continue;
        if(setting(it, "ide.emblem", &settings->ide_emblem)) continue;
        if(setting(it, "ide.scroll.fixed", &settings->ide_scroll_fixed)) continue;
        if(setting(it, "ide.scroll.multiplier", &settings->ide_scroll_multiplier)) continue;
        if(setting(it, "ide.error.underline", &settings->ide_error_underline)) continue;
        if(setting(it, "ide.warning.underline", &settings->ide_warning_underline)) continue;

        if(setting(it, "editor.default.theme", &tmp)){
            settings->editor_default_theme = themeFromString(tmp);
            continue;
        }

        if(setting(it, "editor.default.language", &tmp)){
            settings->editor_default_language = languageFromString(tmp);
            continue;
        }

        if(setting(it, "editor.default.text", &settings->editor_default_text)) continue;
        if(setting(it, "editor.default.position", &settings->editor_default_position)) continue;
        if(setting(it, "editor.icons", &settings->editor_icons)) continue;
        if(setting(it, "editor.caret.animation", &settings->editor_caret_animation)) continue;

        if(setting(it, "adept.root", &settings->adept_root)) continue;
        if(setting(it, "adept.compiler", &settings->adept_compiler)) continue;

        if(setting(it, "explorer.default.show", &settings->explorer_default_show)) continue;
        if(setting(it, "explorer.default.collapse", &settings->explorer_default_collapse)) continue;
        if(setting(it, "explorer.default.folder", &settings->explorer_default_folder)) continue;
        if(setting(it, "explorer.show.hidden", &settings->explorer_show_hidden)) continue;
        if(setting(it, "explorer.show.icons", &settings->explorer_show_icons)) continue;
        if(setting(it, "explorer.show.folders", &settings->explorer_show_folders)) continue;
        if(setting(it, "explorer.show.files", &settings->explorer_show_files)) continue;
        if(setting(it, "explorer.prefer.folders", &settings->explorer_prefer_folders)) continue;
        if(setting(it, "explorer.prefer.code", &settings->explorer_prefer_code)) continue;

        if(setting(it, "terminal.show", &settings->terminal_show)) continue;
        if(setting(it, "terminal.transparent", &settings->terminal_transparent)) continue;
        if(setting(it, "terminal.shell", &settings->terminal_shell)) continue;
        if(setting(it, "terminal.shell.arguments", &settings->terminal_shell_arguments)) continue;
        if(setting(it, "terminal.environment.term", &settings->terminal_environment_term)) continue;

        if(setting(it, "insight.passive.rate", &settings->insight_passive_rate)) continue;

        if(it.key() == "windows"){
            #ifdef _WIN32
            objectIntoSettings(settings, it.value());
            #endif
            continue;
        }

        if(it.key() == "macos"){
            #ifdef __APPLE__
            objectIntoSettings(settings, it.value());
            #endif
            continue;
        }
        
        if(it.key() == "linux"){
            #ifdef __linux__
            objectIntoSettings(settings, it.value());
            #endif
            continue;
        }

        if(it.key() == "unix"){
            #if defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__))
            objectIntoSettings(settings, it.value());
            #endif
            continue;
        }

        unknownSetting(it.key());
    }

    // Auto append '/' or '\\' to 'adept.root' if missing
    // TODO: Cleanup this ugly code
    {
        size_t len = settings->adept_root.length();
        if(len != 0){
            char c = settings->adept_root[len - 1];
            if(c != '/' && c != '\\'){
                #if _WIN32
                settings->adept_root += "\\";
                #else
                settings->adept_root += "/";
                #endif
            }
        }
    }
    
    return true;
}

void Settings::loadFromFile(const std::string& filename){
    this->defaults();

    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string text = buffer.str();

    if(access(filename.c_str(), 0) != 0) return;

    try {
        if(!objectIntoSettings(this, nlohmann::json::parse(text))){
            alertError("Expected settings to be an object", "Invalid settings file");
        }
    } catch(...){
        alertError("Failed to parse settings.json file", "Invalid settings file");
    }
}

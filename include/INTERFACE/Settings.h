
#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>

#include "INTERFACE/FileType.h"
#include "INTERFACE/SyntaxColorPalette.h"

struct HiddenState {
    double delta;
    bool fastForward;

    void defaults();
};

class Settings {
private:
    void defaults();

public:
    // ide.*
    bool ide_default_maximized;
    int ide_default_width;
    int ide_default_height;
    int ide_default_fps;
    bool ide_quicktype;
    bool ide_suggestions;
    size_t ide_suggestions_max;
    bool ide_debug_fps;
    bool ide_emblem;
    bool ide_scroll_fixed;
    double ide_scroll_multiplier;
    bool ide_error_underline;
    bool ide_warning_underline;

    // editor.*
    SyntaxColorPalette::Defaults editor_default_theme;
    FileType editor_default_language;
    std::string editor_default_text;
    size_t editor_default_position;
    bool editor_icons;
    bool editor_caret_animation;

    // adept.*
    std::string adept_root;
    std::string adept_compiler;

    // explorer.*
    bool explorer_default_show;
    bool explorer_default_collapse;
    std::string explorer_default_folder;
    bool explorer_show_hidden;
    bool explorer_show_icons;
    bool explorer_show_folders;
    bool explorer_show_files;
    bool explorer_prefer_folders;
    bool explorer_prefer_code;

    // terminal.*
    bool terminal_show;
    bool terminal_transparent;
    std::string terminal_shell;
    std::vector<std::string> terminal_shell_arguments;
    std::string terminal_environment_term;

    // insight.*
    double insight_passive_rate;

    // Hidden values accessable via Settings
    HiddenState hidden;

    void loadFromFile(const std::string& filename);
};

#endif // SETTINGS_H
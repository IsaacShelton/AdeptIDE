
#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

#include "INTERFACE/FileType.h"
#include "INTERFACE/SyntaxColorPalette.h"

struct HiddenState {
    double delta;

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
    bool ide_suggestions;
    bool ide_debug_fps;
    bool ide_emblem;

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

    // Hidden values accessable via Settings
    HiddenState hidden;

    void loadFromFile(const std::string& filename);
};

#endif // SETTINGS_H
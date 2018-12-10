
#ifndef SYNTAX_COLOR_PALETTE_H
#define SYNTAX_COLOR_PALETTE_H

#include "OPENGL/Vector3f.h"

class SyntaxColorPalette {
    public:
    enum Defaults {
        FRUIT_SMOOTHIE, VISUAL_STUDIO, TROPICAL_OCEAN, ISLAND_CAMPFIRE, ONE_DARK
    };

    Vector3f plain;
    Vector3f keyword;
    Vector3f comment;
    Vector3f type;
    Vector3f string;
    Vector3f number;
    Vector3f operation;
    Vector3f compile_time;

    SyntaxColorPalette();
    SyntaxColorPalette(SyntaxColorPalette::Defaults defaults);
    void generate(SyntaxColorPalette::Defaults defaults);
};

#endif // SYNTAX_COLOR_PALETTE_H
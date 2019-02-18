
#ifndef FONT_H_INCLUDED
#define FONT_H_INCLUDED

#include <string>
#include <vector>
#include "OPENGL/Model.h"
#include "OPENGL/Vector3f.h"
#include "INTERFACE/SyntaxColorPalette.h"

// For InconsolataGo
#define FONT_NAME "inconsolatago"
#define FONT_SCALE 0.17

// For Courier New
//#define FONT_NAME "courier-new"
//#define FONT_SCALE 0.20

class FontCharacter {
    // MEMORY: Using doubles here might be a bit overkill (No pun in ten did)

    public:
    int id;
    double u;
    double v;
    double u_max;
    double v_max;
    double x_offset;
    double y_offset;
    double size_x;
    double size_y;
    double advance;

    FontCharacter(int, double, double, double, double, double, double, double, double, double);
};

class AttributeEntry {
    public:
    struct SubAttribute {
        std::string name;
        std::string value;
    };

    std::string name;
    std::vector<SubAttribute> sub_attributes;
    AttributeEntry::SubAttribute* getSubAttribute(const std::string&);
};

class Font {
    public:
    float width;
    float height;
    int line_height;
    double mono_character_width;
    std::vector<AttributeEntry> attributes;
    std::vector<FontCharacter> characters;

    void load(const std::string& filename);
    void loadData(const std::string& filename);
    void generateCharacters();
    AttributeEntry* getAttribute(const std::string& name);
    TextModel generatePlainTextModel(const std::string& text, float scale, Vector3f color = Vector3f(0.83, 0.83, 0.83));

    TextModel generateAdeptTextModel(const std::string& text, float scale, const SyntaxColorPalette& palette);
    TextModel generateJavaTextModel(const std::string& text, float scale, const SyntaxColorPalette& palette);
    TextModel generateHtmlTextModel(const std::string& text, float scale, const SyntaxColorPalette& palette);

    void highlightCharacters(const std::string&, size_t&, size_t, const Vector3f&,
                             std::vector<unsigned int>&, std::vector<float>&, std::vector<float>&, std::vector<float>&, size_t&, float&, float&, float);
    void print();
};

#endif // FONT_H_INCLUDED

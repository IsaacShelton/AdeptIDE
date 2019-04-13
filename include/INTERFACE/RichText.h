
#ifndef RICHTEXT_H
#define RICHTEXT_H

#include "INTERFACE/Font.h"
#include "INTERFACE/FileType.h"
#include "INTERFACE/SyntaxColorPalette.h"
#include "OPENGL/Vector3f.h"

enum HighlightState {
    PLAIN, MULTILINE_COMMENT, STRING, CSTRING
};

class RichText {
    float    *vertices = NULL;
    float    *uvs      = NULL;
    float    *colors   = NULL;
    unsigned *indices  = NULL;
    size_t length = 0, capacity = 0; // (in characters)

    TextModel model;
    bool has = false, updated = true;

    Font *font = NULL;
    std::vector<HighlightState> forwardLineState;
    SyntaxColorPalette palette = SyntaxColorPalette::VISUAL_STUDIO;

    void insertCharacterData(size_t index, char character);
    void removeCharacterData(size_t index);
    void grow(size_t additionalCharacters);

    enum Relation {WHITESPACE, WHITESPACE_OR_OPERATOR};

    struct Special {
        std::string name;
        Relation prereleation, postrelation;
        const Vector3f *color;
    };

    std::vector<Special> adeptSpecials;
    std::vector<Special> javaSpecials;

public:
    // NOTE: 'text' shouldn't be modified directly when possible
    std::string text;
    FileType fileType = FileType::ADEPT;

    RichText();
    ~RichText();
    void setFont(Font *font);
    void loadFromFile(const std::string& filename);
    void insert(size_t index, char character);
    void insert(size_t index, const std::string& characters);
    void remove(size_t index, size_t amount);
    void changeColor(size_t beginning, size_t end, Vector3f color);
    void generate();
    TextModel* getTextModel();
    void setFileType(const FileType& fileType);
    void setSyntaxColorPalette(const SyntaxColorPalette& palette);
    SyntaxColorPalette getSyntaxColorPalette();
    void updateSpecials();

    void forceHighlightEverything();
    size_t highlightLine(size_t lineBeginning, size_t line);
    void highlightAffectedLines(size_t lineBeginning);
    size_t tryHighlightKeyword(size_t index);
    void highlightNumber(size_t& i);
    void setCharacterColor(size_t index, const Vector3f& color);
    void setCharacterColor(size_t index, float r, float g, float b);
    void setStringColor(size_t index, size_t length, const Vector3f& color);
    void setStringColor(size_t index, size_t length, float r, float g, float b);
};

#endif // RICHTEXT_H


#ifndef CARET_H
#define CARET_H

#include "INTERFACE/Font.h"
#include "INTERFACE/Settings.h"
#include "OPENGL/Model.h"
#include "OPENGL/Matrix4f.h"

class Caret {
    TextModel *caretModel;
    size_t caretPosition;
    float characterWidth;
    float lineHeight;
    Settings *settings;
    Font *font;
    float x;
    float y;
public:

    static void getTargetCoords(size_t position, const std::string& text, Font& font, float xOffset, float yOffset, float *out_targetX, float *out_targetY);

    Caret();
    ~Caret();

    void set(size_t amount);
    void increase(size_t amount);
    void decrease(size_t amount);
    void moveLeft(const std::string &text);
    void moveRight(const std::string &text);
    void moveUp(const std::string &text);
    void moveDown(const std::string &text);
    void moveBeginningOfLine(const std::string &text);
    void moveEndOfLine(const std::string &text);
    void moveBeginningOfWord(const std::string &text);
    void moveEndOfWord(const std::string &text);
    void moveBeginningOfSubWord(const std::string& text);
    void moveEndOfSubWord(const std::string& text);
    void moveOutside(const std::string& text);
    void moveTo(const std::string& text, int row, int column);
    void snapToTargetPosition(const std::string& text, float xOffset, float yOffset);

    void generate(Settings *settings, Font *font);
    void draw();

    size_t getLineBeginning(const std::string &text);
    size_t getLineEnd(const std::string &text);
    size_t getLineEndAfterNewline(const std::string &text);
    size_t getPositionInLine(const std::string& text);
    size_t getNextLineBeginning(const std::string &text);
    size_t getPreviousLineBeginning(const std::string &text);
    size_t getBeforeWhitespace(const std::string& text, size_t position);
    size_t getAfterWhitespace(const std::string& text, size_t position);
    size_t getAfterWhitespaceInLine(const std::string& text, size_t position);
    size_t getBeginningOfWord(const std::string& text);
    size_t getEndOfWord(const std::string& text);
    size_t getBeginningOfSubWord(const std::string& text);
    size_t getEndOfSubWord(const std::string& text);
    size_t getOutside(const std::string& text);
    size_t getLine(const std::string& text);

    void getTransformationMatrix(const std::string& text, float xOffset, float yOffset, Matrix4f *matrix);
    size_t getPosition();
    size_t countLeadingWhitespace(const std::string &text);
    float getX();
    float getY();
};

#endif // CARET_H
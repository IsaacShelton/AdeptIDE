
#include <math.h>
#include <ctype.h>

#include "UTIL/lexical.h"
#include "UTIL/document.h"
#include "INTERFACE/Caret.h"

Caret::Caret(){
    this->caretModel = NULL;
    this->x = 0.0f;
    this->y = 0.0f;
}

Caret::~Caret(){
    delete this->caretModel;
}

void Caret::set(size_t position){
    this->caretPosition = position;
}

void Caret::increase(size_t amount){
    this->caretPosition += amount;
}

void Caret::decrease(size_t amount){
    this->caretPosition -= amount;
}

void Caret::generate(Settings *settings, Font *font){
    this->settings = settings;
    this->caretModel = new TextModel(font->generatePlainTextModel("|", 0.17f));
    this->characterWidth = font->mono_character_width * 0.17f;
    this->lineHeight = font->line_height * 0.17f;
    this->font = font;
}

void Caret::draw(){
    if(this->caretModel) this->caretModel->draw();
}

#include <iostream>

void Caret::getTransformationMatrix(const std::string& text, float xOffset, float yOffset, Matrix4f *matrix){
    float targetX, targetY;
    this->getTargetCoords(this->caretPosition, text, *this->font, xOffset, yOffset, &targetX, &targetY);

    if(settings->editor_caret_animation){

        // Expression used for scaling decay movement by delta
        // (where delta is 1.0 for expected frame rate)
        // relativeDistanceChange = delta / 2

        if(fabs(this->x - targetX) > 0.01f){
            this->x += (targetX > this->x ? 1 : -1) * fabs(this->x - targetX) * (this->settings->hidden.delta / 2);
        } else this->x = targetX;

        if(fabs(this->y - targetY) > 0.01f){
            this->y += (targetY > this->y ? 1 : -1) * fabs(this->y - targetY) * (this->settings->hidden.delta / 2);
        } else this->y = targetY;
    } else {
        this->x = targetX;
        this->y = targetY;
    }

    matrix->translateFromIdentity(this->x, this->y, 0.1f);
}

void Caret::getTargetCoords(size_t position, const std::string& text, Font& font, float xOffset, float yOffset, float *out_targetX, float *out_targetY){
    float textX = 0.0f, textY = 0.0f;
    size_t finalLineIndex = 0;
    size_t lineCount = 1;
    size_t beforeCount = 0;

    float characterWidth = font.mono_character_width * 0.17f;
    float lineHeight = font.line_height * 0.17f;

    for(size_t i = 0; i != position; i++){
        if(text[i] == '\n'){
            lineCount += 1;
            finalLineIndex = i + 1;
        }
    }

    while(finalLineIndex + beforeCount + 1 <= position){
        beforeCount += 1;
    }

    textX = (beforeCount * characterWidth);
    textY = (lineCount - 1) * lineHeight;

    *out_targetX = textX + xOffset;
    *out_targetY = textY + yOffset;
}

size_t Caret::getPosition(){
    return this->caretPosition;
}

size_t Caret::countLeadingWhitespace(const std::string& text){
    size_t lineBeginning = this->getLineBeginning(text);
    return this->getAfterWhitespaceInLine(text, lineBeginning) - lineBeginning;
}

float Caret::getX(){
    return this->x;
}

float Caret::getY(){
    return this->y;
}

void Caret::moveLeft(const std::string &text){
    if(this->caretPosition == 0) return;

    this->caretPosition -= 1;
}

void Caret::moveRight(const std::string &text){
    if(this->caretPosition == text.length()) return;

    this->caretPosition += 1;
}

void Caret::moveUp(const std::string &text){
    size_t distanceToBeginning = this->caretPosition - this->getLineBeginning(text);
    size_t previousLineBeginning = this->getPreviousLineBeginning(text);
    size_t previousLineLength = getLineLength(text, previousLineBeginning);
    size_t forceDistance = distanceToBeginning <= previousLineLength ? distanceToBeginning : previousLineLength;
    this->caretPosition = previousLineBeginning + forceDistance;
}

void Caret::moveDown(const std::string &text){
    size_t distanceToBeginning = this->caretPosition - this->getLineBeginning(text);
    size_t nextLineBeginning = this->getNextLineBeginning(text);
    size_t nextLineLength = getLineLength(text, nextLineBeginning);
    size_t forceDistance = distanceToBeginning <= nextLineLength ? distanceToBeginning : nextLineLength;
    this->caretPosition = nextLineBeginning + forceDistance;
}

void Caret::moveBeginningOfLine(const std::string &text){
    size_t trueBeginning = this->getLineBeginning(text);
    size_t homeBeginning = this->getAfterWhitespaceInLine(text, trueBeginning);

    if(this->caretPosition != homeBeginning){
        this->caretPosition = homeBeginning;
    } else {
        this->caretPosition = trueBeginning;
    }
}

void Caret::moveEndOfLine(const std::string &text){
    size_t beginning = this->getLineBeginning(text);
    size_t length = getLineLength(text, beginning);
    this->caretPosition = beginning + length;
}

void Caret::moveBeginningOfWord(const std::string& text){
    this->caretPosition = this->getBeginningOfWord(text);
}

void Caret::moveEndOfWord(const std::string& text){
    this->caretPosition = this->getEndOfWord(text);
}

void Caret::moveBeginningOfSubWord(const std::string& text){
    this->caretPosition = this->getBeginningOfSubWord(text);
}

void Caret::moveEndOfSubWord(const std::string& text){
    this->caretPosition = this->getEndOfSubWord(text);
}

void Caret::moveOutside(const std::string& text){
    this->caretPosition = this->getOutside(text);
}

void Caret::moveTo(const std::string& text, int row, int column){
    if(row < 1)    row = 1;    
    if(column < 1) column = 1;

    size_t i = 0;
    int current_row = 1;
    int current_column = 1;

    while(current_row != row && i != text.length()){
        if(text[i] == '\n') current_row++;
        i++;
    }

    while(current_column != column && i != text.length() && text[i] != '\n'){
        current_column++;
        i++;
    }

    this->caretPosition = i;
}

void Caret::snapToTargetPosition(const std::string& text, float xOffset, float yOffset){
    float targetX, targetY;
    this->getTargetCoords(this->caretPosition, text, *this->font, xOffset, yOffset, &targetX, &targetY);
    this->x = targetX;
    this->y = targetY;
}

size_t Caret::getPositionInLine(const std::string& text){
    return this->caretPosition - this->getLineBeginning(text);
}

size_t Caret::getLineBeginning(const std::string &text){
    size_t i = this->caretPosition;

    while(i != 0 && text[i - 1] != '\n'){
        i--;
    }

    return i;
}

size_t Caret::getLineEnd(const std::string& text){
    return ::getLineEnd(text, this->caretPosition);
}

size_t Caret::getLineEndAfterNewline(const std::string& text){
    size_t i = this->caretPosition;

    while(i < text.length()){
        if(text[i] == '\n') return i + 1;
        i++;
    }

    return i;
}

size_t Caret::getNextLineBeginning(const std::string &text){
    size_t i = this->caretPosition + 1;

    while(i <= text.length()){
        if(text[i - 1] == '\n') return i;
        i++;
    }

    return this->caretPosition;
}

size_t Caret::getPreviousLineBeginning(const std::string &text){
    size_t i = this->caretPosition;
    bool passedOverNewline = false;

    while(i != 0){
        if(text[i - 1] == '\n'){
            if(passedOverNewline) break;
            else passedOverNewline = true;
        }
        i--;
    }

    return i;
}

size_t Caret::getBeforeWhitespace(const std::string& text, size_t position){
    size_t i = position;

    while(i > 0 && isWhitespace(text[i - 1])) i--;

    return i;
}

size_t Caret::getAfterWhitespace(const std::string& text, size_t position){
    size_t i = position;

    while(i < text.length() && isWhitespace(text[i])) i++;

    return i;
}

size_t Caret::getAfterWhitespaceInLine(const std::string& text, size_t position){
    size_t i = position;

    while(i < text.length() && isWhitespace(text[i]) && text[i] != '\n') i++;

    return i;
}

size_t Caret::getBeginningOfWord(const std::string& text){
    size_t i = this->caretPosition;
    if(i == 0) return 0;

    if(isWhitespace(text[i - 1])){
        return this->getBeforeWhitespace(text, i);
    }

    if(isOperator(text[i - 1])) return i - 1;

    while(i > 0 && (
        !isWhitespace(text[i - 1]) && !isOperator(text[i - 1])
    )) i--;

    return i;
}

size_t Caret::getEndOfWord(const std::string& text){
    size_t i = this->caretPosition;
    if(text.length() == 0) return 0;

    if(isWhitespace(text[i])){
        return this->getAfterWhitespace(text, i);
    }

    if(isOperator(text[i])) return i + 1;

    while(i < text.length() && (
        !isWhitespace(text[i]) && !isOperator(text[i])
    )) i++;

    return i;
}

size_t Caret::getBeginningOfSubWord(const std::string& text){
    size_t i = this->caretPosition;
    if(i == 0) return 0;

    if(isWhitespace(text[i - 1])){
        return this->getBeforeWhitespace(text, i);
    }

    if(isOperator(text[i - 1])) return i - 1;

    while(i > 0 && text[i - 1] == '_') i--;

    while(i > 0 && (
        !isWhitespace(text[i - 1]) && !isOperator(text[i - 1]) && text[i - 1] != '_'
        && !isupper(text[i - 1])
    )) i--;

    if(isupper(text[i - 1]) && i > 0) i--;

    return i;
}

size_t Caret::getEndOfSubWord(const std::string& text){
    size_t i = this->caretPosition;
    if(text.length() == 0) return 0;

    if(isWhitespace(text[i])){
        return this->getAfterWhitespace(text, i);
    }

    if(isOperator(text[i])) return i + 1;

    while(i < text.length() && text[i] == '_') i++;

    if(i < text.length() && isupper(text[i])) i++;

    while(i < text.length() && (
        !isWhitespace(text[i]) && !isOperator(text[i]) && text[i] != '_'
        && !isupper(text[i])
    )) i++;

    return i;
}

size_t Caret::getOutside(const std::string& text){
    size_t i = this->caretPosition;
    if(text.length() == 0) return 0;

    while(i < text.length()){
        if(text[i] == ')' || text[i] == '}' || text[i] == ']' || text[i] == '\'' || text[i] == '"'){
            return i + 1;
        }

        i++;
    }

    return i;
}

size_t Caret::getLine(const std::string& text){
    return getLineNumber(text, this->caretPosition);
}

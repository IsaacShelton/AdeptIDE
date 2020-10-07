
#include <fstream>
#include <sstream>
#include <assert.h>
#include <string.h>

#include "UTIL/strings.h"
#include "UTIL/document.h"
#include "INTERFACE/Caret.h"
#include "INTERFACE/RichText.h"

RichText::RichText(){
    this->updateSpecials();
}

RichText::~RichText(){
    if(this->has) this->model.free();
    free(this->vertices);
    free(this->uvs);
    free(this->colors);
    free(this->indices);
}

void RichText::setFont(Font *font){
    this->font = font;
    this->forwardLineState.clear();
    this->forwardLineState.push_back(PLAIN); // Given state to line 1
    this->forwardLineState.push_back(PLAIN); // Forward state from line 1
    this->fileType = FileType::PLAIN_TEXT;
}
#include <iostream>
void RichText::loadFromFile(const std::string& filename){
    std::ifstream stream(filename);
    std::string everything;

    if(stream.is_open()){
        std::stringstream buffer;
        buffer << stream.rdbuf();
        everything = buffer.str();

        if(everything.length() > 100000){
            this->setFileType(PLAIN_TEXT);
            everything = "AdeptIDE Error: Failed to open file, file was too large...\n\nSorry about that :\\\n";
        } else {
            if(string_ends_with(filename, ".adept")){
                this->setFileType(FileType::ADEPT);
            } else if(string_ends_with(filename, ".java")){
                this->setFileType(FileType::JAVA);
            } else if(string_ends_with(filename, ".html")){
                this->setFileType(FileType::HTML);
            } else if(string_ends_with(filename, ".json")){
                this->setFileType(FileType::JSON);
            }

            everything = string_replace_all(everything, "\t", "    ");
        }
        
    } else {
        this->setFileType(PLAIN_TEXT);
        everything = "AdeptIDE Error: Failed to open file...\n\nSorry about that :\\\n";
    }

    this->length = 0;

    for(size_t i = 0; i != everything.length(); i++){
        this->insertCharacterData(this->text.length(), everything[i]);
    }
}

void RichText::insert(size_t index, char character){
    assert(this->font != NULL);

    this->insertCharacterData(index, character);
}

void RichText::insert(size_t index, const std::string& characters){
    assert(this->font != NULL);

    for(size_t i = 0; i != characters.length(); i++){
        this->insert(index + i, characters[i]);
    }
}

void RichText::remove(size_t index, size_t amount){
    assert(this->font != NULL);

    size_t toLine = 0;
    bool toPlain; // undef

    for(size_t i = 0; i != amount; i++){
        bool newline = (this->text[index] == '\n');
        this->removeCharacterData(index);

        if(newline){
            if(toLine == 0){
                toLine = getLineNumber(this->text, index);
                toPlain = (this->forwardLineState[toLine] == PLAIN);
            }

            bool fromPlain = (this->forwardLineState[toLine + 1] == PLAIN);
            this->forwardLineState.erase(this->forwardLineState.begin() + toLine + 1);

            if(!toPlain || !fromPlain){
                this->highlightAffectedLines(getLineBeginning(this->text, index));
            }
        }
    }
}

void RichText::generate(){
    if(has) model.free();
    model.load(vertices, this->length * 8, indices, this->length * 6, uvs, this->length * 8, colors, this->length * 12);
    updated = false;
    has = true;
}

TextModel* RichText::getTextModel(){
    if(updated) this->generate();
    return &this->model;
}

void RichText::setFileType(const FileType& fileType){
    if(this->fileType == fileType) return;

    this->fileType = fileType;
    this->forceHighlightEverything();
}

void RichText::setSyntaxColorPalette(const SyntaxColorPalette& palette){
    this->palette = palette;
    this->forceHighlightEverything();
    this->updateSpecials();    
}

SyntaxColorPalette RichText::getSyntaxColorPalette(){
    return this->palette;
}

void RichText::updateSpecials(){
    this->adeptSpecials = {
        {"as", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"at", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"if", WHITESPACE, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"in", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"or", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"it", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"for", WHITESPACE, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"and", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"new", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"int", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"ptr", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"out", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"idx", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"def", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"for", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"Any", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"POD", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"func", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"void", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"null", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"cast", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"uint", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"byte", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"long", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"bool", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"else", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"case", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"true", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"this", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"each", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"enum", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"using", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"const", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"while", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"until", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"defer", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"ubyte", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"ulong", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"short", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"float", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"usize", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"false", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"alias", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"break", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"undef", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"inout", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"union", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"define", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"struct", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"public", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"import", WHITESPACE, WHITESPACE, &palette.keyword},
        {"return", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"switch", WHITESPACE, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"delete", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"sizeof", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"unless", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"ushort", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"String", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"double", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"packed", WHITESPACE, WHITESPACE, &palette.keyword},
        {"pragma", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"repeat", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"static", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"va_arg", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"va_end", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"AnyType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"private", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"foreign", WHITESPACE, WHITESPACE, &palette.keyword},
        {"dynamic", WHITESPACE, WHITESPACE, &palette.keyword},
        {"default", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"stdcall", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"va_copy", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"va_list", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"constant", WHITESPACE, WHITESPACE, &palette.keyword},
        {"continue", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"external", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"implicit", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"typeinfo", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"va_start", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"verbatim", WHITESPACE, WHITESPACE, &palette.keyword},
        {"__types__", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.number},
        {"dangerous", WHITESPACE, WHITESPACE, &palette.keyword},
        {"AnyPtrType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"exhaustive", WHITESPACE_OR_OPERATOR, WHITESPACE, &palette.keyword},
        {"successful", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"AnyTypeKind", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},    
        {"namespace", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"fallthrough", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"thread_local", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"AnyStructType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"__type_kinds__", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.number},
        {"AnyFuncPtrType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"__types_length__", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.number},
        {"AnyFixedArrayType", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"__type_kinds_length__", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.number},

    };

    this->javaSpecials = {
        {"do", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"if", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"for", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"int", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"new", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"try", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"File", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"Math", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"byte", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"case", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"char", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"else", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"enum", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"goto", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"long", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"null", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"this", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"true", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"void", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"break", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"catch", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"class", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"const", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"false", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"final", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"float", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"short", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"super", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"while", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"Object", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"Random", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"System", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"String", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"assert", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"double", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"import", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"native", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"public", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"return", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"static", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"switch", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"throws", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"Console", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"Scanner", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"boolean", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"default", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"extends", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"finally", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"package", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"private", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"abstract", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"continue", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"strictfp", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"volatile", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"Exception", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"interface", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"protected", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"transient", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"BigDecimal", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"BigInteger", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"implements", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"instanceof", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"IOException", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"InputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"MathContext", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"EOFException", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"OutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"Serializable", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"StringBuffer", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"synchronized", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"StringBuilder", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"BufferedReader", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"BufferedWriter", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"FileDescriptor", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"CharArrayReader", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"CharArrayWriter", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"DataInputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"FileInputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"DataOutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"FileOutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"BufferedInputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"BufferedOutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"ByteArrayInputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
        {"ByteArrayOutputStream", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.type},
    };

    this->jsonSpecials = {
        {"true", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"false", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword},
        {"null", WHITESPACE_OR_OPERATOR, WHITESPACE_OR_OPERATOR, &palette.keyword}
    };
}

void RichText::insertCharacterData(size_t index, char character){
    float x, y;
    Caret::getTargetCoords(index, this->text, *this->font, 0.0f, 0.0f, &x, &y);

    this->text.insert(index, &character, 1);
    this->grow(1);

    for(size_t i = this->length; i != index; i--){
        memcpy(&this->vertices[i * 8], &this->vertices[i * 8 - 8], sizeof(float) * 8);
        memcpy(&this->uvs[i * 8], &this->uvs[i * 8 - 8], sizeof(float) * 8);
        memcpy(&this->colors[i * 12], &this->colors[i * 12 - 12], sizeof(float) * 12);

        this->indices[i * 6 - 6] += 4; this->indices[i * 6 - 5] += 4;
        this->indices[i * 6 - 4] += 4; this->indices[i * 6 - 3] += 4;
        this->indices[i * 6 - 2] += 4; this->indices[i * 6 - 1] += 4;
        memcpy(&this->indices[i * 6], &this->indices[i * 6 - 6], sizeof(unsigned int) * 6);
    }

    FontCharacter *fontCharacter = NULL;
    for(FontCharacter& testFontCharacter : font->characters){
        if(testFontCharacter.id == character){
            fontCharacter = &testFontCharacter;
            break;
        }
    }

    if(fontCharacter == NULL){
        this->vertices[index * 8    ] = 0.0f; this->vertices[index * 8 + 1] = 0.0f;
        this->vertices[index * 8 + 2] = 0.0f; this->vertices[index * 8 + 3] = 0.0f;
        this->vertices[index * 8 + 4] = 0.0f; this->vertices[index * 8 + 5] = 0.0f;
        this->vertices[index * 8 + 6] = 0.0f; this->vertices[index * 8 + 7] = 0.0f;

        this->uvs[index * 8    ] = 0.0f; this->uvs[index * 8 + 1] = 0.0f;
        this->uvs[index * 8 + 2] = 0.0f; this->uvs[index * 8 + 3] = 0.0f;
        this->uvs[index * 8 + 4] = 0.0f; this->uvs[index * 8 + 5] = 0.0f;
        this->uvs[index * 8 + 6] = 0.0f; this->uvs[index * 8 + 7] = 0.0f;
    } else {
        this->vertices[index * 8    ] = x + fontCharacter->x_offset * FONT_SCALE;
        this->vertices[index * 8 + 1] = y + fontCharacter->y_offset * FONT_SCALE;
        this->vertices[index * 8 + 2] = x + fontCharacter->x_offset * FONT_SCALE;
        this->vertices[index * 8 + 3] = y + fontCharacter->size_y * FONT_SCALE + fontCharacter->y_offset * FONT_SCALE;
        this->vertices[index * 8 + 4] = x + fontCharacter->size_x * FONT_SCALE + fontCharacter->x_offset * FONT_SCALE;
        this->vertices[index * 8 + 5] = y + fontCharacter->size_y * FONT_SCALE + fontCharacter->y_offset * FONT_SCALE;
        this->vertices[index * 8 + 6] = x + fontCharacter->size_x * FONT_SCALE + fontCharacter->x_offset * FONT_SCALE;
        this->vertices[index * 8 + 7] = y + fontCharacter->y_offset * FONT_SCALE;

        this->uvs[index * 8    ] = fontCharacter->u;     this->uvs[index * 8 + 1] = fontCharacter->v;
        this->uvs[index * 8 + 2] = fontCharacter->u;     this->uvs[index * 8 + 3] = fontCharacter->v_max;
        this->uvs[index * 8 + 4] = fontCharacter->u_max; this->uvs[index * 8 + 5] = fontCharacter->v_max;
        this->uvs[index * 8 + 6] = fontCharacter->u_max; this->uvs[index * 8 + 7] = fontCharacter->v;
    }

    this->colors[index * 12    ]  = 1.0f; this->colors[index * 12 + 1]  = 1.0f;
    this->colors[index * 12 + 2]  = 1.0f; this->colors[index * 12 + 3]  = 1.0f;
    this->colors[index * 12 + 4]  = 1.0f; this->colors[index * 12 + 5]  = 1.0f;
    this->colors[index * 12 + 6]  = 1.0f; this->colors[index * 12 + 7]  = 1.0f;
    this->colors[index * 12 + 8]  = 1.0f; this->colors[index * 12 + 9]  = 1.0f;
    this->colors[index * 12 + 10] = 1.0f; this->colors[index * 12 + 11] = 1.0f;

    this->indices[index * 6    ]  = index * 4;     this->indices[index * 6 + 1]  = index * 4 + 1;
    this->indices[index * 6 + 2]  = index * 4 + 2; this->indices[index * 6 + 3]  = index * 4 + 2;
    this->indices[index * 6 + 4]  = index * 4 + 3; this->indices[index * 6 + 5]  = index * 4;

    if(character == '\n'){
        size_t fromLine = getLineNumber(this->text, index);
        size_t lineBeginning = getLineBeginning(this->text, index);
        size_t lineBeginningLength = index - lineBeginning;

        this->forwardLineState.insert(this->forwardLineState.begin() + fromLine + 1, PLAIN);

        // Move everything after down a line
        for(size_t i = index + 1; i != this->text.length(); i++){
            this->vertices[i * 8 + 1] += font->line_height * FONT_SCALE;
            this->vertices[i * 8 + 3] += font->line_height * FONT_SCALE;
            this->vertices[i * 8 + 5] += font->line_height * FONT_SCALE;
            this->vertices[i * 8 + 7] += font->line_height * FONT_SCALE;
        }

        // Move existing text in newly created line back to start of line
        for(size_t i = index + 1; i != this->text.length() && this->text[i] != '\n'; i++){
            this->vertices[i * 8 + 0] -= lineBeginningLength * font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 2] -= lineBeginningLength * font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 4] -= lineBeginningLength * font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 6] -= lineBeginningLength * font->mono_character_width * FONT_SCALE;
        }
    } else {
        // Move all characters within the line that are after the new character, forward a character
        for(size_t i = index + 1; i != this->text.length() && this->text[i] != '\n'; i++){
            this->vertices[i * 8 + 0] += font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 2] += font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 4] += font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 6] += font->mono_character_width * FONT_SCALE;
        }

        this->highlightAffectedLines(getLineBeginning(this->text, index));
    }

    this->length++;
    this->updated = true;
}

void RichText::removeCharacterData(size_t index){
    bool is_newline = this->text[index] == '\n';
    
    memmove(&this->vertices[index * 8], &this->vertices[(index + 1) * 8], sizeof(float) * 8 * (this->length - index - 1));
    memmove(&this->uvs[index * 8], &this->uvs[(index + 1) * 8], sizeof(float) * 8 * (this->length - index - 1));
    memmove(&this->colors[index * 12], &this->colors[(index + 1) * 12], sizeof(float) * 12 * (this->length - index - 1));
    memmove(&this->indices[index * 6], &this->indices[(index + 1) * 6], sizeof(unsigned int) * 6 * (this->length - index - 1));
    this->text.erase(index, 1);

    for(size_t i = index; i != this->text.length(); i++){
        this->indices[i * 6 + 5] -= 4; this->indices[i * 6 + 2] -= 4;
        this->indices[i * 6 + 4] -= 4; this->indices[i * 6 + 1] -= 4;
        this->indices[i * 6 + 3] -= 4; this->indices[i * 6 + 0] -= 4;
    }

    if(is_newline){
        // Move line up
        for(size_t i = index; i != this->text.length(); i++){
            this->vertices[i * 8 + 1] -= font->line_height * FONT_SCALE;
            this->vertices[i * 8 + 3] -= font->line_height * FONT_SCALE;
            this->vertices[i * 8 + 5] -= font->line_height * FONT_SCALE;
            this->vertices[i * 8 + 7] -= font->line_height * FONT_SCALE;
        }

        // Move segment to end of line
        size_t segment_length = getLineLength(this->text, index);
        size_t existing_length = getCurrentLineLength(this->text, index) - segment_length;
        for(size_t i = index; i < index + segment_length; i++){
            this->vertices[i * 8 + 0] += existing_length * font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 2] += existing_length * font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 4] += existing_length * font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 6] += existing_length * font->mono_character_width * FONT_SCALE;
        }
    } else {
        // Move all characters within the line after the new character, back a character
        for(size_t i = index; i != this->text.length() && this->text[i] != '\n'; i++){
            this->vertices[i * 8 + 0] -= font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 2] -= font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 4] -= font->mono_character_width * FONT_SCALE;
            this->vertices[i * 8 + 6] -= font->mono_character_width * FONT_SCALE;
        }

        this->highlightAffectedLines(getLineBeginning(this->text, index));
    }

    this->length--;
    this->updated = true;
}

void RichText::grow(size_t additionalCharacters){
    while(length + additionalCharacters >= capacity){
        if(capacity == 0) capacity = 1024;
        else              capacity *= 2;

        vertices = static_cast<float*>(realloc(vertices, sizeof(float) * capacity * 8));
        uvs = static_cast<float*>(realloc(uvs, sizeof(float) * capacity * 8));
        colors = static_cast<float*>(realloc(colors, sizeof(float) * capacity * 12));
        indices = static_cast<unsigned int*>(realloc(indices, sizeof(unsigned int) * capacity * 6));
    }
}

void RichText::forceHighlightEverything(){
    size_t lineBeginning = 0;
    forwardLineState[0] = HighlightState::PLAIN;

    for(size_t line = 1; line != forwardLineState.size(); line++){
        highlightLine(lineBeginning, line);

        while(lineBeginning != text.length() && text[lineBeginning] != '\n'){
            lineBeginning++;
        }

        if(lineBeginning + 1 < text.length() && text[lineBeginning] == '\n'){
            lineBeginning++;
        }
    }

    this->updated = true;
}

size_t RichText::highlightLine(size_t lineBeginning, size_t line){
    // NOTE: Returns beginning of next line if it should be highlighted,
    // otherwise returns the given value for this line beginning

    HighlightState givenState = this->forwardLineState[line - 1];
    HighlightState state = givenState;

    for(size_t i = lineBeginning; i != this->text.length() && this->text[i] != '\n'; i++){
        if(this->fileType != PLAIN_TEXT) switch(state){
        case PLAIN: {
                char ascii = this->text[i];

                if(ascii == '"'){
                    this->setCharacterColor(i, palette.string);
                    state = STRING;
                    continue;
                }

                if(ascii == '\''){
                    this->setCharacterColor(i, palette.string);
                    state = CSTRING;
                    continue;
                }

                if((this->fileType == JAVA && ascii == '@') || (this->fileType == ADEPT && ascii == '#')){
                    size_t scan_i = i + 1;
                    while(scan_i < this->text.length() && (charIsIdentifier(this->text[scan_i]) || charIsNumeric(this->text[scan_i]))) scan_i++;
                    this->setStringColor(i, scan_i - i, palette.compile_time);
                    i = scan_i - 1;
                    continue;
                }

                if(this->fileType == ADEPT && ascii == '$'){
                    size_t scan_i = i + 1;
                    while(scan_i < this->text.length() && (charIsIdentifier(this->text[scan_i]) || charIsNumeric(this->text[scan_i]) || this->text[scan_i] == '~' || this->text[scan_i] == '#')) scan_i++;
                    this->setStringColor(i, scan_i - i, palette.type);
                    i = scan_i - 1;
                    continue;
                }

                if(i + 1 < this->text.length() && ascii == '/'){
                    if(this->text[i + 1] == '/'){
                        size_t remainingLength = getLineLength(this->text, lineBeginning) - (i - lineBeginning);
                        this->setStringColor(i, remainingLength, palette.comment);
                        goto hard_break;
                    } else if(this->text[i + 1] == '*'){
                        this->setCharacterColor(i, palette.comment);
                        this->setCharacterColor(++i, palette.comment);
                        state = MULTILINE_COMMENT;
                        continue;
                    }
                }

                char prevChar = i == 0 ? 0x00 : this->text[i - 1];
                
                if(charIsNumeric(ascii) and !(charIsNumeric(prevChar) or charIsIdentifier(prevChar))){
                    this->highlightNumber(i);
                    continue;
                }

                size_t maybeAfterKeyword = this->tryHighlightKeyword(i);
                if(maybeAfterKeyword != i){
                    i = maybeAfterKeyword - 1;
                    continue;
                }
            }
            break;
        case STRING: {
                this->setCharacterColor(i, palette.string);

                if(this->text[i] == '"'){
                    state = PLAIN;
                    continue;
                }

                if(i + 1 != this->text.size() && this->text[i] == '\\'){
                    this->setCharacterColor(++i, palette.string);
                    continue;
                }

                continue;
            }
        case CSTRING: {
                this->setCharacterColor(i, palette.string);

                if(this->text[i] == '\''){
                    state = PLAIN;

                    if(i + 2 < this->text.length() && this->text[i + 1] == 'u' && this->text[i + 2] == 'b'){
                        this->setCharacterColor(++i, palette.string);
                        this->setCharacterColor(++i, palette.string);
                    }
                    continue;
                }

                if(i + 1 != this->text.size() && this->text[i] == '\\'){
                    this->setCharacterColor(++i, palette.string);
                    continue;
                }

                continue;
            }
        case MULTILINE_COMMENT: {
                this->setCharacterColor(i, palette.comment);

                if(i + 1 < this->text.length() && this->text[i] == '*' && this->text[i + 1] == '/'){
                    this->setCharacterColor(++i, palette.comment);
                    state = PLAIN;
                    continue;
                }

                continue;
            }
        default:
            continue;
        }

        this->setCharacterColor(i, charIsOperator(text[i]) ? palette.operation : palette.plain);
    }

hard_break:
    if(this->forwardLineState[line] != state){
        this->forwardLineState[line] = state;
        
        size_t nextLineBeginning = lineBeginning;

        while(nextLineBeginning != this->text.length() && this->text[nextLineBeginning] != '\n'){
            nextLineBeginning++;
        }

        if(nextLineBeginning + 1 < this->text.length()){
            // Highlight next line
            return ++nextLineBeginning;
        }
    }

    // Don't highlight next line
    return lineBeginning;
}

void RichText::highlightAffectedLines(size_t lineBeginning){
    if(this->fileType == FileType::PLAIN_TEXT) return;

    size_t line = getLineNumber(this->text, lineBeginning);
    size_t nextLineBeginning = highlightLine(lineBeginning, line);

    while(nextLineBeginning != lineBeginning){
        lineBeginning = nextLineBeginning;
        nextLineBeginning = highlightLine(lineBeginning, ++line);
    }
}

size_t RichText::tryHighlightKeyword(size_t index){
    // NOTE: Must be sorted by name length
    Special *specials;
    size_t specials_length;

    if(this->fileType == FileType::ADEPT){
        specials = adeptSpecials.data();
        specials_length = adeptSpecials.size();
    } else if(this->fileType == FileType::JAVA){
        specials = javaSpecials.data();
        specials_length = javaSpecials.size();
    } else if(this->fileType == FileType::JSON){
        specials = jsonSpecials.data();
        specials_length = jsonSpecials.size();
    } else {
        return index;
    }

    size_t name_length_level = 0;
    bool previous_character_is_whitespace = index == 0 ? true : charIsWhitespace(text[index - 1]);
    bool previous_character_is_operator = previous_character_is_whitespace ? false : (index == 0 ? true : charIsOperator(text[index - 1]));
    bool next_character_is_whitespace = false, next_character_is_operator = false;

    for(size_t s = 0; s != specials_length; s++){
        Special *special = &specials[s];
        size_t name_length = special->name.length();

        if(name_length_level != name_length){
            // Update calculations
            if(index + name_length < text.size()){
                next_character_is_whitespace = charIsWhitespace(text[index + name_length]);
                next_character_is_operator = next_character_is_whitespace ? false : charIsOperator(text[index + name_length]);
            } else {
                // Pretend the next character is there
                next_character_is_operator = true;
                next_character_is_whitespace = true;
            }

            name_length_level = name_length;
        }

        // Faster substring compare than using std::string::substr
        if(strncmp(&text[index], special->name.data(), name_length) == 0){
            // Determine whether or not we should highlight it
            bool should_highlight = (special->prereleation == WHITESPACE) ? (
                (previous_character_is_whitespace)
                && ((special->postrelation == WHITESPACE) ? (
                    next_character_is_whitespace
                ) : (special->postrelation == WHITESPACE_OR_OPERATOR ? (
                    next_character_is_whitespace || next_character_is_operator
                ) : false))
            ) : (special->prereleation == WHITESPACE_OR_OPERATOR ? (
                (previous_character_is_whitespace || previous_character_is_operator)
                && ((special->postrelation == WHITESPACE) ? (
                    next_character_is_whitespace
                ) : (special->postrelation == WHITESPACE_OR_OPERATOR ? (
                    next_character_is_whitespace || next_character_is_operator
                ) : false))
            ) : false);

            if(should_highlight){
                if(special->name == "pragma"){
                    // Do special stuff for pragma
                    while(index + name_length < text.size() && charIsWhitespace(text[index + name_length])) name_length++;
                    while(index + name_length < text.size() && charIsIdentifier(text[index + name_length])) name_length++;
                }

                this->setStringColor(index, name_length, *special->color);
                return index + name_length;
            }
        }
    }

    return index;
}

void RichText::highlightNumber(size_t& i){
    size_t scan_i = i + 1;

    if(scan_i != text.size()){
        if(text[scan_i] == 'x'){
            // Hex numbers
            if(++scan_i != text.size()) while( (charIsNumeric(text[scan_i]) or (text[scan_i] >= 'a' and text[scan_i] <= 'f') or (text[scan_i] >= 'A' and text[scan_i] <= 'F') )) if(++scan_i == text.size()) break;
        } else {
            // Regular numbers
            bool can_e = true;
            while(true){
                if(!(charIsNumeric(text[scan_i]) || text[scan_i] == '.')){
                    if(text[scan_i] == 'e' && can_e){
                        can_e = false;
                    } else break;
                }

                if(++scan_i == text.size()) break;
            }
        }

        if(scan_i != text.size()){
            if(text[scan_i] == 's'){
                scan_i++;
                if(scan_i != text.size()){
                    if(text[scan_i] == 'b') scan_i++;
                    else if(text[scan_i] == 's') scan_i++;
                    else if(text[scan_i] == 'i') scan_i++;
                    else if(text[scan_i] == 'l') scan_i++;
                }
            }
            else if(text[scan_i] == 'u'){
                scan_i++;
                if(scan_i != text.size()){
                    if(text[scan_i] == 'b') scan_i++;
                    else if(text[scan_i] == 's') scan_i++;
                    else if(text[scan_i] == 'i') scan_i++;
                    else if(text[scan_i] == 'l') scan_i++;
                    else if(text[scan_i] == 'z') scan_i++;
                }
            }
            else if(text[scan_i] == 'b'){scan_i++;}
            else if(text[scan_i] == 'i'){scan_i++;}
            else if(text[scan_i] == 'l'){scan_i++;}
            else if(text[scan_i] == 'f'){scan_i++;}
            else if(text[scan_i] == 'd'){scan_i++;}
            else if(text[scan_i] == 'h'){scan_i++;}
        }

        this->setStringColor(i, scan_i - i, palette.number);
        i += scan_i - i - 1;
    } else {
        this->setStringColor(i, text.size() - i, palette.number);
        i += scan_i - i - 1;
    }
}

void RichText::setCharacterColor(size_t index, const Vector3f& color){
    this->setCharacterColor(index, color.x, color.y, color.z);
}

void RichText::setCharacterColor(size_t index, float r, float g, float b){
    this->colors[index * 12    ] = r;
    this->colors[index * 12 + 1] = g;
    this->colors[index * 12 + 2] = b;
    this->colors[index * 12 + 3] = r;
    this->colors[index * 12 + 4] = g;
    this->colors[index * 12 + 5] = b;
    this->colors[index * 12 + 6] = r;
    this->colors[index * 12 + 7] = g;
    this->colors[index * 12 + 8] = b;
    this->colors[index * 12 + 9] = r;
    this->colors[index * 12 + 10] = g;
    this->colors[index * 12 + 11] = b;
}

void RichText::setStringColor(size_t index, size_t length, const Vector3f& color){
    this->setStringColor(index, length, color.x, color.y, color.z);
}

void RichText::setStringColor(size_t index, size_t length, float r, float g, float b){
    for(size_t i = index; i != index + length; i++) this->setCharacterColor(i, r, g, b);
}


#include "UTIL/document.h"

size_t getLineNumber(const std::string& text, size_t position){
    size_t line = 1;
    for(size_t i = 0; i != position; i++) if(text[i] == '\n') line++;
    return line;
}

size_t getLineLength(const std::string& text, size_t beginning){
    size_t i = beginning;
    while(i != text.length() && text[i] != '\n') i++;
    return i - beginning;
}

size_t getCurrentLineLength(const std::string& text, size_t position){
    return getLineLength(text, getLineBeginning(text, position));
}

size_t getLineBeginning(const std::string& text, size_t position){
    while(position != 0 && text[position - 1] != '\n') position--;
    return position;
}

size_t getLineEnd(const std::string& text, size_t position){
    while(position < text.length() && text[position] != '\n') position++;
    return position;
}

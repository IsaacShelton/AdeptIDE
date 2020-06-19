
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>

#include "UTIL/strings.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"
#include "INTERFACE/Finder.h"

Finder::Finder(){
    this->container = NULL;
    this->constantText = "Find Text: ";
    this->occurrence = 0; // 0 == first occurrence
    this->unchangedSinceLastGetLineNumber = false;
}

void Finder::load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight){
    TextBar::load(settings, font, fontTexture, containerWidth, containerHeight);
    this->clearInputOnOpen = true;
}

int Finder::getLineNumber(const std::string& documentText){
    // NOTE: Returns 0 when not valid line number

    if(this->unchangedSinceLastGetLineNumber){
        return this->getNextLineNumber(documentText);
    } else {
        this->unchangedSinceLastGetLineNumber = true;
    }

    std::string needle = this->getInput();
    size_t count = string_count(documentText, needle);

    // Needle doesn't exist in haystack
    if(count == 0) return 0;

    size_t index = string_find_nth(documentText, needle, this->occurrence);
    std::string upto = documentText.substr(0, index);
    return std::count(upto.begin(), upto.end(), '\n') + 1;
}

int Finder::getNextLineNumber(const std::string& documentText){
    // NOTE: Returns 0 when not valid line number

    std::string needle = this->getInput();
    size_t count = string_count(documentText, needle);

    // Needle doesn't exist in haystack
    if(count == 0) return 0;

    // Increament occurrence
    if(++this->occurrence >= (long long) count) this->occurrence = 0;

    size_t index = string_find_nth(documentText, needle, this->occurrence);
    std::string upto = documentText.substr(0, index);
    return std::count(upto.begin(), upto.end(), '\n') + 1;
}

int Finder::getPrevLineNumber(const std::string& documentText){
    // NOTE: Returns 0 when not valid line number

    std::string needle = this->getInput();
    size_t count = string_count(documentText, needle);

    // Needle doesn't exist in haystack
    if(count == 0) return 0;

    // Decreament occurrence
    if(this->occurrence-- == 0) this->occurrence = count - 1;

    size_t index = string_find_nth(documentText, needle, this->occurrence);
    std::string upto = documentText.substr(0, index);
    return std::count(upto.begin(), upto.end(), '\n') + 1;
}

void Finder::onType(){
    this->occurrence = 0;
    this->unchangedSinceLastGetLineNumber = false;
}

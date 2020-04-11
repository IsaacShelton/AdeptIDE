
#ifndef FINDER_H_INCLUDED
#define FINDER_H_INCLUDED

#include "INTERFACE/TextBar.h"

class Finder : public TextBar {
    int occurrence;
    bool unchangedSinceLastGetLineNumber;

public:
    Finder();
    void load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight);
    int getLineNumber(const std::string& documentText);
    int getPrevLineNumber(const std::string& documentText);
    int getNextLineNumber(const std::string& documentText);
    void onType();
};

#endif

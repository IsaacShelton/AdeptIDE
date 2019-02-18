
#include "INTERFACE/GenericEditor.h"

void GenericEditor::load(Settings *settings, Font *font, Texture *fontTexture, float maxWidth, float maxHeight){
    this->settings = settings;
    this->font = font;
    this->fontTexture = fontTexture;

    this->maxWidth = maxWidth;
    this->maxHeight = maxHeight;
}

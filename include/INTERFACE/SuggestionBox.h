
#ifndef SUGGESTION_BOX_H
#define SUGGESTION_BOX_H

#include "OPENGL/Shader.h"
#include "OPENGL/Texture.h"
#include "OPENGL/SolidModel.h"
#include "INTERFACE/Font.h"
#include "INTERFACE/Settings.h"

class SuggestionBox {
private:
    Settings *settings;
    Texture *fontTexture;

    float width;
    float height;
    SolidModel *boxModel;
    TextModel textModel;

    void generateBoxModel();

public:
    Font *font;

    void free();
    void load(Settings *settings, Font *font, Texture *fontTexture);
    void generate(const std::string& text, size_t lines, size_t longest);
    void render(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader, float x, float y);
};

#endif // SUGGESTION_BOX_H
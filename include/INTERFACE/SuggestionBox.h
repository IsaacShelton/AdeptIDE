
#ifndef SUGGESTION_BOX_H
#define SUGGESTION_BOX_H

#include <vector>

#include "OPENGL/Shader.h"
#include "OPENGL/Texture.h"
#include "OPENGL/SolidModel.h"
#include "INTERFACE/Font.h"
#include "INTERFACE/Assets.h"
#include "INTERFACE/Settings.h"
#include "INTERFACE/SymbolWeight.h"

class SuggestionBox {
private:
    Settings *settings;
    Texture *fontTexture;

    float width;
    float height;
    SolidModel *boxModel;
    TextModel textModel;
    int heightInLines;
    size_t maxSuggestions;

    void generateBoxModel();

public:
    std::vector<SymbolWeight> symbolWeights;
    Font *font;

    void free();
    void load(Settings *settings, Font *font, Texture *fontTexture);
    void generate(const std::string& text, size_t lines, size_t longest);
    void render(Matrix4f& projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets, float x, float y);
    void setMaxSuggestions(size_t amount);
    void clear();
    void clearIfFileSuggestions();
};

#endif // SUGGESTION_BOX_H
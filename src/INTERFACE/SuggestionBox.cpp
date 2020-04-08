
#include "OPENGL/Matrix4f.h"
#include "OPENGL/Vector4f.h"
#include "INTERFACE/SuggestionBox.h"

void SuggestionBox::free(){
    delete boxModel;
    this->textModel.free();
}

void SuggestionBox::load(Settings *settings, Font *font, Texture *fontTexture){
    this->settings = settings;
    this->font = font;
    this->fontTexture = fontTexture;
    this->textModel = font->generatePlainTextModel("", FONT_SCALE);
}

void SuggestionBox::generate(const std::string& text, size_t lines, size_t longest){
    this->textModel.free();
    delete this->boxModel;

    this->textModel = font->generatePlainTextModel(text, FONT_SCALE);

    this->width = 20.0f + this->font->mono_character_width * FONT_SCALE * longest + 26.0f;
    this->height = 20.0f + this->font->line_height * FONT_SCALE * lines;
    this->boxModel = createSolidModel(this->width, this->height);
    this->heightInLines = lines;
}

void SuggestionBox::render(Matrix4f& projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets, float x, float y){
    if(settings->ide_suggestions_max == 0) return;
    if(this->boxModel == NULL) this->generate("No Insight", 1, 10);

    Matrix4f transformationMatrix;
    //Vector4f color(1.0, 0.19, 0.2, 1.0f);
    Vector4f color(0.13, 0.14, 0.15, 1.0f);

    transformationMatrix.translateFromIdentity(x, y, 0.6f);

    solidShader->bind();
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    solidShader->giveVector4f("color", color);
    this->boxModel->draw();
    
    transformationMatrix.translateFromIdentity(x + 10.0f, y + 10.0f, 0.69f);
    shader->bind();
    shader->giveMatrix4f("projection_matrix", projectionMatrix);
    shader->giveMatrix4f("transformation_matrix", transformationMatrix);

    size_t i = 0;
    for(const SymbolWeight& s : this->symbolWeights){
        if(i++ == settings->ide_suggestions_max) break;

        Model *model = NULL;
        switch(s.kind){
        case SymbolWeight::Kind::FUNCTION:
            model = assets->functionModel;
            break;
        case SymbolWeight::Kind::STRUCT:
            model = assets->structureModel;
            break;
        case SymbolWeight::Kind::GLOBAL:
            model = assets->globalModel;
            break;
        default:
            model = assets->functionModel;
        }

        renderModel(model);

        transformationMatrix.translate(0.0f, this->font->line_height * FONT_SCALE, 0.0f);
        shader->giveMatrix4f("transformation_matrix", transformationMatrix);
    }

    transformationMatrix.translateFromIdentity(x + 10.0f + (symbolWeights.size() == 0 ? 0.0f : 26.0f), y + 10.0f, 0.69f);
    fontShader->bind();
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);
    fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    fontShader->giveFloat("edge", 0.4f);
    fontShader->giveFloat("width", 0.43f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->fontTexture->getID());
    this->textModel.draw();
}

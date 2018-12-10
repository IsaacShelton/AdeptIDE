
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
    this->textModel = font->generatePlainTextModel("", 0.17f);
}

void SuggestionBox::generate(const std::string& text, size_t lines, size_t longest){
    this->textModel.free();
    this->textModel = font->generatePlainTextModel(text, 0.17f);

    this->width = 20.0f + this->font->mono_character_width * 0.17 * longest;
    this->height = 20.0f + this->font->line_height * 0.17 * lines;
    this->boxModel = createSolidModel(this->width, this->height);
}

void SuggestionBox::render(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader, float x, float y){
    if(this->boxModel == NULL) this->generate("No Insight", 1, 10);

    Matrix4f transformationMatrix;
    //Vector4f color(1.0, 0.19, 0.2, 1.0f);
    Vector4f color(0.13, 0.14, 0.15, 1.0f);

    transformationMatrix.translateFromIdentity(x, y, 0.9f);

    solidShader->bind();
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    solidShader->giveVector4f("color", color);
    this->boxModel->draw();

    transformationMatrix.translateFromIdentity(x + 10.0f, y + 10.0f, 0.99f);
    fontShader->bind();
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);
    fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    fontShader->giveFloat("edge", 0.4f);
    fontShader->giveFloat("width", 0.43f);
    this->textModel.draw();
}

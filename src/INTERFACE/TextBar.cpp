
#include "INTERFACE/TextBar.h"

TextBar::~TextBar(){
    delete this->container;
}

void TextBar::load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight){
    this->settings = settings;
    this->font = font;
    this->fontTexture = fontTexture;
    this->containerWidth = containerWidth;
    this->containerHeight = containerHeight;
    this->container = createSolidModel(containerWidth, containerHeight);
    this->visible = false;
    this->containerX = 0.0f;
    this->containerY = 0.0f;
    this->inputRichText.fileType = FileType::PLAIN_TEXT;
    this->inputRichText.setFont(this->font);
    this->inputRichText.insert(0, this->constantText);
}

void TextBar::render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets){
    if(!this->visible) return;

    // Draw container
    Vector4f color(0.13, 0.14, 0.15, 1.0f);
    this->transformationMatrix.translateFromIdentity(this->containerX, this->containerY, 0.92f);
    solidShader->bind();
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    solidShader->giveVector4f("color", color);
    this->container->draw();

    fontShader->bind();
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->fontTexture->getID());

    this->transformationMatrix.translateFromIdentity(this->containerX + 12.0f, this->containerY + 8.0f, 0.93f);
    fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    fontShader->giveFloat("width", 0.43);
    fontShader->giveFloat("edge", 0.275);
    fontShader->giveFloat("y_upper_clip", 0.0f);
    this->inputRichText.getTextModel()->draw();
}

bool TextBar::leftClick(AdeptIDE *adeptide, double x, double y){
    return this->visible ? this->isBeingHovered(x, y) : false;
}

bool TextBar::scrollUpIfHovering(double x, double y, int lineCount){
    return this->visible ? this->isBeingHovered(x, y) : false;
}

bool TextBar::scrollDownIfHovering(double x, double y, int lineCount){
    return this->visible ? this->isBeingHovered(x, y) : false;
}

bool TextBar::isBeingHovered(double x, double y){
    assert(false && "LineNavigator::isBeingHovered is unimplemented");
    return true;
}

bool TextBar::isVisible(){
    return this->visible;
}

void TextBar::toggleVisibility(){
    this->setVisibility(!this->visible);
}

void TextBar::setVisibility(bool visibility){
    this->visible = visibility;
    if(this->visible) this->inputRichText.remove(this->constantText.length(), this->inputRichText.text.length() - this->constantText.length());
}

bool TextBar::type(std::string text){
    if(!this->visible) return false;
    text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
    this->inputRichText.insert(this->inputRichText.text.length(), text);
    return true;
}

bool TextBar::type(unsigned int codepoint){
    if(!this->visible) return false;
    if(codepoint == '\n') return true;
    this->inputRichText.insert(this->inputRichText.text.length(), codepoint);
    return true;
}

bool TextBar::backspace(){
    if(!this->visible) return false;
    if(this->inputRichText.text.length() > this->constantText.length())
        this->inputRichText.remove(this->inputRichText.text.length() - 1, 1);
    return true;
}

std::string TextBar::getInput(){
    return this->inputRichText.text.substr(this->constantText.length());
}


#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include "INTERFACE/LineNavigator.h"
#include "UTIL/strings.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"

LineNavigator::LineNavigator(){
    this->container = NULL;
}

LineNavigator::~LineNavigator(){
    delete this->container;
}

void LineNavigator::load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight){
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
}

void LineNavigator::render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets){
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

bool LineNavigator::leftClick(AdeptIDE *adeptide, double x, double y){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        return true;
    }

    return false;
}

bool LineNavigator::scrollUpIfHovering(double x, double y, int lineCount){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        return true;
    }

    return false;
}

bool LineNavigator::scrollDownIfHovering(double x, double y, int lineCount){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        return true;
    }

    return false;
}

bool LineNavigator::isBeingHovered(double x, double y){
    assert(false && "LineNavigator::isBeingHovered is unimplemented");
    return true;
}

bool LineNavigator::isVisible(){
    return this->visible;
}

void LineNavigator::toggleVisibility(){
    this->visible = !this->visible;
    if(this->visible) this->inputRichText.remove(0, this->inputRichText.text.length());
}

void LineNavigator::setVisibility(bool visibility){
    this->visible = visibility;
    if(this->visible) this->inputRichText.remove(0, this->inputRichText.text.length());
}

bool LineNavigator::type(std::string text){
    if(!this->visible) return false;
    text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
    this->inputRichText.insert(this->inputRichText.text.length(), text);
    return true;
}

bool LineNavigator::type(unsigned int codepoint){
    if(!this->visible) return false;
    if(codepoint == '\n') return true;
    this->inputRichText.insert(this->inputRichText.text.length(), codepoint);
    return true;
}

bool LineNavigator::backspace(){
    if(!this->visible) return false;
    if(this->inputRichText.text.length() != 0)
        this->inputRichText.remove(this->inputRichText.text.length() - 1, 1);
    return true;
}

int LineNavigator::getLineNumber(){
    // NOTE: Returns 0 when not valid line number
    int line = to_int(this->inputRichText.text);
    return line < 0 ? 0 : line;
}


#include <math.h>
#include "INTERFACE/PopUp.h"
#include "UTIL/animationMath.h"

PopUp::PopUp(const std::string& message, Font *font, PopUp::Kind kind){
    const float minPopWidth = 512.0f;
    const float minPopHeight = 256.0f;

    this->model = font->generatePlainTextModel(message, FONT_SCALE);
    this->animationStartTime = glfwGetTime();
    this->closing = false;

    this->targetWidth = 512.0f;
    this->targetHeight = 256.0f;

    // Enforce minimum target size
    if(this->targetWidth  < minPopWidth)  this->targetWidth  = minPopWidth;
    if(this->targetHeight < minPopHeight) this->targetHeight = minPopHeight;
}

PopUp::~PopUp(){
    this->model.free();
}

void PopUp::close(){
    if(this->closing) return;

    this->animationStartTime = glfwGetTime();
    this->closing = true;
}

bool PopUp::shouldDie(){
    return this->closing && this->calculateT() >= 1.0f; // TODO
}

bool PopUp::leftClick(float xpos, float ypos, int windowWidth, int windowHeight){
    float t = this->calculateT();
    float percent = this->calculatePercent(t);
    float popWidth = this->targetWidth * percent;
    float popHeight = this->targetHeight * percent;

    float centerX = (float) windowWidth / 2.0f;
    float centerY = (float) windowHeight / 2.0f;

    float top  = centerY - popHeight / 2.0f;
    float left = centerX - popWidth  / 2.0f;

    if(!(xpos >= left && xpos <= left + popWidth && ypos >= top && ypos <= top + popHeight)){
        // Not clicked
        return false;
    }

    this->close();
    return true;
}

void PopUp::render(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader, Texture *fontTexture, int windowWidth, int windowHeight, AdeptIDEAssets *assets){
    Matrix4f transformationMatrix;
    float t = this->calculateT();
    float percent = this->calculatePercent(t);

    float popWidth = this->targetWidth * percent;
    float popHeight = this->targetHeight * percent;

    float centerX = (float) windowWidth / 2.0f;
    float centerY = (float) windowHeight / 2.0f;
    float left = centerX - popWidth / 2.0f;
    float top = centerY - popHeight / 2.0f;

    solidShader->bind();
    transformationMatrix.translateFromIdentity(left, top, 0.98f);
    transformationMatrix.scale(popWidth, popHeight, 1.0f);
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    solidShader->giveVector4f("color", Vector4f(0.05f, 0.07f, 0.08f, 1.0f));
    assets->singlePixelModel->draw();

    const float textPadding = 8.0f;

    fontShader->bind();
    transformationMatrix.translateFromIdentity(left + textPadding * percent, top + textPadding * percent, 0.99f);
    transformationMatrix.scale(percent, percent, 0.0f);
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);
    fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    fontShader->giveFloat("edge", 0.4f);
    fontShader->giveFloat("width", 0.43f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture->getID());
    this->model.draw();
}

float PopUp::calculateT(){
    const float animationDuration = this->closing ? 0.1 : 1.0;
    float t = (glfwGetTime() - this->animationStartTime) / animationDuration;
    return t < 0 ? 0 : t > 1 ? 1 : t;
}

float PopUp::calculatePercent(double t){
    return this->closing ? 1.0f - t * t : easeOutElastic(t);
}

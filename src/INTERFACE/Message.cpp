
#include <math.h>
#include "UTIL/animationMath.h"
#include "INTERFACE/Message.h"

Message::Message(const std::string& message, Font *font, double seconds, int windowWidth, int windowHeight){
    exitTime = glfwGetTime() + seconds;
    this->model = font->generatePlainTextModel(message, 0.17f);

    // Calculate Text Width
    int widthInCharacters = 0;
    int lineWidthInCharacters = 0;
    int newlines = 0;
    for(size_t i = 0; i != message.length(); i++){
        if(message[i] == '\n'){
            if(lineWidthInCharacters > widthInCharacters) widthInCharacters = lineWidthInCharacters;
            lineWidthInCharacters = 0;
            newlines++;
        } else {
            lineWidthInCharacters++;
        }
    }
    if(lineWidthInCharacters > widthInCharacters) widthInCharacters = lineWidthInCharacters;
    textWidth = widthInCharacters * (font->mono_character_width * 0.17f);
    textHeight = (newlines + 1) * (font->line_height * 0.17f);
    containerWidth = textWidth + 20.0f;
    containerHeight = textHeight + 20.0f;
    container = createSolidModel(containerWidth, containerHeight);
    x = windowWidth;
    y = windowHeight - containerHeight;
}

Message::~Message(){
    this->model.free();
    delete container;
}

void Message::update(int windowWidth, double delta){
    float targetX = (glfwGetTime() > exitTime) ? windowWidth : windowWidth - containerWidth;

    if(fabs(this->x - targetX) > 0.01f){
        this->x += (targetX > this->x ? 1 : -1) * fabs(this->x - targetX) * clampedHalfDelta(delta);
    } else this->x = targetX;
}

void Message::render(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader, Texture *fontTexture, int windowWidth, int windowHeight){
    Matrix4f transformationMatrix;

    Vector4f color(0.13, 0.14, 0.15, 1.0f);

    solidShader->bind();
    transformationMatrix.translateFromIdentity(x, y, 0.79f);
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    solidShader->giveVector4f("color", color);
    container->draw();

    fontShader->bind();
    transformationMatrix.translateFromIdentity(x + 10.0f, y + 10.0f, 0.8f);
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);
    fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    fontShader->giveFloat("edge", 0.4f);
    fontShader->giveFloat("width", 0.43f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture->getID());
    this->model.draw();
}

bool Message::shouldClose(int windowWidth){
    return glfwGetTime() > exitTime && x >= windowWidth;
}

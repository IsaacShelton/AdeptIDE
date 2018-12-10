
#ifndef MESSAGE_H_INCLUDED
#define MESSAGE_H_INCLUDED

#include <string>
#include "OPENGL/Model.h"
#include "OPENGL/SolidModel.h"
#include "OPENGL/Shader.h"
#include "OPENGL/Matrix4f.h"
#include "INTERFACE/Font.h"

class Message {
    double timer;
    std::string text;
    TextModel model;
    SolidModel *container;
    float textWidth, textHeight;
    float containerWidth, containerHeight;
    float x, y;

    SolidModel* generateContainerModel();

public:
    Message(const std::string& message, Font *font, double seconds, int windowWidth, int windowHeight);
    ~Message();
    void update(int windowWidth, double delta);
    void render(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader, Texture *fontTexture, int windowWidth, int windowHeight);
    bool shouldClose(int windowWidth);
};

#endif // MESSAGE_H_INCLUDED

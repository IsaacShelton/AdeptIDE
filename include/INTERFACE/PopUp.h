
#ifndef POPUP_H_INCLUDED
#define POPUP_H_INCLUDED

#include <string>
#include "OPENGL/Model.h"
#include "OPENGL/Shader.h"
#include "OPENGL/Matrix4f.h"
#include "INTERFACE/Assets.h"
#include "INTERFACE/Font.h"

class PopUp {
    std::string text;
    TextModel model;
    double animationStartTime;
    bool closing;
    float targetWidth;
    float targetHeight;

    float calculateT();
    float calculatePercent(double t);

public:
    enum Kind {OK};

    PopUp(const std::string& message, Font *font, PopUp::Kind kind);
    ~PopUp();
    void close();
    bool shouldDie();

    bool leftClick(float xpos, float ypos, int windowWidth, int windowHeight);
    void render(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader, Texture *fontTexture, int windowWidth, int windowHeight, AdeptIDEAssets *assets);
};

#endif // POPUP_H_INCLUDED

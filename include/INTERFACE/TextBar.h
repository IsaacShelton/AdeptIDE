
#ifndef TEXT_BAR_H_INCLUDED
#define TEXT_BAR_H_INCLUDED

class AdeptIDE;

#include <string>
#include "OPENGL/Model.h"
#include "OPENGL/SolidModel.h"
#include "OPENGL/Shader.h"
#include "OPENGL/Texture.h"
#include "OPENGL/Matrix4f.h"
#include "INTERFACE/Font.h"
#include "INTERFACE/Assets.h"
#include "INTERFACE/Settings.h"
#include "INTERFACE/RichText.h"

class TextBar {
protected:
    Settings *settings;
    Font *font;
    Texture *fontTexture;
    bool visible;
    SolidModel *container;
    Matrix4f transformationMatrix;
    RichText inputRichText;
    std::string constantText;
public:
    float containerWidth, containerHeight;
    float containerX, containerY;

    virtual ~TextBar();
    virtual void load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight);
    virtual void render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets);
    virtual bool leftClick(AdeptIDE *adeptide, double x, double y);
    virtual bool scrollUpIfHovering(double x, double y, int lineCount);
    virtual bool scrollDownIfHovering(double x, double y, int lineCount);
    virtual bool isBeingHovered(double x, double y);
    virtual bool isVisible();
    virtual void toggleVisibility();
    virtual void setVisibility(bool visibility);
    virtual bool type(std::string text);
    virtual bool type(unsigned int codepoint);
    virtual bool backspace();
    virtual std::string getInput();
};

#endif // TEXT_BAR_H_INCLUDED


#ifndef TERMINAL_H
#define TERMINAL_H

class AdeptIDE;

#include <vector>
#include "INTERFACE/Settings.h"
#include "OPENGL/Model.h"
#include "OPENGL/SolidModel.h"
#include "OPENGL/Shader.h"
#include "OPENGL/Texture.h"
#include "OPENGL/Matrix4f.h"
#include "INTERFACE/Font.h"
#include "INTERFACE/Assets.h"
#include "INTERFACE/Settings.h"
#include "INTERFACE/RichText.h"
#include "PROCESS/PseudoTerminal.h"

class Terminal {
    Settings *settings;
    Font *font;
    Texture *fontTexture;
    bool visible;

    SolidModel *container;
    float containerWidth, containerHeight;
    Matrix4f transformationMatrix;
    float recede;
    int scroll;
    float scrollYOffset;
    float targetScrollYOffset;

    RichText richText;
    PseudoTerminal *pseudoTerminal;
    size_t lineCount;

    std::string editable_buffer;
    bool richTextNotUpdatedSinceInput;

    std::vector<std::string> history;
    int history_index = 0;

    float calculateScrollOffset();
    float calculateScrollOffset(size_t line);

public:
    float containerX;

    Terminal();
    ~Terminal();
    void load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight);
    void render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, float windowHeight, AdeptIDEAssets *assets);
    void resize(float width);
    bool isVisible();
    void toggleVisibility();
    void setVisibility(bool visibility);
    bool type(const std::string& text);
    bool type(unsigned int codepoint);
    bool backspace();
    void append(unsigned int codepoint);
    void append(const std::string& text);
    void clear();
    void paste(GLFWwindow *window);

    bool scrollDownIfHovering(double x, double y, double windowHeight, int lineCount);
    bool scrollUpIfHovering(double x, double y, double windowHeight, int lineCount);
    bool isBeingHovered(double x, double y, double windowHeight);

    void up();
    void down();
};

#include "INTERFACE/AdeptIDE.h"

#endif // TERMINAL_H

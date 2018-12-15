
#ifndef MENUBAR_H
#define MENUBAR_H

#include <vector>

#include "INTERFACE/Font.h"
#include "INTERFACE/Settings.h"
#include "OPENGL/Model.h"
#include "OPENGL/Shader.h"
#include "OPENGL/SolidModel.h"
#include "INTERFACE/TextEditor.h"

typedef void (*MenuAction)(void*);

class Menu;
class DropdownMenu;

class Menu {
public:
    enum FontWidthTarget {
        REGULAR, BOLD
    };

    TextModel model;
    MenuAction action;
    float textWidth;
    void *data;

    DropdownMenu *dropdownMenu;

    float fontWidth;
    FontWidthTarget targetFontWidth;

    Menu(const std::string& label, Font *font, MenuAction action, void *data);
    ~Menu();

    void update(double delta);
    void targetPlainFontWidth();
    void targetBoldFontWidth();
};

class DropdownMenu {
    bool wasOpen;
    float height;

public:
    bool isOpen;
    
    float popupX;
    float popupY;

    std::vector<Menu*> menus;
    float menuHeights;
    SolidModel *containerModel;
    float width;

    DropdownMenu(Font *font, float popupX, float popupY, int max_characters);
    ~DropdownMenu();
    SolidModel* generateContainerModel();
    void update(double delta);
    void renderIfOpen(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader);

    bool leftClick(double xpos, double ypos);
    void changePopupPosition(float popupX, float popupY);
    float getBottom();

    void collapseAll();
};

class MenuBar {
private:
    Settings *settings;
    Texture *fontTexture;

    float xOffset;
    float yOffset;
    float previousWindowWidth;
    std::vector<GenericEditor *> *editors;

    SolidModel *barModel;
    SolidModel *underlineBaseModel;

    float tabUnderlineBeginX;
    float targetTabUnderlineBeginX;
    float tabUnderlineEndX;
    float targetTabUnderlineEndX;

    SolidModel* generateBarModel(float windowWidth);
    SolidModel* generateUnderlineBaseModel();

public:
    Font *font;
    std::vector<Menu*> menus;
    
    MenuBar();
    ~MenuBar();

    void load(Settings *settings, Font *font, Texture *fontTexture, std::vector<GenericEditor*> *editors);
    void addMenu(const std::string& label, MenuAction action, void *data);

    void update();
    void render(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader, float windowWidth);

    bool leftClick(double xpos, double ypos, size_t *outCurrentEditorIndex);
    void targetAllPlainFontWidth();
    void loseFocus();

    void underlineTab(size_t tab);
};

#endif // MENUBAR_H


#include <algorithm>

#include "OPENGL/Vector4f.h"
#include "INTERFACE/MenuBar.h"

Menu::Menu(const std::string& label, Font *font, MenuAction action, void *data){
    this->model = font->generatePlainTextModel(label, 0.17f);
    this->action = action;
    this->textWidth = (font->mono_character_width * 0.17f) * label.length();
    this->targetPlainFontWidth();
    this->fontWidth = 0.43f;
    this->data = data;
    this->dropdownMenu = NULL;
}

Menu::~Menu(){
    this->model.free();
    delete dropdownMenu;
}

void Menu::update(){
    float targetFontWidthValue = this->targetFontWidth == FontWidthTarget::REGULAR ? 0.43f : 0.525f;

    if(fabs(this->fontWidth - targetFontWidthValue) > 0.0001f){
        this->fontWidth += (targetFontWidthValue > this->fontWidth ? 1 : -1) * fabs(this->fontWidth - targetFontWidthValue) * 0.5f;
    } else this->fontWidth = targetFontWidthValue;
}

void Menu::targetPlainFontWidth(){
    this->targetFontWidth = FontWidthTarget::REGULAR;
}

void Menu::targetBoldFontWidth(){
    this->targetFontWidth = FontWidthTarget::BOLD;
}

DropdownMenu::DropdownMenu(Font *font, float popupX, float popupY, int max_characters){
    this->wasOpen = false;
    this->height = 0;
    this->isOpen = false;
    this->popupX = popupX;
    this->popupY = popupY;
    this->menuHeights = font->line_height * 0.17f;
    if(max_characters == 0) this->width = 256.0f;
    else this->width = font->mono_character_width * 0.17f * (float) max_characters + 40.0f;
    this->containerModel = NULL;
}

DropdownMenu::~DropdownMenu(){
    for(Menu *menu : this->menus) delete menu;
}

void DropdownMenu::update(double delta){
    float targetHeight = isOpen ? this->getBottom() : 0;

    if(fabs(this->height - targetHeight) > 0.01f){
        this->height += (targetHeight > this->height ? 1 : -1) * fabs(this->height - targetHeight) * (-1 / (2 * delta) + 1);
    } else this->height = targetHeight;

    for(Menu *menu : this->menus){
        menu->update();

        if(menu->dropdownMenu){
            if(!this->isOpen) menu->dropdownMenu->isOpen = false;
            menu->dropdownMenu->update(delta);
        }
    }
}

void DropdownMenu::renderIfOpen(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader){
    if(this->height <= 0) return;

    delete this->containerModel;
    this->containerModel = NULL;

    if(this->containerModel == NULL){
        this->containerModel = createSolidModel(this->width, this->height);
    }

    Matrix4f transformationMatrix;
    //Vector3f color(0.17, 0.19, 0.2);
    Vector4f color(0.13, 0.14, 0.15, 1.0f);

    transformationMatrix.translateFromIdentity(this->popupX, this->popupY, 0.9f);

    solidShader->bind();
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    solidShader->giveVector4f("color", color);
    this->containerModel->draw();

    float drawingY = this->menuHeights * 0.70f;
    transformationMatrix.translate(20.0f, this->menuHeights * 0.70f, 0.01f);

    fontShader->bind();
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);
    fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    fontShader->giveFloat("edge", 0.4f);

    for(Menu *menu : this->menus){
        if(drawingY + this->menuHeights * 1.70f > this->height) break;

        fontShader->giveFloat("width", menu->fontWidth);
        menu->model.draw();
        transformationMatrix.translate(0.0f, this->menuHeights * 1.70f, 0.0f);
        fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
        drawingY += this->menuHeights * 0.70f;
    }

    for(Menu *menu : this->menus){
        if(menu->dropdownMenu){
            menu->dropdownMenu->renderIfOpen(projectionMatrix, fontShader, solidShader);
        }
    }
}

void DropdownMenu::changePopupPosition(float popupX, float popupY){
    this->popupX = popupX;
    this->popupY = popupY;
}

bool DropdownMenu::leftClick(double xpos, double ypos){
    if(!this->isOpen) return false;

    float bottom = this->getBottom();

    if(xpos >= this->popupX && xpos <= this->popupX + this->width
    && ypos >= this->popupY && ypos <= this->popupY + bottom){
        int index = floor((ypos - this->menuHeights * 0.70f) / (this->menuHeights * 1.70f));

        if(index < 0) index = 0;
        else if(index >= (int) this->menus.size()) index = this->menus.size() - 1;

        Menu *targetMenu = this->menus[index];

        if(targetMenu->action){
            targetMenu->action(targetMenu->data);
            this->isOpen = false;
        } else if(targetMenu->dropdownMenu){
            // Collapse sibling dropdown menus
            for(Menu *another : this->menus)
                if(another != targetMenu && another->dropdownMenu) another->dropdownMenu->collapseAll();
            
            targetMenu->dropdownMenu->isOpen = !targetMenu->dropdownMenu->isOpen;
        }

        return true;
    }

    for(Menu *menu : this->menus){
        if(menu->dropdownMenu && menu->dropdownMenu->leftClick(xpos, ypos)){
            return true;
        }
    }

    return false;
}

float DropdownMenu::getBottom(){
    return this->menuHeights * 0.75f + (float) this->menus.size() * this->menuHeights * 1.70f;
}

void DropdownMenu::collapseAll(){
    for(Menu *menu : this->menus){
        if(menu->dropdownMenu) menu->dropdownMenu->collapseAll();
    }
    this->isOpen = false;
}

MenuBar::MenuBar(){
    this->previousWindowWidth = -1.0f;
    this->barModel = NULL;
    this->underlineBaseModel = NULL;
    this->tabUnderlineBeginX = 0.0f;
    this->targetTabUnderlineBeginX = 0.0f;
    this->tabUnderlineEndX = 0.0f;
    this->targetTabUnderlineEndX = 0.0f;
}

MenuBar::~MenuBar(){
    for(Menu *menu : this->menus) delete menu;
    delete this->barModel;
    delete this->underlineBaseModel;
}

void MenuBar::load(Settings *settings, Font *font, Texture *fontTexture, std::vector<Editor*> *editors){
    this->settings = settings;
    this->font = font;
    this->fontTexture = fontTexture;
    this->xOffset = 0.0f;
    this->yOffset = 0.0f;
    this->editors = editors;
    this->tabUnderlineBeginX = 0.0f;
    this->targetTabUnderlineBeginX = 0.0f;
    this->tabUnderlineEndX = 0.0f;
    this->targetTabUnderlineEndX = 0.0f;
    this->underlineBaseModel = createSolidModel(1.0f, 1.0f);
}

void MenuBar::addMenu(const std::string& label, MenuAction action, void *data){
    this->menus.push_back(new Menu(label, this->font, action, data));
}

void MenuBar::update(){
    if(fabs(this->tabUnderlineBeginX - this->targetTabUnderlineBeginX) > 0.01f){
        this->tabUnderlineBeginX += (this->targetTabUnderlineBeginX > this->tabUnderlineBeginX ? 1 : -1) * fabs(this->tabUnderlineBeginX - this->targetTabUnderlineBeginX) * (-1 / (2 * this->settings->hidden.delta) + 1);
    } else this->tabUnderlineBeginX = this->targetTabUnderlineBeginX;

    if(fabs(this->tabUnderlineEndX - this->targetTabUnderlineEndX) > 0.01f){
        this->tabUnderlineEndX += (this->targetTabUnderlineEndX > this->tabUnderlineEndX ? 1 : -1) * fabs(this->tabUnderlineEndX - this->targetTabUnderlineEndX) * (-1 / (2 * this->settings->hidden.delta) + 1);
    } else this->tabUnderlineEndX = this->targetTabUnderlineEndX;

    for(Menu *menu : this->menus){
        menu->update();

        if(menu->dropdownMenu) menu->dropdownMenu->update(this->settings->hidden.delta);
    }
}

void MenuBar::render(Matrix4f& projectionMatrix, Shader *fontShader, Shader *solidShader, float windowWidth){
    if(barModel == NULL || windowWidth != previousWindowWidth){
        delete this->barModel;
        this->barModel = createSolidModel(windowWidth, 20.0 + 32.0f);
        this->previousWindowWidth = windowWidth;
    }

    Matrix4f transformationMatrix;
    Vector4f color(0.17, 0.19, 0.2, 1.0f);
    //Vector4f color(0.00, 0.61, 0.85, 1.0f);

    transformationMatrix.translateFromIdentity(this->xOffset, this->yOffset, 0.7f);

    solidShader->bind();
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    solidShader->giveVector4f("color", color);
    this->barModel->draw();

    if(this->tabUnderlineBeginX != this->tabUnderlineEndX){
        transformationMatrix.translateFromIdentity(this->tabUnderlineBeginX, 45.5f, 0.71f);
        transformationMatrix.scale(this->tabUnderlineEndX - this->tabUnderlineBeginX, 2.0f, 1.0f);
        solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
        solidShader->giveVector4f("color", Vector4f(0.83, 0.83, 0.83, 1.0f));
        this->underlineBaseModel->draw();
    }

    const float menuSpacing = 16.0f;

    transformationMatrix.translateFromIdentity(this->xOffset + 8.0f, this->yOffset + 2.0f, 0.8f);

    fontShader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->fontTexture->getID());
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);
    fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    fontShader->giveFloat("edge", 0.4f);

    for(Menu *menu : this->menus){
        fontShader->giveFloat("width", menu->fontWidth);
        menu->model.draw();
        transformationMatrix.translate(menu->textWidth + menuSpacing, 0.0f, 0.0f);
        fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    }

    for(Menu *menu : this->menus){
        if(menu->dropdownMenu){
            menu->dropdownMenu->renderIfOpen(projectionMatrix, fontShader, solidShader);
        }
    }
}

bool MenuBar::leftClick(double xpos, double ypos, size_t *outCurrentEditorIndex){
    // Click on actual menu bar
    if(ypos >= this->yOffset && ypos <= this->yOffset + 20.0f){
        const float padding = 8.0f;
        const float spacing = 16.0f;
        float menuX = this->xOffset + padding;

        for(Menu *menu : this->menus){
            if(xpos >= menuX - padding && xpos <= menuX + menu->textWidth + padding){
                
                if(menu->targetFontWidth == Menu::FontWidthTarget::BOLD){
                    this->loseFocus();
                    return true;
                }

                this->loseFocus();

                menu->targetBoldFontWidth();
                if(menu->action) menu->action(menu->data);
                
                return true;
            }

            menuX += menu->textWidth + spacing;
        }

        return true;
    }

    // Try for dropdown menus
    for(Menu *menu : this->menus){
        if(menu->dropdownMenu && menu->dropdownMenu->leftClick(xpos, ypos)){
            return true;
        }
    }

    // Try for tab selection
    if(ypos >= this->yOffset + 20.0f && ypos <= this->yOffset + 20.0f + 32.0f){
        const float padding = 8.0f;
        const float spacing = 24.0f;
        float displayNameX = this->xOffset + padding;

        for(size_t i = 0; i != this->editors->size(); i++){
            float advance = ((*this->editors)[i]->displayFilename.length() * (0.17f * this->font->mono_character_width)) + 16.0f;

            if(xpos >= displayNameX && xpos <= displayNameX + advance){
                *outCurrentEditorIndex = i;
                this->underlineTab(i);
                return true;
            }

            displayNameX += advance + spacing;
        }
    }

    return false;
}

void MenuBar::targetAllPlainFontWidth(){
    for(Menu *menu : this->menus) menu->targetPlainFontWidth();
}

void MenuBar::loseFocus(){
    this->targetAllPlainFontWidth();

    for(Menu *menu : this->menus){
        if(menu->dropdownMenu) menu->dropdownMenu->collapseAll();
    }
}

void MenuBar::underlineTab(size_t tab){
    const float padding = 8.0f;
    const float spacing = this->settings->editor_icons ? 24.0f : 0.0f;

    if(tab >= this->editors->size()){
        this->targetTabUnderlineBeginX = padding;
        this->targetTabUnderlineEndX = padding;
        return;
    }

    float startX = padding;

    for(size_t i = 0; i != tab; i++){
        float advance = ((*this->editors)[i]->displayFilename.length() * (0.17f * this->font->mono_character_width)) + 2 * padding;
        startX += advance + spacing;
    }

    this->targetTabUnderlineBeginX = startX;
    this->targetTabUnderlineEndX = startX + ((*this->editors)[tab]->displayFilename.length() * (0.17f * this->font->mono_character_width)) + spacing;
}

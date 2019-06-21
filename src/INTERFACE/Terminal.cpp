
#include <math.h>
#include "INTERFACE/Alert.h"
#include "INTERFACE/Terminal.h"
#include "UTIL/animationMath.h"

Terminal::Terminal(){
    this->container = NULL;
    this->pseudoTerminal = NULL;
}

Terminal::~Terminal(){
    delete this->container;
    delete this->pseudoTerminal;
}

void Terminal::load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight){
    this->settings = settings;
    this->font = font;
    this->fontTexture = fontTexture;
    this->containerWidth = containerWidth;
    this->containerHeight = containerHeight;
    this->container = createSolidModel(containerWidth, containerHeight);
    this->visible = settings->terminal_show;
    this->containerX = settings->explorer_default_show ? 256.0f : 0.0f;
    this->scroll = 0;
    this->scrollYOffset = 0;
    this->targetScrollYOffset = 0;

    this->richText.fileType = FileType::PLAIN_TEXT;
    this->richText.setFont(this->font);

    if((this->pseudoTerminal = PseudoTerminal::create(settings)) == NULL){
        alertError("Terminal Unsupported", "Terminal Unsupported");
    }

    this->lineCount = 1;

    this->editable_buffer = "";
    this->richTextNotUpdatedSinceInput = true;
}

void Terminal::resize(float width){
    if(this->containerWidth == width) return;
    
    delete this->container;
    containerWidth = width;
    this->container = createSolidModel(containerWidth, containerHeight);
}

void Terminal::render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, float windowHeight, AdeptIDEAssets *assets){
    float targetRecede = this->visible ? 256 : 0;
    this->targetScrollYOffset = this->calculateScrollOffset();

    if(fabs(this->recede - targetRecede) > 0.01f){
        this->recede += (targetRecede > this->recede ? 1 : -1) * fabs(this->recede - targetRecede) * clampedHalfDelta(this->settings->hidden.delta);
    } else this->recede = targetRecede;

    if(fabs(this->scrollYOffset - this->targetScrollYOffset) > 0.01f){
        this->scrollYOffset += (this->targetScrollYOffset > this->scrollYOffset ? 1 : -1) * fabs(this->scrollYOffset - this->targetScrollYOffset) * clampedHalfDelta(this->settings->hidden.delta);
    } else this->scrollYOffset = this->targetScrollYOffset;

    Vector4f color(0.17f * 0.4, 0.19 * 0.4, 0.2 * 0.4, this->settings->terminal_transparent ? 0.5f : 1.0f);
    this->transformationMatrix.translateFromIdentity(this->containerX, windowHeight - this->recede, 0.9f);
    solidShader->bind();
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    solidShader->giveVector4f("color", color);
    this->container->draw();

    fontShader->bind();
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->fontTexture->getID());

    this->transformationMatrix.translateFromIdentity(this->containerX + 12.0f, windowHeight - this->recede + 8.0f  - this->scrollYOffset, 0.91f);
    fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    fontShader->giveFloat("width", 0.43);
    fontShader->giveFloat("edge", 0.275);
    fontShader->giveFloat("y_upper_clip", this->recede - 8.0f);

    this->richText.getTextModel()->draw();

    if(this->pseudoTerminal){
        std::string out = this->pseudoTerminal->readOutput();

        // Filter out CSI escape sequences
        for(size_t i = 0; i < out.length(); i++){
            if(out[i] != '\e') continue;
            size_t start = i;

            if(++i < out.length() && out[i] == '['){
                while(++i < out.length() && out[i] >= 0x30 && out[i] <= 0x3F);
                if(i < out.length() && out[i] >= 0x40 && out[i] <= 0x7E) i++;

                if(out.substr(start, i - start) == "\e[2J"){
                    this->clear();
                }

                out.erase(start, i - start);
                i = start - 1;
            }
        }

        this->append(out);
        if(out.length() != 0) this->richTextNotUpdatedSinceInput = false;
    }
    fontShader->giveFloat("y_upper_clip", 0.0f);
}

bool Terminal::isVisible(){
    return this->visible;
}

void Terminal::toggleVisibility(){
    this->visible = !this->visible;
}

void Terminal::setVisibility(bool visibility){
    this->visible = visibility;
}

bool Terminal::type(const std::string& text){
    if(!this->visible) return false;

    for(size_t i = 0; i != text.length(); i++)
        this->type((unsigned int) text[i]);

    return true;
}

bool Terminal::type(unsigned int codepoint){
    if(!this->visible) return false;

    if(editable_buffer.length() == 0){
        this->richTextNotUpdatedSinceInput = true;
    }

    if((char) codepoint == '\n' && this->pseudoTerminal){
        // Feed buffer to pseudo terminal
        this->pseudoTerminal->feedInput(this->editable_buffer + "\n");
        this->history.insert(this->history.begin(), editable_buffer);
        editable_buffer = "";

        if(this->history.size() > 25) this->history.erase(this->history.begin() + 25);
        this->history_index = -1;
    } else {
        editable_buffer += (char) codepoint;
    }

    this->append(codepoint);
    return true;
}

bool Terminal::backspace(){
    if(!this->visible) return false;
    if(this->editable_buffer.length() == 0) return true;
    this->editable_buffer.erase(this->editable_buffer.length() - 1, 1);

    if(this->richTextNotUpdatedSinceInput){
        this->richText.remove(this->richText.text.length() - 1, 1);
    } else {
        this->append(this->editable_buffer);
        this->richTextNotUpdatedSinceInput = true;
    }

    return true;
}

void Terminal::append(unsigned int codepoint){
    if((char) codepoint == '\n'){
        if(++this->lineCount > 12) this->scroll = this->lineCount - 12;
    }
    this->richText.insert(this->richText.text.length(), codepoint);
}

void Terminal::append(const std::string& text){
    for(size_t i = 0; i != text.length(); i++)
        if(text[i] == '\n'){
            if(++this->lineCount > 12) this->scroll = this->lineCount - 12;
        }
    
    this->richText.insert(this->richText.text.length(), text);
}

void Terminal::clear(){
    this->richText.remove(0, this->richText.text.length());
    this->lineCount = 1;
    this->scroll = 0;
}

void Terminal::paste(GLFWwindow *window){
    this->type(std::string(glfwGetClipboardString(window)));
}

bool Terminal::scrollDownIfHovering(double x, double y, double windowHeight, int lineAmount){
    if(!this->visible || !this->isBeingHovered(x, y, windowHeight)) return false;

    this->scroll += lineAmount;
    if(this->scroll > (int) this->lineCount - 1) this->scroll = this->lineCount - 1;
    return true;
}

bool Terminal::scrollUpIfHovering(double x, double y, double windowHeight, int lineAmount){
    if(!this->visible || !this->isBeingHovered(x, y, windowHeight)) return false;

    this->scroll -= lineAmount;
    if(this->scroll < 0) this->scroll = 0;
    return true;
}

bool Terminal::isBeingHovered(double x, double y, double windowHeight){
    return y > windowHeight - this->recede;
}

float Terminal::calculateScrollOffset(){
    return this->calculateScrollOffset(this->scroll);
}

float Terminal::calculateScrollOffset(size_t line){
    return (this->font->line_height * FONT_SCALE) * line;
}

void Terminal::up(){
    if(this->history_index + 1 < (int) this->history.size()){
        for(int i = 0; i != 1000; i++) this->backspace();
        //this->richText.remove(this->richText.text.length() - this->editable_buffer.length(), this->editable_buffer.length());
        this->editable_buffer = this->history[++this->history_index];
        this->append(this->editable_buffer);
        this->richTextNotUpdatedSinceInput = true;
    }
}

void Terminal::down(){
    if(this->history_index > 0){
        for(int i = 0; i != 1000; i++) this->backspace();
        //this->richText.remove(this->richText.text.length() - this->editable_buffer.length(), this->editable_buffer.length());
        this->editable_buffer = this->history[--this->history_index];
        this->append(this->editable_buffer);
        this->richTextNotUpdatedSinceInput = true;
    }
}

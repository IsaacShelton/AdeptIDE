
#include <math.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "UTIL/strings.h"
#include "UTIL/lexical.h"
#include "UTIL/animationMath.h"
#include "OPENGL/Vector3f.h"
#include "INTERFACE/TextEditor.h"
#include "INTERFACE/SymbolWeight.h"
#include "PROCESS/background.h"

// src/INSIGHT/include
#include "UTIL/levenshtein.h"

TextEditor::~TextEditor(){
    if(this->hasLineNumbersModel) this->lineNumbersModel.free();
    if(this->hasFilenameModel) this->filenameModel.free();
    delete this->selection;

    for(Caret *additionalCaret : this->additionalCarets){
        delete additionalCaret;
    }
    for(Caret *additionalSelectionCarets : this->additionalSelectionStartCarets){
        delete additionalSelectionCarets;
    }
    for(Selection *additionalSelections : this->additionalSelections){
        delete additionalSelections;
    }

    this->suggestionBox.free();

    if(this->hasAst){
        ast_free(&this->ast);
        tokenlist_free(&this->preserveTokenlist);
        ::free(this->preserveBuffer);
    }

    if(this->astCreationThread.joinable()) this->astCreationThread.join();
}

void TextEditor::load(Settings *settings, Font *font, Texture *fontTexture, float maxWidth, float maxHeight){
    GenericEditor::load(settings, font, fontTexture, maxWidth, maxHeight);
    this->palette.generate(SyntaxColorPalette::VISUAL_STUDIO);
    this->richText.fileType = FileType::ADEPT;
    this->richText.setFont(this->font);
    this->hasLineNumbersModel = false;
    this->lineNumbersUpdated = true;
    this->hasFilenameModel = false;
    this->textWidth = 0.43f;
    this->textEdge = 0.275f;
    this->caret.generate(settings, this->font);
    this->caret.set(this->richText.text.length());
    this->xOffset = 4.0f;
    this->yOffset = 4.0f;
    this->selection = NULL;
    this->selectionStartCaret.generate(this->settings, this->font);
    this->selecting = false;
    this->scroll = 0;
    this->scrollXOffset = 0.0f;
    this->scrollYOffset = 0.0f;
    this->targetScrollXOffset = 0.0f;
    this->targetScrollYOffset = 0.0f;
    this->textXOffset = 48.0f;
    this->updateFilenameModel();

    this->suggestionBox.load(this->settings, this->font, this->fontTexture);
    this->showSuggestionBox = false;

    this->hasAst = false;
}

void TextEditor::render(Matrix4f& projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader){
    this->targetScrollYOffset = this->calculateScrollOffset();

    if(fabs(this->scrollXOffset - this->targetScrollXOffset) > 0.01f){
        this->scrollXOffset += (this->targetScrollXOffset > this->scrollXOffset ? 1 : -1) * fabs(this->scrollXOffset - this->targetScrollXOffset) * clampedHalfDelta(this->settings->hidden.delta);
    } else this->scrollXOffset = this->targetScrollXOffset;

    if(fabs(this->scrollYOffset - this->targetScrollYOffset) > 0.01f){
        this->scrollYOffset += (this->targetScrollYOffset > this->scrollYOffset ? 1 : -1) * fabs(this->scrollYOffset - this->targetScrollYOffset) * clampedHalfDelta(this->settings->hidden.delta);
    } else this->scrollYOffset = this->targetScrollYOffset;

    if(this->selection != NULL){
        this->selectionStartCaret.getTransformationMatrix(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->scrollYOffset, &transformationMatrix);

        fontShader->bind();
        fontShader->giveMatrix4f("projection_matrix", projectionMatrix);
        fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
        fontShader->giveFloat("width", this->textWidth);
        fontShader->giveFloat("edge", this->textEdge);
        this->selectionStartCaret.draw();

        Vector4f color(0.17, 0.19, 0.2, 1.0f);
        transformationMatrix.translateFromIdentity(this->xOffset + this->textXOffset, this->yOffset - this->scrollYOffset, -0.9f);

        solidShader->bind();
        solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
        solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
        solidShader->giveVector4f("color", color);
        this->selection->draw(this->richText.text, *this->font);
    }
    
    fontShader->bind();
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);

    if(this->lineNumbersUpdated){
        if(this->hasLineNumbersModel) this->lineNumbersModel.free();
        this->generateLineNumbersText();
        this->lineNumbersModel = this->font->generatePlainTextModel(this->lineNumbersText, 0.17f, Vector3f(0.5, 0.5, 0.5));
        this->hasLineNumbersModel = true;
        this->lineNumbersUpdated = false;
    }

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->fontTexture->getID());

    this->transformationMatrix.translateFromIdentity(this->xOffset + this->textXOffset, this->yOffset - this->scrollYOffset, 0.0f);
    fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    fontShader->giveFloat("width", this->textWidth);
    fontShader->giveFloat("edge", this->textEdge);

    this->richText.getTextModel()->draw();

    this->caret.getTransformationMatrix(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->scrollYOffset, &transformationMatrix);
    fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    this->caret.draw();

    for(Caret *additionalCaret : this->additionalCarets){
        additionalCaret->getTransformationMatrix(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->scrollYOffset, &transformationMatrix);
        fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
        additionalCaret->draw();
    }

    if(this->hasLineNumbersModel){
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, this->fontTexture->getID());

        this->transformationMatrix.translateFromIdentity(this->xOffset + 4.0f, this->yOffset - this->scrollYOffset, 0.0f);
        fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
        fontShader->giveFloat("width", this->textWidth);
        fontShader->giveFloat("edge", this->textEdge);

        this->lineNumbersModel.draw();
    }    

    if(this->showSuggestionBox){
        this->suggestionBox.render(projectionMatrix, fontShader, solidShader, this->caret.getX(), this->caret.getY() + this->font->line_height * 0.17f);
    }
}

TextModel* TextEditor::getFilenameModel(){
    if(!this->hasFilenameModel) this->updateFilenameModel();
    return &this->filenameModel;
}

FileType TextEditor::getFileType(){
    return this->richText.fileType;
}

void TextEditor::resize(float width, float height){
    this->maxWidth = width;
    this->maxHeight = height;
}

void TextEditor::setFileType(const FileType& type){
    this->richText.setFileType(type);
}

void TextEditor::setSyntaxColorPalette(const SyntaxColorPalette& palette){
    this->richText.setSyntaxColorPalette(palette);
}

void TextEditor::setSyntaxColorPalette(const SyntaxColorPalette::Defaults& presetPalette){
    this->richText.setSyntaxColorPalette(SyntaxColorPalette(presetPalette));
}

void TextEditor::updateFilenameModel(){
    if(this->hasFilenameModel) this->filenameModel.free();

    this->displayFilename = (this->filename == "") ? "Untitled" : filename_name(this->filename);

    if(this->displayFilename.length() >= 5 && this->displayFilename.substr(this->displayFilename.length() - 5, 5) == ".java"){
        this->richText.fileType = FileType::JAVA;
    } else if(this->displayFilename.length() >= 5 && this->displayFilename.substr(this->displayFilename.length() - 5, 5) == ".html"){
        this->richText.fileType = FileType::HTML;
    } else {
        this->richText.fileType = FileType::ADEPT;
    }
    
    this->filenameModel = this->font->generatePlainTextModel(this->displayFilename, 0.17f);
    this->hasFilenameModel = true;
}

void TextEditor::type(const std::string& characters){
    if(this->selection != NULL){
        this->deleteSelected();
    }

    this->relationallyIncreaseCaret(&this->caret, characters.length());
    this->richText.insert(this->caret.getPosition() - characters.length(), characters);

    for(Caret *additionalCaret : this->additionalCarets){
        this->relationallyIncreaseCaret(additionalCaret, characters.length());
        this->richText.insert(additionalCaret->getPosition() - characters.length(), characters);
    }

    if(std::count(characters.begin(), characters.end(), '\n') != 0){
        this->lineNumbersUpdated = true;
    }

    if(characters.length() != 0 && isIdentifier(characters[characters.length() - 1])){
        this->showSuggestionBox = true;
    } else {
        this->showSuggestionBox = false;
    }

    this->adjustViewForCaret();
}

void TextEditor::type(char character){
    if(this->selection != NULL){
        this->deleteSelected();
    }

    this->relationallyIncreaseCaret(&this->caret, 1);
    this->richText.insert(this->caret.getPosition() - 1, character);

    for(Caret *additionalCaret : this->additionalCarets){
        this->relationallyIncreaseCaret(additionalCaret, 1);
        this->richText.insert(additionalCaret->getPosition() - 1, character);
    }

    if(character == '\n'){
        this->lineNumbersUpdated = true;
    }

    if(this->settings->ide_suggestions && this->richText.fileType == FileType::ADEPT && isIdentifier(character)){
        this->showSuggestionBox = true;

        this->astMutex.lock();
        if(this->hasAst){
            size_t end = this->getCaretPosition();
            if(end != 0){
                size_t beginning = end - 1;

                while(true){
                    if(!isIdentifier(this->richText.text[beginning])){
                        beginning++;
                        break;
                    }

                    if(beginning == 0) break;
                    beginning--;
                }

                std::string last = this->richText.text.substr(beginning, end - beginning);

                this->symbolWeights.clear();

                std::string list;
                for(size_t i = 0; i != this->ast.funcs_length; i++){
                    const char *name = this->ast.funcs[i].name;
                    if(strlen(name) < last.length() || strncmp(name, last.c_str(), last.length()) != 0) continue;

                    std::string args;

                    for(size_t j = 0; j != this->ast.funcs[i].arity; j++){
                        char *a = ast_type_str(&this->ast.funcs[i].arg_types[j]);
                        args += std::string(a);
                        ::free(a);
                        if(j + 1 != this->ast.funcs[i].arity) args += ", ";
                    }

                    if(this->ast.funcs[i].traits & AST_FUNC_VARARG) args += ", ...";

                    char *r = ast_type_str(&this->ast.funcs[i].return_type);
                    std::string label = "f " + std::string(name) + "(" + args + ") " + r;
                    ::free(r);

                    this->symbolWeights.push_back(SymbolWeight(name, label, levenshtein(last.c_str(), name),  true));
                }
                for(size_t i = 0; i != this->ast.structs_length; i++){
                    const char *name = this->ast.structs[i].name;
                    if(strlen(name) < last.length() || strncmp(name, last.c_str(), last.length()) != 0) continue;
                    this->symbolWeights.push_back(SymbolWeight(name, "s " + std::string(name), levenshtein(last.c_str(), name), false));
                }
                for(size_t i = 0; i != this->ast.globals_length; i++){
                    //const char *name = this->ast.globals[i].name;
                    //if(strlen(name) < last.length() || strncmp(name, last.c_str(), last.length()) != 0) continue;
                    //weights.push_back(StringWeightPair(name, "g " + std::string(name), levenshtein(last.c_str(), name)));
                }
                std::sort(this->symbolWeights.begin(), this->symbolWeights.end());

                size_t lines = 0;
                size_t longest = 0;
                for(size_t i = 0; i != 5 && i != this->symbolWeights.size(); i++){
                    list += this->symbolWeights[i].label + "\n";
                    if(this->symbolWeights[i].label.length() > longest) longest = this->symbolWeights[i].label.length();
                    lines++;
                }
                
                this->suggestionBox.generate(list, lines, longest);
                if(lines == 0) this->showSuggestionBox = false;
            }
        }
        this->astMutex.unlock();
    } else {
        this->showSuggestionBox = false;
        this->symbolWeights.clear();
    }

    this->adjustViewForCaret();

    global_background_input.mutex.lock();
    global_background_input.text = this->richText.text;
    global_background_input.filename = this->filename;
    global_background_input.mutex.unlock();
    global_background_input.updated.store(true);
}

void TextEditor::typeBlock(){
    size_t whitespaceCount = this->caret.countLeadingWhitespace(this->richText.text);

    std::string initialWhitespace(whitespaceCount, ' ');

    this->type("{\n" + initialWhitespace + "    \n" + initialWhitespace + "}");
    this->moveCaretUp();
    this->moveCaretEndOfLine();
}

void TextEditor::typeExpression(){
    this->type("()");
    this->moveCaretLeft();
}

void TextEditor::typeArrayAccess(){
    this->type("[]");
    this->moveCaretLeft();
}

void TextEditor::typeString(){
    this->type("\"\"");
    this->moveCaretLeft();
}

void TextEditor::typeCString(){
    this->type("\'\'");
    this->moveCaretLeft();
}

void TextEditor::backspace(){
    this->backspaceForCaret(&this->caret);

    for(Caret *additionalCaret : this->additionalCarets)
        this->backspaceForCaret(additionalCaret);

    this->adjustViewForCaret();
}

void TextEditor::backspaceForCaret(Caret *caret){
    // WARNING: caret is not the same as this->caret

    size_t i = caret->getPosition();

    if(this->selection != NULL){
        this->deleteSelected();
        //this->textUpdated = true;
        return;
    }

    if(i >= 4 && this->richText.text.substr(i - 4, 4) == "    "){
        std::string deleted = richText.text.substr(i - 4, 4);

        if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
            this->lineNumbersUpdated = true;
        }

        this->richText.remove(i - 4, 4);
        this->relationallyDecreaseCaret(caret, 4);
    } else if(i > 0){
        std::string deleted = richText.text.substr(i - 1, 1);

        if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
            this->lineNumbersUpdated = true;
        }

        this->richText.remove(i - 1, 1);
        this->relationallyDecreaseCaret(caret, 1);
    }
}

void TextEditor::del(){
    size_t i = this->caret.getPosition();

    if(this->selection != NULL){
        this->deleteSelected();
        return;
    }

    if(i + 4 < richText.text.length() && richText.text.substr(i, 4) == "    "){
        std::string deleted = richText.text.substr(i, 4);
        
        if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
            this->lineNumbersUpdated = true;
        }

        richText.remove(i, 4);
    } else if(i < richText.text.length()){
        std::string deleted = richText.text.substr(i, 1);
        
        if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
            this->lineNumbersUpdated = true;
        }

        richText.remove(i, 1);
    }
}

void TextEditor::smartBackspace(){
    if(this->selection != NULL){
        this->backspace(); return;
    }

    if(this->smartRemove()) return;

    // If nothing special to do, just do a normal backspace
    this->backspace();
}

void TextEditor::smartDel(){
    if(this->selection != NULL){
        this->del(); return;
    }

    if(this->smartRemove()) return;

    // If nothing special to do, just do a normal del
    this->del();
}

bool TextEditor::smartRemove(){
    // Performs a smart remove operation if possible.
    // Returns whether an operator was performed
 
    size_t i = this->caret.getPosition();

    if(i > 0 && i < this->richText.text.length()){
        if((richText.text[i - 1] == '(' && richText.text[i] == ')')
        || (richText.text[i - 1] == '[' && richText.text[i] == ']')
        || (richText.text[i - 1] == '{' && richText.text[i] == '}')){
            richText.remove(i - 1, 2);
            this->caret.decrease(1);
            this->adjustViewForCaret();
            return true;
        }
    }

    return false;
}

void TextEditor::backspaceLine(){
    size_t beginning = this->caret.getLineBeginning(this->richText.text);
    size_t end = this->caret.getLineEndAfterNewline(this->richText.text);
    this->deleteAdditionalCarets();
    this->destroySelection();

    std::string deleted = this->richText.text.substr(beginning, end - beginning);
    if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
        this->lineNumbersUpdated = true;
    }

    this->richText.remove(beginning, end - beginning);
    this->caret.set(beginning);

    this->moveCaretUp();
    this->moveCaretEndOfLine();

    this->adjustViewForCaret();
}

void TextEditor::delLine(){
    size_t beginning = this->caret.getLineBeginning(this->richText.text);
    size_t end = this->caret.getLineEndAfterNewline(this->richText.text);
    this->deleteAdditionalCarets();
    this->destroySelection();

    std::string deleted = this->richText.text.substr(beginning, end - beginning);    
    if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
        this->lineNumbersUpdated = true;
    }

    this->richText.remove(beginning, end - beginning);
    this->caret.set(beginning);

    this->moveCaretEndOfLine();
}

void TextEditor::startSelection(bool snapSelectionCaret){
    if(!this->selecting){
        if(this->selection) this->destroySelection();

        this->selecting = true;
        this->selection = new Selection(this->caret.getPosition(), this->caret.getPosition());
        this->selectionStartCaret.set(this->caret.getPosition());
        if(snapSelectionCaret){
            this->selectionStartCaret.snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());
        }
    }
}

void TextEditor::endSelection(){
    this->selecting = false;

    if(this->selection && this->selection->getBeginning() == this->selection->getEnd()){
        this->destroySelection();
    }
}

void TextEditor::destroySelection(){
    delete this->selection;
    this->selection = NULL;
    this->selecting = false;
}

void TextEditor::deleteSelected(){
    if(this->selection == NULL) return;
    this->deleteRange(this->selection->getBeginning(), this->selection->getEnd());
}

bool TextEditor::hasSelection(){
    return this->selection;
}

void TextEditor::deleteRange(size_t beginning, size_t end){
    if(beginning == end) return;
    this->deleteAdditionalCarets();

    // Ensure that beginning is the lesser position
    if(beginning > end) std::swap(beginning, end);

    std::string deleted = this->richText.text.substr(beginning, end - beginning);
    
    if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
        this->lineNumbersUpdated = true;
    }

    this->richText.remove(beginning, end - beginning);
    this->destroySelection();
    if(this->caret.getPosition() == end){
        this->relationallyDecreaseCaret(&this->caret, end - beginning);
    } else {
        this->relationallyMaintainDecrease(&this->caret, end - beginning);
    }
}

void TextEditor::copySelected(GLFWwindow *window){
    if(this->selection == NULL) return;

    size_t beginning = this->selection->getBeginning();
    size_t end = this->selection->getEnd();

    if(beginning == end) return;

    // Ensure that beginning is the lesser position
    if(beginning > end) std::swap(beginning, end);

    std::string copied = this->richText.text.substr(beginning, end - beginning);
    glfwSetClipboardString(window, copied.c_str());
}

void TextEditor::cutSelected(GLFWwindow *window){
    this->copySelected(window);
    this->deleteSelected();
}

void TextEditor::paste(GLFWwindow *window){
    this->type(std::string(glfwGetClipboardString(window)));
}

void TextEditor::tab(){
    size_t home = this->caret.getAfterWhitespaceInLine(this->richText.text, this->caret.getLineBeginning(this->richText.text));

    if(this->caret.getPosition() <= home)
        this->type("    ");
    else
        this->moveCaretOutside();
}

void TextEditor::nextLine(){
    size_t whitespaceCount = this->caret.countLeadingWhitespace(this->richText.text);

    size_t position = this->caret.getPosition();
    if(position - 1 >= 0 && (this->richText.text[position - 1] == '{' || this->richText.text[position - 1] == ',')){
        whitespaceCount += 4;
    }

    std::string initialWhitespace(whitespaceCount, ' ');
    this->moveCaretEndOfLine();
    this->type("\n" + initialWhitespace);
}

void TextEditor::nextPrecedingLine(){
    size_t whitespaceCount = this->caret.countLeadingWhitespace(this->richText.text);

    std::string initialWhitespace(whitespaceCount, ' ');
    this->moveCaretBeginningOfLine();
    this->type("\n" + initialWhitespace);
    this->moveCaretUp();
    this->moveCaretEndOfLine();
}

void TextEditor::finishSuggestion(){
    if(this->symbolWeights.size() == 0) return;

    const std::string& target = this->symbolWeights[0].name;

    size_t end = this->getCaretPosition();
    size_t beginning = end;
    if(end != 0){
        beginning--;

        while(true){
            if(!isIdentifier(this->richText.text[beginning])){
                beginning++;
                break;
            }

            if(beginning == 0) break;
            beginning--;
        }
    }

    for(size_t i = end - beginning; i < target.length(); i++){
        this->type(target[i]);
    }

    if(this->symbolWeights[0].isFunction){
        this->type("()");
        this->moveCaretLeft();
    }
}

void TextEditor::moveCaretToPosition(size_t position){
    this->deleteAdditionalCarets();
    this->caret.set(position);
    this->adjustViewForCaret();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaret(double xpos, double ypos){
    this->deleteAdditionalCarets();

    int row, column;
    this->getRowAndColumnAt(xpos, ypos, &row, &column);
    this->caret.moveTo(this->richText.text, row, column);
    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretLeft(){
    this->caret.moveLeft(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveLeft(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretRight(){
    this->caret.moveRight(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveRight(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretUp(){
    this->caret.moveUp(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveUp(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretDown(){
    this->caret.moveDown(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveDown(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretBeginningOfLine(){
    this->caret.moveBeginningOfLine(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveBeginningOfLine(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretEndOfLine(){
    this->caret.moveEndOfLine(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveEndOfLine(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretBeginningOfWord(){
    this->caret.moveBeginningOfWord(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveBeginningOfWord(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretEndOfWord(){
    this->caret.moveEndOfWord(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveEndOfWord(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretBeginningOfSubWord(){
    this->caret.moveBeginningOfSubWord(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveBeginningOfSubWord(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretEndOfSubWord(){
    this->caret.moveEndOfSubWord(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveEndOfSubWord(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretOutside(){
    this->caret.moveOutside(this->richText.text);

    for(Caret *additionalCaret : this->additionalCarets)
        additionalCaret->moveOutside(this->richText.text);

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::scrollDown(int lineCount){
    this->scroll += lineCount;

    size_t newlines = std::count(this->richText.text.begin(), this->richText.text.end(), '\n');
    if(this->scroll > (int) newlines) this->scroll = newlines;
}

void TextEditor::scrollUp(int lineCount){
    this->scroll -= lineCount;

    if(this->scroll < 0) this->scroll = 0;
}

void TextEditor::pageUp(){
    for(size_t i = 0; i != 30; i++) this->moveCaretUp();
    this->showSuggestionBox = false;
}

void TextEditor::pageDown(){
    for(size_t i = 0; i != 30; i++) this->moveCaretDown();
    this->showSuggestionBox = false;
}

void TextEditor::adjustViewForCaret(){
    if(this->maxHeight < 0) return;

    size_t currentLine = this->caret.getLine(this->richText.text);
    size_t linesViewable = (size_t) (this->maxHeight / this->calculateScrollOffset(1));
    size_t maxSeenLine = linesViewable + this->scroll;

    if(this->calculateScrollOffset(currentLine - 1) < this->calculateScrollOffset()){
        this->scroll = currentLine - 1;
    }
    if(currentLine > maxSeenLine){
        this->scroll = currentLine - linesViewable;
    }
}

void TextEditor::selectAll(){
    this->deleteAdditionalCarets();

    if(this->richText.text.length() != 0){
        this->selectionStartCaret.set(this->caret.getPosition());
        this->selectionStartCaret.snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());

        this->caret.set(this->richText.text.length());
        this->startSelection(false);
        this->caret.set(0);
        this->selection->adjustEnd(this->caret.getPosition());
        this->endSelection();
    }
}

void TextEditor::selectLine(){
    this->deleteAdditionalCarets();

    if(this->richText.text.length() != 0){
        size_t lineHome = this->caret.getAfterWhitespaceInLine(this->richText.text, this->caret.getLineBeginning(this->richText.text));
        size_t lineEnd = this->caret.getLineEnd(this->richText.text);

        if(lineHome == lineEnd) return;

        this->selectionStartCaret.set(this->caret.getPosition());
        this->selectionStartCaret.snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());

        this->caret.set(lineEnd);
        this->startSelection(false);
        this->caret.set(lineHome);
        this->selection->adjustEnd(this->caret.getPosition());
        this->endSelection();
    }
}

void TextEditor::relationallyIncreaseCaret(Caret *caret, size_t amount){
    // WARNING: caret is not the same as this->caret

    if(caret != &this->caret && this->caret.getPosition() > caret->getPosition()){
        this->caret.increase(amount);
    }

    for(Caret *additionalCaret : this->additionalCarets){
        if(caret != additionalCaret && additionalCaret->getPosition() > caret->getPosition()){
            additionalCaret->increase(amount);
        }
    }

    caret->increase(amount);
}

void TextEditor::relationallyDecreaseCaret(Caret *caret, size_t amount){
    // WARNING: caret is not the same as this->caret

    if(caret != &this->caret && this->caret.getPosition() >= caret->getPosition()){
        this->caret.decrease(amount);
    }

    for(Caret *additionalCaret : this->additionalCarets){
        if(caret != additionalCaret && additionalCaret->getPosition() >= caret->getPosition()){
            additionalCaret->decrease(amount);
        }
    }

    caret->decrease(amount);
}

void TextEditor::relationallyMaintainIncrease(Caret *caret, size_t amount){
    // WARNING: caret is not the same as this->caret

    if(caret != &this->caret && this->caret.getPosition() > caret->getPosition()){
        this->caret.increase(amount);
    }

    for(Caret *additionalCaret : this->additionalCarets){
        if(caret != additionalCaret && additionalCaret->getPosition() > caret->getPosition()){
            additionalCaret->increase(amount);
        }
    }
}

void TextEditor::relationallyMaintainDecrease(Caret *caret, size_t amount){
    // WARNING: caret is not the same as this->caret

    if(caret != &this->caret && this->caret.getPosition() > caret->getPosition()){
        this->caret.decrease(amount);
    }

    for(Caret *additionalCaret : this->additionalCarets){
        if(caret != additionalCaret && additionalCaret->getPosition() > caret->getPosition()){
            additionalCaret->decrease(amount);
        }
    }
}

void TextEditor::duplicateCaretUp(){
    Caret *newCaret = new Caret();
    newCaret->generate(this->settings, this->font);
    newCaret->set(this->caret.getPosition());
    newCaret->snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());
    caret.moveUp(this->richText.text);
    this->additionalCarets.push_back(newCaret);
}

void TextEditor::duplicateCaretDown(){
    Caret *newCaret = new Caret();
    newCaret->generate(this->settings, this->font);
    newCaret->set(this->caret.getPosition());
    newCaret->snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());
    caret.moveDown(this->richText.text);
    this->additionalCarets.push_back(newCaret);
}

void TextEditor::deleteAdditionalCarets(){
    for(Caret *additionalCaret : this->additionalCarets){
        delete additionalCaret;
    }

    this->additionalCarets.resize(0);
}

void TextEditor::setOffset(float xOffset, float yOffset){
    this->xOffset = xOffset;
    this->yOffset = yOffset;
}

float TextEditor::getNetXOffset(){
    return this->xOffset + this->textXOffset + this->scrollXOffset;
}

float TextEditor::getNetYOffset(){
    return this->yOffset + this->scrollYOffset;
}

void TextEditor::loadTextFromFile(const std::string& filename){
    this->destroySelection();
    
    this->richText.setFont(this->font);
    this->richText.loadFromFile(filename);
    this->makeAst();

    this->caret.set(0);
    this->scroll = 0;
    this->lineNumbersUpdated = true;
}

#include "UTIL/util.h"
#include "INSIGHT/insight.h"

// src/INSIGHT/include/
#include "UTIL/filename.h"

void TextEditor::saveFile(){
    std::ofstream out(this->filename);

    if(!out.is_open()) { puts("oops"); return; }

    out << this->richText.text;
    out.close();

    this->makeAst();
}

void TextEditor::getRowAndColumnAt(double xpos, double ypos, int *out_row, int *out_column){
    xpos -= this->xOffset - (this->font->mono_character_width * 0.17f * 0.5f) + this->textXOffset;
    ypos -= this->yOffset - this->calculateScrollOffset();

    *out_row = floor(ypos / (this->font->line_height * 0.17f)) + 1;
    *out_column = floor(xpos / (this->font->mono_character_width * 0.17f)) + 1;
}

void TextEditor::handleSelection(){
    if(!this->selecting && this->selection != NULL){
        this->destroySelection();
    } else if(this->selection != NULL){
        this->selection->adjustEnd(this->caret.getPosition());
    }
}

float TextEditor::calculateScrollOffset(){
    return this->calculateScrollOffset(this->scroll);
}

float TextEditor::calculateScrollOffset(size_t line){
    return (this->font->line_height * 0.17f) * line;
}

void TextEditor::generateLineNumbersText(){
    this->lineNumbersText = "";

    int newlines = std::count(this->richText.text.begin(), this->richText.text.end(), '\n');
    if(newlines < 0) return;

    for(int i = 0; i - 1 != newlines; i++){
        std::string number = to_str(i + 1) + "\n";

        if(number.size() < 5){
            number.insert(0, 5 - number.size(), ' ');
        }

        this->lineNumbersText.append(number);
    }
}

void TextEditor::makeAst(){
    if(this->richText.fileType == FileType::ADEPT){
        if(this->astCreationThread.joinable()) this->astCreationThread.join();

        this->astCreationThread = std::thread([this] (std::string filename, std::string adept_root) {
            this->astMutex.lock();
            insight_buffer_index = 0;
            insight_buffer[0] = '\0';
            compiler_t compiler;
            compiler_init(&compiler);
            object_t *object = compiler_new_object(&compiler);
            object->filename = strclone(filename.c_str());
            object->full_filename = filename_absolute(object->filename);
            compiler.root = strclone(adept_root.c_str());
            if(lex(&compiler, object)){
                puts("Failed to lex");
                compiler_free(&compiler);
                this->astMutex.unlock();
                return;
            }

            if(parse(&compiler, object)){
                puts("Failed to parse");
                compiler_free(&compiler);
                this->astMutex.unlock();
                return;
            }

            if(this->hasAst){
                ast_free(&this->ast);
                tokenlist_free(&this->preserveTokenlist);
                ::free(this->preserveBuffer);
            }

            this->ast = object->ast;
            object->compilation_stage = COMPILATION_STAGE_FILENAME;
            this->preserveTokenlist = object->tokenlist;
            this->preserveBuffer = object->buffer;
            this->hasAst = true;
            compiler_free(&compiler);
            this->astMutex.unlock();
        }, this->filename, this->settings->adept_root);
    }
}

size_t TextEditor::getCaretPosition(){
    return this->caret.getPosition();
}

TextEditor* TextEditor::asTextEditor(){
    return this;
}

ImageEditor* TextEditor::asImageEditor(){
    return NULL;
}

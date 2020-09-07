
#include <math.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>

#include "UTIL/strings.h"
#include "UTIL/lexical.h"
#include "UTIL/document.h"
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
    if(this->insightThread.joinable()) this->insightThread.join();

    if(this->hasCompiler) compiler_free(&this->compiler);
    if(this->error) adept_error_free_fully(this->error);
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
    this->mainCaret.generate(settings, this->font);
    this->mainCaret.set(this->richText.text.length());
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

    this->insightRunning = false;
    this->hasCompiler = false;
    this->insightCreationResult = InsightCreationResultNothingNew;
    this->lastPassiveInsightUpdate = 0.0;
    this->error = NULL;
    this->lastChanged = 0.0;
}

void TextEditor::render(Matrix4f& projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets){
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

    this->insightMutex.lock();

    if(glfwGetTime() - this->lastChanged > 0.5){
        // Show error underline
        if(this->error && this->error->source.object_index == 0 && this->settings->ide_error_underline){
            float startX, startY;
            Caret::getTargetCoords(this->error->source.index, this->richText.text, *this->font, this->xOffset + this->textXOffset, this->yOffset - this->scrollYOffset, &startX, &startY);
            
            solidShader->bind();
            transformationMatrix.translateFromIdentity(startX, startY + this->font->line_height * FONT_SCALE * 0.8, -0.89f);
            transformationMatrix.scale(this->error->source.stride * this->font->mono_character_width * FONT_SCALE, 2.0f, 1.0f);
            solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
            solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
            solidShader->giveVector4f("color", Vector4f(1.00, 0.00, 0.20, 1.00));

            assets->singlePixelModel->draw();
        }

        // Show warning underlines
        if(this->settings->ide_warning_underline) for(length_t i = 0; i != this->warnings_length; i++){
            adept_warning_t *warning = &this->warnings[i];

            // Ignore if not in this file
            if(warning->source.object_index != 0) continue;

            float startX, startY;
            Caret::getTargetCoords(warning->source.index, this->richText.text, *this->font, this->xOffset + this->textXOffset, this->yOffset - this->scrollYOffset, &startX, &startY);
            
            solidShader->bind();
            transformationMatrix.translateFromIdentity(startX, startY + this->font->line_height * FONT_SCALE * 0.8, -0.89f);
            transformationMatrix.scale(warning->source.stride * this->font->mono_character_width * FONT_SCALE, 2.0f, 1.0f);
            solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
            solidShader->giveMatrix4f("transformation_matrix", transformationMatrix);
            solidShader->giveVector4f("color", Vector4f(1.00, 0.80, 0.00, 1.00));

            assets->singlePixelModel->draw();
        }
    }

    this->insightMutex.unlock();
    
    fontShader->bind();
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);

    if(this->lineNumbersUpdated){
        if(this->hasLineNumbersModel) this->lineNumbersModel.free();
        this->generateLineNumbersText();
        this->lineNumbersModel = this->font->generatePlainTextModel(this->lineNumbersText, FONT_SCALE, Vector3f(0.5, 0.5, 0.5));
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

    this->mainCaret.getTransformationMatrix(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->scrollYOffset, &transformationMatrix);
    fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    this->mainCaret.draw();

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
        this->suggestionBox.render(projectionMatrix, shader, fontShader, solidShader, assets, this->mainCaret.getX(), this->mainCaret.getY() + this->font->line_height * FONT_SCALE);
    }
}

TextModel* TextEditor::getFilenameModel(){
    if(!this->hasFilenameModel) this->updateFilenameModel();
    return &this->filenameModel;
}

size_t TextEditor::getDisplayFilenameLength(){
    if(!this->hasFilenameModel) this->updateFilenameModel();
    return this->displayFilename.length();
}

FileType TextEditor::getFileType(){
    return this->richText.fileType;
}

void TextEditor::resize(float width, float height){
    this->maxWidth = width;
    this->maxHeight = height;
}

void TextEditor::snapCaretToPosition(float x, float y){
    this->mainCaret.snapToPosition(x, y);
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

    if(string_ends_with(this->displayFilename, ".adept")){
        this->richText.fileType = FileType::ADEPT;
    } else if(string_ends_with(this->displayFilename, ".java")){
        this->richText.fileType = FileType::JAVA;
    } else if(string_ends_with(this->displayFilename, ".html")){
        this->richText.fileType = FileType::HTML;
    } else if(string_ends_with(this->displayFilename, ".json")){
        this->richText.fileType = FileType::JSON;
    } else {
        this->richText.fileType = FileType::ADEPT;
    }
    
    this->filenameModel = this->font->generatePlainTextModel(this->displayFilename, FONT_SCALE);
    this->hasFilenameModel = true;
}

void TextEditor::type(const std::string& characters){
    this->onTextChange();

    if(this->selection != NULL){
        this->deleteSelected();
    }

    if(this->additionalCarets.size() != 0)
        this->changeRecord.startGroup();

    this->changeRecord.addInsertion(this->mainCaret.getPosition(), characters);
    this->relationallyIncreaseCaret(&this->mainCaret, characters.length());
    this->richText.insert(this->mainCaret.getPosition() - characters.length(), characters);

    for(Caret *additionalCaret : this->additionalCarets){
        this->changeRecord.addInsertion(additionalCaret->getPosition(), characters);
        this->relationallyIncreaseCaret(additionalCaret, characters.length());
        this->richText.insert(additionalCaret->getPosition() - characters.length(), characters);
    }

    if(this->additionalCarets.size() != 0)
        this->changeRecord.endGroup();

    if(std::count(characters.begin(), characters.end(), '\n') != 0){
        this->lineNumbersUpdated = true;
        this->showSuggestionBox = false;
        this->suggestionBox.clearIfFileSuggestions();
    }

    if(this->settings->ide_suggestions && this->richText.fileType == FileType::ADEPT && characters.length() != 0 && isIdentifier(characters[characters.length() - 1])){
        this->showSuggestionBox = true;
        this->generateSuggestions();
    }

    this->adjustViewForCaret();
}

void TextEditor::type(char character){
    this->type(std::string(1, character));
}

void TextEditor::typeBlock(){
    size_t whitespaceCount = this->mainCaret.countLeadingWhitespace(this->richText.text);

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
    this->forEachCaret([this](Caret *caret){ this->backspaceForCaret(caret); });
    this->adjustViewForCaret();
}

void TextEditor::backspaceForCaret(Caret *caret){
    // WARNING: caret is not the same as this->caret

    this->onTextChange();

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

        this->changeRecord.addDeletion(i - 4, deleted, true);
        this->richText.remove(i - 4, 4);
        this->relationallyDecreaseCaret(caret, 4);
    } else if(i > 0){
        std::string deleted = richText.text.substr(i - 1, 1);

        if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
            this->lineNumbersUpdated = true;
        }

        this->changeRecord.addDeletion(i - 1, deleted, true);
        this->richText.remove(i - 1, 1);
        this->relationallyDecreaseCaret(caret, 1);
    }
}

void TextEditor::del(){
    this->onTextChange();
    size_t i = this->mainCaret.getPosition();

    if(this->selection != NULL){
        this->deleteSelected();
        return;
    }

    if(i + 4 < richText.text.length() && richText.text.substr(i, 4) == "    "){
        std::string deleted = richText.text.substr(i, 4);
        
        if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
            this->lineNumbersUpdated = true;
        }

        this->changeRecord.addDeletion(i, deleted, false);
        richText.remove(i, 4);
    } else if(i < richText.text.length()){
        std::string deleted = richText.text.substr(i, 1);
        
        if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
            this->lineNumbersUpdated = true;
        }

        this->changeRecord.addDeletion(i, deleted, false);
        richText.remove(i, 1);
    }
}

void TextEditor::smartBackspace(){
    this->onTextChange();

    if(this->selection != NULL){
        this->backspace(); return;
    }

    if(this->smartRemove()) return;

    // If nothing special to do, just do a normal backspace
    this->backspace();
}

void TextEditor::smartDel(){
    this->onTextChange();

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

    this->onTextChange();
 
    size_t i = this->mainCaret.getPosition();

    if(i > 0 && i < this->richText.text.length()){
        if((richText.text[i - 1] == '(' && richText.text[i] == ')')
        || (richText.text[i - 1] == '[' && richText.text[i] == ']')
        || (richText.text[i - 1] == '{' && richText.text[i] == '}')){
            this->changeRecord.addDeletion(i - 1, this->richText.text.substr(i - 1, 2), false);
            this->richText.remove(i - 1, 2);
            this->mainCaret.decrease(1);
            this->adjustViewForCaret();
            return true;
        }
    }

    return false;
}

void TextEditor::backspaceLine(){
    this->onTextChange();

    size_t beginning = this->mainCaret.getLineBeginning(this->richText.text);
    size_t end = this->mainCaret.getLineEndAfterNewline(this->richText.text);
    this->deleteAdditionalCarets();
    this->destroySelection();

    std::string deleted = this->richText.text.substr(beginning, end - beginning);
    if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
        this->lineNumbersUpdated = true;
    }

    this->changeRecord.addDeletion(beginning, this->richText.text.substr(beginning, end - beginning), true);
    this->richText.remove(beginning, end - beginning);
    this->mainCaret.set(beginning);

    this->moveCaretUp();
    this->moveCaretEndOfLine();

    this->adjustViewForCaret();
}

void TextEditor::delLine(){
    this->onTextChange();

    size_t beginning = this->mainCaret.getLineBeginning(this->richText.text);
    size_t end = this->mainCaret.getLineEndAfterNewline(this->richText.text);
    this->deleteAdditionalCarets();
    this->destroySelection();

    std::string deleted = this->richText.text.substr(beginning, end - beginning);    
    if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
        this->lineNumbersUpdated = true;
    }

    this->changeRecord.addDeletion(beginning, this->richText.text.substr(beginning, end - beginning), false);
    this->richText.remove(beginning, end - beginning);
    this->mainCaret.set(beginning);

    this->moveCaretEndOfLine();
}

void TextEditor::startSelection(bool snapSelectionCaret){
    if(!this->selecting){
        if(this->selection) this->destroySelection();

        this->selecting = true;
        this->selection = new Selection(this->mainCaret.getPosition(), this->mainCaret.getPosition());
        this->selectionStartCaret.set(this->mainCaret.getPosition());
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

    this->onTextChange();
    this->deleteAdditionalCarets();

    // Ensure that beginning is the lesser position
    if(beginning > end) std::swap(beginning, end);

    std::string deleted = this->richText.text.substr(beginning, end - beginning);
    
    if(std::count(deleted.begin(), deleted.end(), '\n') != 0){
        this->lineNumbersUpdated = true;
    }

    this->changeRecord.addDeletion(beginning, this->richText.text.substr(beginning, end - beginning), true);
    this->richText.remove(beginning, end - beginning);
    this->destroySelection();

    if(this->mainCaret.getPosition() == end){
        this->relationallyDecreaseCaret(&this->mainCaret, end - beginning);
    } else {
        this->relationallyMaintainDecrease(&this->mainCaret, end - beginning);
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
    const char *clipboard = glfwGetClipboardString(window);
    if(clipboard) this->type(std::string(clipboard));
}

void TextEditor::tab(){
    size_t home = this->mainCaret.getAfterWhitespaceInLine(this->richText.text, this->mainCaret.getLineBeginning(this->richText.text));

    if(this->mainCaret.getPosition() <= home)
        this->type("    ");
    else
        this->moveCaretOutside();
}

void TextEditor::nextLine(){
    size_t whitespaceCount = this->mainCaret.countLeadingWhitespace(this->richText.text);

    size_t position = this->mainCaret.getPosition();
    if(position - 1 >= 0 && (this->richText.text[position - 1] == '{' || this->richText.text[position - 1] == ',')){
        whitespaceCount += 4;
    }

    std::string initialWhitespace(whitespaceCount, ' ');
    this->moveCaretEndOfLine();
    this->type("\n" + initialWhitespace);
}

void TextEditor::nextPrecedingLine(){
    size_t whitespaceCount = this->mainCaret.countLeadingWhitespace(this->richText.text);

    std::string initialWhitespace(whitespaceCount, ' ');
    this->moveCaretBeginningOfLine();
    this->type("\n" + initialWhitespace);
    this->moveCaretUp();
    this->moveCaretEndOfLine();
}

void TextEditor::finishSuggestion(){
    if(this->suggestionBox.symbolWeights.size() == 0) return;

    const std::string& target = this->suggestionBox.symbolWeights[0].name;

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

    if(this->suggestionBox.symbolWeights[0].kind == SymbolWeight::Kind::FUNCTION){
        this->type("()");
        this->moveCaretLeft();
    }
}

void TextEditor::maybeUpdatePassiveInsight(){
    if(this->getFileType() != FileType::ADEPT) return;

    if(!this->insightRunning && glfwGetTime() > this->lastPassiveInsightUpdate + this->settings->insight_passive_rate){
        this->makeInsight(false, true, false);
        this->lastPassiveInsightUpdate = glfwGetTime();
    }
}

void TextEditor::moveCaretToPosition(size_t position){
    this->deleteAdditionalCarets();
    this->mainCaret.set(position);
    this->adjustViewForCaret();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaret(double xpos, double ypos){
    this->deleteAdditionalCarets();

    int row, column;
    this->getRowAndColumnAt(xpos, ypos, &row, &column);
    this->mainCaret.moveTo(this->richText.text, row, column);
    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretLeft(){
    this->forEachCaret([this](Caret *caret){
        caret->moveLeft(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretRight(){
    this->forEachCaret([this](Caret *caret){
        caret->moveRight(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretUp(){
    this->forEachCaret([this](Caret *caret){
        caret->moveUp(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretDown(){
    this->forEachCaret([this](Caret *caret){
        caret->moveDown(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretBeginningOfLine(){
    this->forEachCaret([this](Caret *caret){
        caret->moveBeginningOfLine(this->richText.text);
    });
    
    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretEndOfLine(){
    this->forEachCaret([this](Caret *caret){
        caret->moveEndOfLine(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretBeginningOfWord(){
    this->forEachCaret([this](Caret *caret){
        caret->moveBeginningOfWord(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretEndOfWord(){
    this->forEachCaret([this](Caret *caret){
        caret->moveEndOfWord(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretBeginningOfSubWord(){
    this->forEachCaret([this](Caret *caret){
        caret->moveBeginningOfSubWord(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretEndOfSubWord(){
    this->forEachCaret([this](Caret *caret){
        caret->moveEndOfSubWord(this->richText.text);
    });

    this->adjustViewForCaret();
    this->handleSelection();
    this->showSuggestionBox = false;
}

void TextEditor::moveCaretOutside(){
    this->forEachCaret([this](Caret *caret){
        caret->moveOutside(this->richText.text);
    });

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

    size_t currentLine = this->mainCaret.getLine(this->richText.text);
    size_t linesViewable = (size_t) (this->maxHeight / this->calculateScrollOffset(1));
    size_t maxSeenLine = linesViewable + this->scroll;

    if(this->calculateScrollOffset(currentLine - 1) < this->calculateScrollOffset()){
        this->scroll = currentLine - 1;
    }
    if(currentLine > maxSeenLine){
        this->scroll = currentLine - linesViewable;
    }
}

void TextEditor::focusViewForCaret(){
    if(this->maxHeight < 0) return;

    size_t currentLine = this->mainCaret.getLine(this->richText.text);
    size_t linesViewable = (size_t) (this->maxHeight / this->calculateScrollOffset(1));

    this->scroll = currentLine - 1 - linesViewable / 2;
    if(this->scroll < 0) this->scroll = 0;
    
    long long totalLines = std::count(this->richText.text.begin(), this->richText.text.end(), '\n') + 1;
    if(totalLines > (long long) linesViewable && this->scroll > totalLines - (long long) linesViewable) this->scroll = totalLines - (long long) linesViewable;
}

void TextEditor::selectAll(){
    this->deleteAdditionalCarets();

    if(this->richText.text.length() != 0){
        this->selectionStartCaret.set(this->mainCaret.getPosition());
        this->selectionStartCaret.snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());

        this->mainCaret.set(this->richText.text.length());
        this->startSelection(false);
        this->mainCaret.set(0);
        this->selection->adjustEnd(this->mainCaret.getPosition());
        this->endSelection();
    }
}

void TextEditor::selectLine(){
    this->deleteAdditionalCarets();

    if(this->richText.text.length() != 0){
        size_t lineHome = this->mainCaret.getAfterWhitespaceInLine(this->richText.text, this->mainCaret.getLineBeginning(this->richText.text));
        size_t lineEnd = this->mainCaret.getLineEnd(this->richText.text);

        if(lineHome == lineEnd) return;

        this->selectionStartCaret.set(this->mainCaret.getPosition());
        this->selectionStartCaret.snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());

        this->mainCaret.set(lineEnd);
        this->startSelection(false);
        this->mainCaret.set(lineHome);
        this->selection->adjustEnd(this->mainCaret.getPosition());
        this->endSelection();
    }
}

void TextEditor::relationallyIncreaseCaret(Caret *targetCaret, size_t amount){
    // Moves targetCaret ahead 'amount' and retains other carets positions
    this->relationallyMaintainIncrease(targetCaret, amount);
    targetCaret->increase(amount);
}

void TextEditor::relationallyDecreaseCaret(Caret *targetCaret, size_t amount){
    // Moves targetCaret back 'amount' and retains other carets positions
    this->relationallyMaintainDecrease(targetCaret, amount);
    targetCaret->decrease(amount);
}

void TextEditor::relationallyMaintainIncrease(Caret *targetCaret, size_t amount){
    // Retains other carets positions when targetCaret moves ahead 'amount'
    this->forEachCaret([targetCaret, amount](Caret *caret){
        if(caret != targetCaret && caret->getPosition() > targetCaret->getPosition())
            caret->increase(amount);
    });
}

void TextEditor::relationallyMaintainDecrease(Caret *targetCaret, size_t amount){
    // Retains other carets positions when targetCaret moves back 'amount'
    // NOTE: '>=' is used instead of '>' like in relationallyMaintainIncrease in
    // order to prevent carets being left behind
    this->forEachCaret([targetCaret, amount](Caret *caret){
        if(caret != targetCaret && caret->getPosition() >= targetCaret->getPosition())
            caret->decrease(amount);
    });
}

void TextEditor::gotoLine(int lineNumber){
    if(lineNumber < 1) return;

    int number = 1;
    size_t position = 0;

    while(position < this->richText.text.length() && number != lineNumber){
        if(this->richText.text[position++] == '\n') number++;
    }

    this->moveCaretToPosition(position);
    this->moveCaretBeginningOfLine();
    this->focusViewForCaret();
}

void TextEditor::forEachCaret(std::function<void (Caret *)> lambda_function){
    lambda_function(&this->mainCaret);
    for(Caret *caret : this->additionalCarets) lambda_function(caret);
}

void TextEditor::duplicateCaretUp(){
    Caret *newCaret = new Caret();
    newCaret->generate(this->settings, this->font);
    newCaret->set(this->mainCaret.getPosition());
    newCaret->snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());
    mainCaret.moveUp(this->richText.text);
    this->additionalCarets.push_back(newCaret);
}

void TextEditor::duplicateCaretDown(){
    Caret *newCaret = new Caret();
    newCaret->generate(this->settings, this->font);
    newCaret->set(this->mainCaret.getPosition());
    newCaret->snapToTargetPosition(this->richText.text, this->xOffset + this->textXOffset, this->yOffset - this->calculateScrollOffset());
    mainCaret.moveDown(this->richText.text);
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
    this->makeInsight();

    this->mainCaret.set(0);
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

    this->makeInsight();
}

void TextEditor::undo(){
    if(Change *changeToUndo = this->changeRecord.getChangeToUndo()){
        this->undoChange(changeToUndo);
        this->changeRecord.nextUndo--;
    }
}

void TextEditor::redo(){
    if(Change *changeToRedo = this->changeRecord.getChangeToRedo()){
        this->redoChange(changeToRedo);
        this->changeRecord.nextUndo++;
    }
}

void TextEditor::undoChange(Change *change){
    this->onTextChange();

    switch(change->kind){
    case INSERTION: {
            InsertionChange *insertion = (InsertionChange*) change;
            this->richText.remove(insertion->position, insertion->inserted.size());
            this->moveCaretToPosition(insertion->position);

            if(std::count(insertion->inserted.begin(), insertion->inserted.end(), '\n') != 0){
                this->lineNumbersUpdated = true;
            }
        }
        break;
    case DELETION: {
            DeletionChange *deletion = (DeletionChange*) change;
            this->richText.insert(deletion->position, deletion->deleted);
            this->moveCaretToPosition(deletion->backspace ? deletion->position + deletion->deleted.size() : deletion->position);

            if(std::count(deletion->deleted.begin(), deletion->deleted.end(), '\n') != 0){
                this->lineNumbersUpdated = true;
            }
        }
        break;
    case GROUPED: {
            GroupedChange *group = (GroupedChange*) change;
            for(Change *innerChange : group->children)
                this->undoChange(innerChange);
        }
        break;
    }
}

void TextEditor::redoChange(Change *change){
    this->onTextChange();

    switch(change->kind){
    case INSERTION: {
            InsertionChange *insertion = (InsertionChange*) change;
            this->richText.insert(insertion->position, insertion->inserted);
            this->moveCaretToPosition(insertion->position + insertion->inserted.size());

            if(std::count(insertion->inserted.begin(), insertion->inserted.end(), '\n') != 0){
                this->lineNumbersUpdated = true;
            }
        }
        break;
    case DELETION: {
            DeletionChange *deletion = (DeletionChange*) change;
            this->richText.remove(deletion->position, deletion->deleted.size());
            this->moveCaretToPosition(deletion->position);

            if(std::count(deletion->deleted.begin(), deletion->deleted.end(), '\n') != 0){
                this->lineNumbersUpdated = true;
            }
        }
        break;
    case GROUPED: {
            GroupedChange *group = (GroupedChange*) change;
            for(Change *innerChange : group->children)
                this->redoChange(innerChange);
        }
        break;
    }
}

void TextEditor::getRowAndColumnAt(double xpos, double ypos, int *out_row, int *out_column){
    xpos -= this->xOffset - (this->font->mono_character_width * FONT_SCALE * 0.5f) + this->textXOffset;
    ypos -= this->yOffset - this->calculateScrollOffset();

    *out_row = floor(ypos / (this->font->line_height * FONT_SCALE)) + 1;
    *out_column = floor(xpos / (this->font->mono_character_width * FONT_SCALE)) + 1;
}

void TextEditor::handleSelection(){
    if(!this->selecting && this->selection != NULL){
        this->destroySelection();
    } else if(this->selection != NULL){
        this->selection->adjustEnd(this->mainCaret.getPosition());
    }
}

float TextEditor::calculateScrollOffset(){
    return this->calculateScrollOffset(this->scroll);
}

float TextEditor::calculateScrollOffset(size_t line){
    return (this->font->line_height * FONT_SCALE) * line;
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

void TextEditor::generateSuggestions(){
    size_t end = this->getCaretPosition();

    // Can't generate suggestions, because we don't have insight or
    // our caret is at the beginning of the file!
    if(!this->hasCompiler || end == 0){
        return;
    }

    std::string list;
    size_t lines = 0;
    size_t longest = 0;
    bool should_import_autocompletions = false;

    size_t line_beginning = getLineBeginning(this->richText.text, end);

    if(this->richText.text.length() >= line_beginning + 6 && strncmp(&this->richText.text[line_beginning], "import", 6) == 0){
        // Show auto completions for import
        should_import_autocompletions = true;
    }

    // Find beginning of word being typed
    // TODO: Clean up this messy code
    size_t beginning = end - 1;
    while(true){
        if(!isIdentifier(this->richText.text[beginning])){
            beginning++;
            break;
        }

        if(beginning == 0) break;
        beginning--;
    }

    // Store last word
    std::string lastword = this->richText.text.substr(beginning, end - beginning);

    /*
    std::string object_string = "";

    // TODO: Clean up this messy code
    if(beginning > 1 && this->richText.text[beginning - 1] == '.' && isIdentifier(this->richText.text[beginning - 2])){
        size_t object_end = beginning - 1;
        size_t object_beginning = object_end - 1;

        while(true){
            if(!isIdentifier(this->richText.text[object_beginning])){
                object_beginning++;
                break;
            }

            if(object_beginning == 0) break;
            object_beginning--;
        }

        object_string = this->richText.text.substr(object_beginning, object_end - object_beginning);
    }

    // TODO: Find object_beginning's associated scope to get the AST type of object_string
    */

    std::vector<SymbolWeight> newSymbolWeights;

    if(should_import_autocompletions){
        // TODO: Fill in with actual filesystem-based import suggestions
        std::vector<std::string> normal_list = {
            "AABB", "Anything", "Array", "array_util", "audio", "basics", "captain", "cerrno", "cmath",
            "csignal", "cstdio", "cstdlib", "cstring", "ctime", "List", "math", "Matrix4f", "mt19937",
            "Optional", "Ownership", "parse", "random", "string_util", "String", "terminal", "TypeInfo",
            "VariadicArray", "Vector2f", "Vector3f", "where"
        };

        // Ignore if just typed import
        if(lastword == "import") lastword = "";

        for(size_t i = 0; i != normal_list.size(); i++){
            int distance = levenshtein_overlapping(lastword.c_str(), normal_list[i].c_str());
            newSymbolWeights.push_back(SymbolWeight(normal_list[i], normal_list[i], distance, SymbolWeight::Kind::FILE, NULL_SOURCE));    
        }
        
        std::stable_sort(newSymbolWeights.begin(), newSymbolWeights.end());
    } else {
        compiler_t *compiler = this->borrowCompiler();
        nearestSymbols(compiler, "" /*object_string*/, lastword, &newSymbolWeights);
        this->returnCompiler();
    }

    // Grab our best suggestions into a single string and record the longest length
    for(size_t i = 0; i != settings->ide_suggestions_max && i != newSymbolWeights.size(); i++){
        list += newSymbolWeights[i].label + "\n";
        if(newSymbolWeights[i].label.length() > longest) longest = newSymbolWeights[i].label.length();
        lines++;
    }

    // Update Symbol Weights
    if(lines != 0){
        this->suggestionBox.symbolWeights = newSymbolWeights;
        this->suggestionBox.generate(list, lines, longest);
    }
}

void TextEditor::makeInsight(bool storeCreationResult, bool fromMemory, bool showSuccessMessage){
    InsightCreationResult successful_result = showSuccessMessage ? InsightCreationResultSuccess : InsightCreationResultSilentSuccess;

    if(this->richText.fileType == FileType::ADEPT){
        if(this->insightThread.joinable()) this->insightThread.join();

        char *astMaybeFromMemoryBuffer = NULL;
        
        if(fromMemory){
            // Construct memory buffer
            astMaybeFromMemoryBuffer = new char[this->richText.text.length() + 2];
            memcpy(astMaybeFromMemoryBuffer, this->richText.text.data(), this->richText.text.length());
            astMaybeFromMemoryBuffer[this->richText.text.length()] = '\n';
            astMaybeFromMemoryBuffer[this->richText.text.length() + 1] = '\0';
        }

        this->insightThread = std::thread([this, successful_result] (std::string filename, std::string adept_root, bool storeCreationResult, char *buffer) {
            this->insightRunning = true;

            compiler_t temporaryCompiler;
            compiler_init(&temporaryCompiler);
            object_t *object = compiler_new_object(&temporaryCompiler);
            InsightCreationResult result_code = successful_result;
            adept_error_t *result_error = NULL;
            adept_warning_t *result_warnings = NULL;
            length_t result_warnings_length = 0;

            object->filename = strclone(filename.c_str());
            object->full_filename = filename_absolute(object->filename);
            
            // Force object->full_filename to not be NULL
            if(object->full_filename == NULL) object->full_filename = strclone("");

            // Set compiler root
            temporaryCompiler.root = strclone(adept_root.c_str());

            // Prevent multiple people from using insight buffer at a time
            this->insightOutMessageMutex.lock();
            insight_buffer_index = 0;
            insight_buffer[0] = '\0';

            if(buffer){
                // NOTE: Passing ownership of 'buffer' to object instance!!!
                object->buffer = buffer;
            }
            
            if(buffer ? lex_buffer(&temporaryCompiler, object) : lex(&temporaryCompiler, object)){
                this->insightOutMessageMutex.unlock();

                // Failed to lex
                result_code = InsightCreationResultFailure;
                result_error = temporaryCompiler.error;
                result_warnings = temporaryCompiler.warnings;
                result_warnings_length = temporaryCompiler.warnings_length;
                temporaryCompiler.error = NULL;
                temporaryCompiler.warnings = NULL;
                temporaryCompiler.warnings_length = 0;
                temporaryCompiler.warnings_capacity = 0;
                compiler_free(&temporaryCompiler);
                goto cleanup;
            }

            if(parse(&temporaryCompiler, object)){
                this->insightOutMessageMutex.unlock();

                result_warnings = temporaryCompiler.warnings;
                result_warnings_length = temporaryCompiler.warnings_length;
                
                temporaryCompiler.warnings = NULL;
                temporaryCompiler.warnings_length = 0;
                temporaryCompiler.warnings_capacity = 0;

                if(temporaryCompiler.result_flags & COMPILER_RESULT_SUCCESS){
                    // Alternative success
                    goto store_and_cleanup;
                } else {
                    // Failed to parse
                    result_code = InsightCreationResultFailure;
                    result_error = temporaryCompiler.error;
                    temporaryCompiler.error = NULL;
                    compiler_free(&temporaryCompiler);
                }
                
                goto cleanup;
            }

            result_warnings = temporaryCompiler.warnings;
            result_warnings_length = temporaryCompiler.warnings_length;
            
            temporaryCompiler.warnings = NULL;
            temporaryCompiler.warnings_length = 0;
            temporaryCompiler.warnings_capacity = 0;

            this->insightOutMessageMutex.unlock();
        
        store_and_cleanup:
            
            this->insightMutex.lock();
            if(this->hasCompiler) compiler_free(&this->compiler);
            this->compiler = temporaryCompiler;
            this->hasCompiler = true;
            this->insightMutex.unlock();

        cleanup:
            this->insightMutex.lock();
            if(this->warnings) adept_warnings_free_fully(this->warnings, this->warnings_length);
            this->warnings = result_warnings;
            this->warnings_length = result_warnings_length;

            if(storeCreationResult) this->insightCreationResult = result_code;
            if(this->error) adept_error_free_fully(this->error);
            this->error = result_error;
            this->insightMutex.unlock();
            
            this->insightRunning = false;
        }, this->filename, this->settings->adept_root, storeCreationResult, astMaybeFromMemoryBuffer);
    } else if(storeCreationResult){
        this->insightCreationResult = InsightCreationResultNotAdept;
    }
}

size_t TextEditor::getCaretPosition(){
    return this->mainCaret.getPosition();
}

compiler_t *TextEditor::borrowCompiler(){
    this->insightMutex.lock();
    return this->hasCompiler ? &this->compiler : NULL;
}

void TextEditor::returnCompiler(){
    // NOTE: Must be called regardless of what textEditor->borrowCompiler() returns
    this->insightMutex.unlock();
}

void TextEditor::onTextChange(){
    // NOTE: The contents of 'richText->text' are not guaranteed to have changed yet

    this->clearErrorMessage();
    this->clearWarningMessages();
    this->lastChanged = glfwGetTime();
}

void TextEditor::clearErrorMessage(){
    this->insightMutex.lock();
    if(this->error){
        adept_error_free_fully(this->error);
        this->error = NULL;
    }
    this->insightMutex.unlock();
}

void TextEditor::clearWarningMessages(){
    this->insightMutex.lock();
    if(this->warnings){
        adept_warnings_free_fully(this->warnings, this->warnings_length);
        this->warnings = NULL;
        this->warnings_length = 0;
    }
    this->insightMutex.unlock();
}

const std::string &TextEditor::getText(){
    return this->richText.text;
}

TextEditor* TextEditor::asTextEditor(){
    return this;
}

ImageEditor* TextEditor::asImageEditor(){
    return NULL;
}

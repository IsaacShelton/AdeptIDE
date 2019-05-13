
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include "INTERFACE/FileLooker.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"
#include "UTIL/levenshtein.h" // NOTE: From INSIGHT

FileLooker::FileLooker(){
    this->container = NULL;
}

FileLooker::~FileLooker(){
    delete this->container;
}

void FileLooker::load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight){
    this->settings = settings;
    this->font = font;
    this->fontTexture = fontTexture;
    this->containerWidth = containerWidth;
    this->containerHeight = containerHeight;
    this->container = createSolidModel(containerWidth, containerHeight);
    this->visible = false;
    this->containerX = 0.0f;
    this->containerY = 0.0f;
    this->inputRichText.fileType = FileType::PLAIN_TEXT;
    this->inputRichText.setFont(this->font);
}

void FileLooker::render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets){
    if(!this->visible) return;

    // Draw container
    Vector4f color(0.13, 0.14, 0.15, 1.0f);
    this->transformationMatrix.translateFromIdentity(this->containerX, this->containerY, 0.92f);
    solidShader->bind();
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    solidShader->giveVector4f("color", color);
    this->container->draw();

    fontShader->bind();
    fontShader->giveMatrix4f("projection_matrix", projectionMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, this->fontTexture->getID());

    this->transformationMatrix.translateFromIdentity(this->containerX + 12.0f, this->containerY + 8.0f, 0.93f);
    fontShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    fontShader->giveFloat("width", 0.43);
    fontShader->giveFloat("edge", 0.275);
    fontShader->giveFloat("y_upper_clip", 0.0f);
    this->inputRichText.getTextModel()->draw();
}

bool FileLooker::leftClick(AdeptIDE *adeptide, double x, double y){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        return true;
    }

    return false;
}

bool FileLooker::scrollUpIfHovering(double x, double y, int lineCount){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        return true;
    }

    return false;
}

bool FileLooker::scrollDownIfHovering(double x, double y, int lineCount){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        return true;
    }

    return false;
}

bool FileLooker::isBeingHovered(double x, double y){
    assert(false && "FileLooker::isBeingHovered is unimplemented");
    return true;
}

bool FileLooker::isVisible(){
    return this->visible;
}

void FileLooker::toggleVisibility(){
    this->visible = !this->visible;
    if(this->visible) this->inputRichText.remove(0, this->inputRichText.text.length());
}

void FileLooker::setVisibility(bool visibility){
    this->visible = visibility;
    if(this->visible) this->inputRichText.remove(0, this->inputRichText.text.length());
}

bool FileLooker::type(std::string text){
    if(!this->visible) return false;
    text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
    this->inputRichText.insert(this->inputRichText.text.length(), text);
    return true;
}

bool FileLooker::type(unsigned int codepoint){
    if(!this->visible) return false;
    if(codepoint == '\n') return true;
    this->inputRichText.insert(this->inputRichText.text.length(), codepoint);
    return true;
}

bool FileLooker::backspace(){
    if(!this->visible) return false;
    if(this->inputRichText.text.length() != 0)
        this->inputRichText.remove(this->inputRichText.text.length() - 1, 1);
    return true;
}

void FileLooker::setFiles(ExplorerNode *rootNode){
    this->clearFiles();
    this->addFiles(rootNode);
    this->sortFiles();
}

void FileLooker::addFiles(ExplorerNode *node){
    for(ExplorerNode *child : node->children)
        this->addFiles(child);
    if(!node->isFolder()){
        if(node->getFilenameExtension() == "adept" || node->getFilenameExtension() == "java"
        || node->getFilenameExtension() == "html" || node->getFilenameExtension() == "json")
            this->possibilities.push_back(FileSearchEntry(node->getFilename(), node->getFilenameWithoutPath()));
    }
}

void FileLooker::clearFiles(){
    this->possibilities.clear();
}

void FileLooker::sortFiles(){
    std::sort(this->possibilities.begin(), this->possibilities.end());
}

std::string FileLooker::look(){
    // NOTE: Returns "" if nothing found
    const std::string& text = this->inputRichText.text;

    for(size_t i = 0; i != this->possibilities.size(); i++){
        if(text.length() > this->possibilities[i].withPath.length()) continue;

        size_t where = this->possibilities[i].withPath.find(text);
        if(where == std::string::npos) continue;

        return this->possibilities[i].withPath;
    }

    return "";
}

FileSearchEntry::FileSearchEntry(const std::string withPath, const std::string withoutPath){
    this->withPath = withPath;
    this->withoutPath = withoutPath;
}

bool FileSearchEntry::operator<(const FileSearchEntry& other) const {
    return this->withPath < other.withPath;
}

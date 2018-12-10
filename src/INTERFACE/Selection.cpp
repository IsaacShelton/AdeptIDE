
#include <stdlib.h>

#include "INTERFACE/Caret.h"
#include "INTERFACE/Selection.h"
#include "OPENGL/Vector3f.h"

Selection::Selection(size_t beginning, size_t end){
    this->model = NULL;
    this->adjust(beginning, end);
}

Selection::~Selection(){
    delete this->model;
}

void Selection::draw(const std::string& text, Font& font){
    if(this->updated){
        delete this->model;
        this->generateModel(text, font);
    }

    this->model->draw();
}

void Selection::adjust(size_t beginning, size_t end){
    this->beginning = beginning;
    this->end = end;
    this->updated = true;
}

void Selection::adjustBeginning(size_t beginning){
    this->beginning = beginning;
    this->updated = true;
}

void Selection::adjustEnd(size_t end){
    this->end = end;
    this->updated = true;
}

void Selection::generateModel(const std::string& text, Font& font){
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    if(this->beginning == this->end){
        this->model = new SolidModel(vertices, indices);
        this->updated = false;
    }

    size_t beginning = this->beginning;
    size_t end = this->end;

    if(end < beginning) std::swap(beginning, end);

    float characterWidth = font.mono_character_width * 0.17f;
    float lineHeight = font.line_height * 0.17f;
    size_t i = beginning;
    float lineStartX, lineStartY, lineEndX, lineEndY;

    while(true){
        if(i == end) break;
        
        if(text[i] == '\n'){
            i++;
            continue;
        }

        Caret::getTargetCoords(i, text, font, 0.0f, 0.0f, &lineStartX, &lineStartY);

        while(i + 1 < text.length() && text[i + 1] != '\n' && i + 1 != end){
            i++;
        }

        Caret::getTargetCoords(i, text, font, 0.0f, 0.0f, &lineEndX, &lineEndY);

        vertices.push_back(lineStartX);
        vertices.push_back(lineEndY + lineHeight);
        vertices.push_back(0.0f);
        vertices.push_back(lineStartX);
        vertices.push_back(lineStartY);
        vertices.push_back(0.0f);
        vertices.push_back(lineEndX + characterWidth);
        vertices.push_back(lineStartY);
        vertices.push_back(0.0f);
        vertices.push_back(lineEndX + characterWidth);
        vertices.push_back(lineEndY + lineHeight);
        vertices.push_back(0.0f);

        size_t indicesOffset = vertices.size() / 12 * 4 - 4;
        indices.push_back(0 + indicesOffset);
        indices.push_back(1 + indicesOffset);
        indices.push_back(3 + indicesOffset);
        indices.push_back(3 + indicesOffset);
        indices.push_back(1 + indicesOffset);
        indices.push_back(2 + indicesOffset);

        if(i >= text.length() || i + 1 == end) break;
        i++;
        if(i >= text.length() || i + 1 == end) break;

        beginning = ++i;
    }

    this->model = new SolidModel(vertices, indices);
    this->updated = false;
}

size_t Selection::getBeginning(){
    return this->beginning;
}

size_t Selection::getEnd(){
    return this->end;
}

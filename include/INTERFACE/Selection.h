
#ifndef SELECTION_H
#define SELECTION_H

#include <string>

#include "INTERFACE/Font.h"
#include "OPENGL/Shader.h"
#include "OPENGL/SolidModel.h"

class Selection {
    SolidModel *model;
    bool updated;

    size_t beginning;
    size_t end;

    void generateModel(const std::string& text, Font& font);

public:
    Selection(size_t beginning, size_t end);
    ~Selection();

    void draw(const std::string& text, Font& font);
    void adjust(size_t beginning, size_t end);
    void adjustBeginning(size_t beginning);
    void adjustEnd(size_t end);

    size_t getBeginning();
    size_t getEnd();
};

#endif // SELECTION_H
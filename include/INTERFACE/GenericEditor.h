
#ifndef GENERIC_EDITOR_H_INCLUDED
#define GENERIC_EDITOR_H_INCLUDED

#include <string>
#include "OPENGL/Model.h"

class GenericEditor;
class TextEditor;

#include "INTERFACE/TextEditor.h"

class GenericEditor {
public:
    Settings *settings;
    std::string filename;
    float xOffset, yOffset;
    float scrollXOffset, scrollYOffset;
    float targetScrollXOffset, targetScrollYOffset;
    float textWidth;
    float textEdge;
    float maxWidth;
    float maxHeight;
    std::string displayFilename;

protected:
    TextModel filenameModel;
    bool hasFilenameModel;

public:
    virtual ~GenericEditor(){}
    virtual TextEditor *asTextEditor() = 0;
    virtual void free(){}

    virtual TextModel *getFilenameModel() = 0;
};

#endif // GENERIC_EDITOR_H_INCLUDED

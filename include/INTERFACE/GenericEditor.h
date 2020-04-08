
#ifndef GENERIC_EDITOR_H_INCLUDED
#define GENERIC_EDITOR_H_INCLUDED

#include <string>
#include "OPENGL/Model.h"
#include "OPENGL/Shader.h"
#include "OPENGL/Matrix4f.h"
#include "INTERFACE/Font.h"
#include "INTERFACE/Assets.h"
#include "INTERFACE/FileType.h"
#include "INTERFACE/Settings.h"

class TextEditor;
class ImageEditor;

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

protected:
    Font *font;
    Texture *fontTexture;
    TextModel filenameModel;
    bool hasFilenameModel;
    std::string displayFilename;

public:
    void load(Settings *settings, Font *font, Texture *fontTexture, float maxWidth, float maxHeight);

    virtual ~GenericEditor(){}
    virtual TextEditor *asTextEditor() = 0;
    virtual ImageEditor *asImageEditor() = 0;
    virtual void free(){}

    virtual FileType getFileType() = 0;
    virtual TextModel *getFilenameModel() = 0;
    virtual size_t getDisplayFilenameLength() = 0;

    virtual void render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets) = 0;

    virtual void setOffset(float xOffset, float yOffset) = 0;
};

#include "INTERFACE/TextEditor.h"
#include "INTERFACE/ImageEditor.h"

#endif // GENERIC_EDITOR_H_INCLUDED

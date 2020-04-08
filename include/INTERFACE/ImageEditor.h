
#ifndef IMAGE_EDITOR_H_INCLUDED
#define IMAGE_EDITOR_H_INCLUDED

#include "OPENGL/Image.h"
#include "INTERFACE/GenericEditor.h"

class ImageEditor : public GenericEditor {
    Matrix4f transformationMatrix;
    Model *model;

    std::string filename;
    Texture *texture;
    Image *image;
    double imgX, imgY, startDragX, startDragY, endDragX, endDragY;
    bool dragging;
    double scale;

    void updateFilenameModel();

public:
    ~ImageEditor();
    void load(Settings *settings, Font *font, Texture *fontTexture, float maxWidth, float maxHeight);
    void render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets);
    void loadImageFromFile(const std::string& filename);

    TextEditor *asTextEditor();
    ImageEditor *asImageEditor();

    FileType getFileType();
    TextModel *getFilenameModel();
    size_t getDisplayFilenameLength();

    void startDrag(double x, double y);
    void updateDrag(double x, double y);
    void endDrag(double x, double y);
    bool isDragging();
    void zoomIn(int lineCount);
    void zoomOut(int lineCount);

    void setOffset(float xOffset, float yOffset);
};

#endif // IMAGE_EDITOR_H_INCLUDED


#include "UTIL/strings.h"
#include "INTERFACE/ImageEditor.h"

ImageEditor::~ImageEditor(){
    delete this->model;
    delete this->texture;
    delete this->image;
}

void ImageEditor::updateFilenameModel(){
    if (this->hasFilenameModel)
        this->filenameModel.free();

    this->displayFilename = (this->filename == "") ? "Untitled" : filename_name(this->filename);

    this->filenameModel = this->font->generatePlainTextModel(this->displayFilename, 0.17f);
    this->hasFilenameModel = true;
}

void ImageEditor::load(Settings *settings, Font *font, Texture *fontTexture, float maxWidth, float maxHeight){
    GenericEditor::load(settings, font, fontTexture, maxWidth, maxHeight);
    this->model = NULL;
    this->texture = NULL;
    this->image = NULL;
    this->imgX = 0;
    this->imgY = 0;
    this->startDragX = 0;
    this->startDragY = 0;
    this->endDragX = 0;
    this->endDragY = 0;
    this->dragging = false;
    this->scale = 1;

    this->loadImageFromFile("/Users/isaac/Downloads/test.png");
}

void ImageEditor::render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader){
    this->transformationMatrix.translateFromIdentity(0.0f, 0.0f, 0.0f);

    Vector4f color(1.0, 0.14, 0.15, 1.0f);

    if(this->model == NULL && this->image){
        this->model = makeRectangleModelCentered(this->texture, this->image->width, this->image->height);
        this->model->texture = this->texture;
    }

    if(this->model && this->model->texture != NULL){
        float offsetX, offsetY;

        if(this->dragging){
            offsetX = endDragX - startDragX;
            offsetY = endDragY - startDragY;
        } else {
            offsetX = 0.0f;
            offsetY = 0.0f;
        }

        // const float halfWidth = (float) this->image->width / 2.0f;
        // const float halfHeight = (float) this->image->height / 2.0f;

        shader->bind();
        shader->giveMatrix4f("projection_matrix", projectionMatrix);
        transformationMatrix.translateFromIdentity(imgX + offsetX, imgY + offsetY + 24.0f, 0.5f);
        transformationMatrix.scale(scale, scale, 0.0f);
        //transformationMatrix.translateFromIdentity(0.0f - halfWidth, 0.0f - halfHeight, 0.0f);
        shader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
        renderModel(model);
    }
}

void ImageEditor::loadImageFromFile(const std::string& filename){
    this->filename = filename;
    delete this->texture;
    this->texture = new Texture(filename, TextureLoadOptions::ALPHA_BLEND);

    this->image = new Image("/Users/isaac/Downloads/test.bmp");
    delete this->texture;
    this->texture = new Texture(this->image);

    if(this->model) this->model->texture = this->texture;
}

FileType ImageEditor::getFileType(){
    return FileType::PAINTING;
}

TextModel *ImageEditor::getFilenameModel(){
    if (!this->hasFilenameModel) this->updateFilenameModel();
    return &this->filenameModel;
}

void ImageEditor::startDrag(double x, double y){
    this->startDragX = x;
    this->startDragY = y;
    this->endDragX = x;
    this->endDragY = y;
    this->dragging = true;
}

void ImageEditor::updateDrag(double x, double y){
    this->endDragX = x;
    this->endDragY = y;
}

void ImageEditor::endDrag(double x, double y){
    this->dragging = false;
    this->imgX += endDragX - startDragX;
    this->imgY += endDragY - startDragY;
}

bool ImageEditor::isDragging(){
    return this->dragging;
}

void ImageEditor::zoomIn(int lineCount){
    this->scale += 0.01 * lineCount;
}

void ImageEditor::zoomOut(int lineCount){
    this->scale -= 0.01 * lineCount;
}

TextEditor *ImageEditor::asTextEditor(){
    return NULL;
}

ImageEditor *ImageEditor::asImageEditor(){
    return this;
}


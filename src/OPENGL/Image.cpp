
#include "stb/stb_image.h"
#include "OPENGL/Image.h"

Image::Image(CreationMode mode, imgcoord_t w, imgcoord_t h){
    this->width = w;
    this->height = h;
    this->size = w * h * 4;
    this->data = new imgcomp_t[this->size];

    switch(mode){
    case UNINITIALIZED:           break;
    case BLANK_WHITE:             this->fillRectUnsafe(0, 0, width, height, 255, 255, 255, 255); break;
    case BLANK_BLACK:             this->fillRectUnsafe(0, 0, width, height,   0,   0,   0, 255); break;
    case BLANK_BLACK_TRANSPARENT: this->fillRectUnsafe(0, 0, width, height,   0,   0,   0,   0); break;
    }
}

Image::Image(const std::string& filename){
    // Returns true if failed to load or file doesn't exist

    int comp, w, h;
    unsigned char* image = stbi_load(filename.c_str(), &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) this->data = NULL;

    this->width = w;
    this->height = h;
    this->size = w * h * 4;
    this->data = new imgcomp_t[this->size];
    memcpy(this->data, image, this->size);
    stbi_image_free(image);
}

Image::~Image(){
    delete this->data;
}


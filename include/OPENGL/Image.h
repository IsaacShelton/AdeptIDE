
#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED

#include <string>

typedef unsigned long long imgcoord_t; // Image Coordinate
typedef unsigned char imgcomp_t;       // Image Component (R, G, B, or A)

struct Image {
    // RGBA Image Data
    imgcomp_t *data;
    imgcoord_t size;
    imgcoord_t width;
    imgcoord_t height;

    enum CreationMode {
        UNINITIALIZED, BLANK_WHITE, BLANK_BLACK, BLANK_BLACK_TRANSPARENT
    };

    Image(CreationMode mode, imgcoord_t w, imgcoord_t h);
    Image(const std::string& filename);
    ~Image();

    inline void setPixelUnsafe(imgcoord_t x, imgcoord_t y, imgcomp_t r, imgcomp_t g, imgcomp_t b, imgcomp_t a){
        imgcomp_t *const beginningOfPixel = &data[(y * width + x) * 4];
        beginningOfPixel[0] = r;
        beginningOfPixel[1] = g;
        beginningOfPixel[2] = b;
        beginningOfPixel[3] = a;
    }

    inline void setPixelSafe(imgcoord_t x, imgcoord_t y, imgcomp_t r, imgcomp_t g, imgcomp_t b, imgcomp_t a){
        if(x < 0 || x >= width || y < 0 || y >= height) return;
        setPixelUnsafe(x, y, r, g, b, a);
    }

    inline void fillRectUnsafe(imgcoord_t x, imgcoord_t y, imgcoord_t w, imgcoord_t h, imgcomp_t r, imgcomp_t g, imgcomp_t b, imgcomp_t a){
        for(imgcoord_t yy = y; yy != y + h; yy++) for(imgcoord_t xx = x; xx != x + w; xx++)
            setPixelUnsafe(xx, yy, r, g, b, a);
    }

    inline void fillRectSafe(imgcoord_t x, imgcoord_t y, imgcoord_t w, imgcoord_t h, imgcomp_t r, imgcomp_t g, imgcomp_t b, imgcomp_t a){
        if(x < 0 || x >= width || y < 0 || y >= height || x + w < 0 || x + w >= width || y + h < 0 || y + h >= height) return;
        fillRectUnsafe(x, y, w, h, r, g, b, a);
    }

private:
    // Don't allow others to copy
    Image(const Image&);
};

#endif // IMAGE_H_INCLUDED
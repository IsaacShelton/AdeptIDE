
#ifndef ASSETS_H_INCLUDED
#define ASSETS_H_INCLUDED

#include "OPENGL/Model.h"
#include "OPENGL/Texture.h"

struct AdeptIDEAssets {
    Model *emblemModel = NULL;
    Model *plainTextModel = NULL;
    Model *adeptModel = NULL;
    Model *javaModel = NULL;
    Model *htmlModel = NULL;
    Model *paintingModel = NULL;
    Model *folderModel = NULL;
    Model *explorerToggleModel = NULL;
    Model *openFolderModel = NULL;
    Model *treeModel = NULL;
    Model *functionModel = NULL;
    Model *structureModel = NULL;

    Texture *fontTexture = NULL;
    Texture *emblemTexture = NULL;
    Texture *plainTextTexture = NULL;
    Texture *adeptTexture = NULL;
    Texture *javaTexture = NULL;
    Texture *htmlTexture = NULL;
    Texture *paintingTexture = NULL;
    Texture *folderTexture = NULL;
    Texture *explorerToggleTexture = NULL;
    Texture *openFolderTexture = NULL;
    Texture *treeTexture = NULL;
    Texture *functionTexture = NULL;
    Texture *structureTexture = NULL;

    ~AdeptIDEAssets();

    void loadAdeptIDEAssets(const std::string &assetsFolder);

private:
    inline void texture(const std::string& assetsFolder, const std::string& filename, Texture **output){
        *output = new Texture(assetsFolder + filename, TextureLoadOptions::ALPHA);
    }

    inline void model(size_t size, Texture *texture, Model **output){
        *output = makeSquareModel(texture, size);
    }
};

#endif // ASSETS_H_INCLUDED

#include "INTERFACE/Assets.h"

AdeptIDEAssets::~AdeptIDEAssets(){
    delete emblemModel;
    delete plainTextModel;
    delete adeptModel;
    delete javaModel;
    delete htmlModel;
    delete paintingModel;
    delete folderModel;
    delete explorerToggleModel;
    delete openFolderModel;
    delete treeModel;
    delete functionModel;
    delete structureModel;
    delete cdModel;

    delete emblemTexture;
    delete plainTextTexture;
    delete adeptTexture;
    delete javaTexture;
    delete htmlTexture;
    delete paintingTexture;
    delete folderTexture;
    delete explorerToggleTexture;
    delete openFolderTexture;
    delete treeTexture;
    delete functionTexture;
    delete structureTexture;
    delete cdTexture;
}

void AdeptIDEAssets::loadAdeptIDEAssets(const std::string& assetsFolder){
    texture(assetsFolder, "emblem.png", &emblemTexture);
    texture(assetsFolder, "plaintext.png", &plainTextTexture);
    texture(assetsFolder, "adept.png", &adeptTexture);
    texture(assetsFolder, "java.png", &javaTexture);
    texture(assetsFolder, "html.png", &htmlTexture);
    texture(assetsFolder, "painting.png", &paintingTexture);
    texture(assetsFolder, "folder.png", &folderTexture);
    texture(assetsFolder, "explorer.png", &explorerToggleTexture);
    texture(assetsFolder, "openFolder.png", &openFolderTexture);
    texture(assetsFolder, "tree.png", &treeTexture);
    texture(assetsFolder, "function.png", &functionTexture);
    texture(assetsFolder, "structure.png", &structureTexture);
    texture(assetsFolder, "cd.png", &cdTexture);
    
    model(256, emblemTexture, &emblemModel);
    model(16, plainTextTexture, &plainTextModel);
    model(16, adeptTexture, &adeptModel);
    model(16, javaTexture, &javaModel);
    model(16, htmlTexture, &htmlModel);
    model(16, paintingTexture, &paintingModel);
    model(16, folderTexture, &folderModel);
    model(16, explorerToggleTexture, &explorerToggleModel);
    model(16, openFolderTexture, &openFolderModel);
    model(16, treeTexture, &treeModel);
    model(16, functionTexture, &functionModel);
    model(16, structureTexture, &structureModel);
    model(16, cdTexture, &cdModel);
}


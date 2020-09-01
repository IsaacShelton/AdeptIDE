
#include "INTERFACE/Assets.h"

AdeptIDEAssets::~AdeptIDEAssets(){
    delete emblemModel;
    delete plainTextModel;
    delete adeptModel;
    delete javaModel;
    delete htmlModel;
    delete jsonModel;
    delete paintingModel;
    delete folderModel;
    delete explorerToggleModel;
    delete openFolderModel;
    delete treeModel;
    delete terminalModel;
    delete functionModel;
    delete structureModel;
    delete globalModel;
    delete constantModel;
    delete enumModel;
    delete cdModel;
    delete singlePixelModel;

    delete emblemTexture;
    delete plainTextTexture;
    delete adeptTexture;
    delete javaTexture;
    delete htmlTexture;
    delete jsonTexture;
    delete paintingTexture;
    delete folderTexture;
    delete explorerToggleTexture;
    delete openFolderTexture;
    delete treeTexture;
    delete terminalTexture;
    delete functionTexture;
    delete structureTexture;
    delete globalTexture;
    delete constantTexture;
    delete enumTexture;
    delete cdTexture;
}

void AdeptIDEAssets::loadAdeptIDEAssets(const std::string& assetsFolder){
    texture(assetsFolder, "emblem.png", &emblemTexture);
    texture(assetsFolder, "plaintext.png", &plainTextTexture);
    texture(assetsFolder, "adept.png", &adeptTexture);
    texture(assetsFolder, "java.png", &javaTexture);
    texture(assetsFolder, "html.png", &htmlTexture);
    texture(assetsFolder, "json.png", &jsonTexture);
    texture(assetsFolder, "painting.png", &paintingTexture);
    texture(assetsFolder, "folder.png", &folderTexture);
    texture(assetsFolder, "explorer.png", &explorerToggleTexture);
    texture(assetsFolder, "openFolder.png", &openFolderTexture);
    texture(assetsFolder, "tree.png", &treeTexture);
    texture(assetsFolder, "terminal.png", &terminalTexture);
    texture(assetsFolder, "function.png", &functionTexture);
    texture(assetsFolder, "structure.png", &structureTexture);
    texture(assetsFolder, "global.png", &globalTexture);
    texture(assetsFolder, "constant.png", &constantTexture);
    texture(assetsFolder, "enum.png", &enumTexture);
    texture(assetsFolder, "cd.png", &cdTexture);
    
    model(256, emblemTexture, &emblemModel);
    model(16, plainTextTexture, &plainTextModel);
    model(16, adeptTexture, &adeptModel);
    model(16, javaTexture, &javaModel);
    model(16, htmlTexture, &htmlModel);
    model(16, jsonTexture, &jsonModel);
    model(16, paintingTexture, &paintingModel);
    model(16, folderTexture, &folderModel);
    model(16, explorerToggleTexture, &explorerToggleModel);
    model(16, openFolderTexture, &openFolderModel);
    model(16, treeTexture, &treeModel);
    model(16, terminalTexture, &terminalModel);
    model(16, functionTexture, &functionModel);
    model(16, structureTexture, &structureModel);
    model(16, globalTexture, &globalModel);
    model(16, constantTexture, &constantModel);
    model(16, enumTexture, &enumModel);
    model(16, cdTexture, &cdModel);
    singlePixelModel = createSolidModel(1.0f, 1.0f);
}


#ifndef EXPLORER_H_INCLUDED
#define EXPLORER_H_INCLUDED

class AdeptIDE;
class FileLooker;

#include <string>
#include "OPENGL/Model.h"
#include "OPENGL/SolidModel.h"
#include "OPENGL/Shader.h"
#include "OPENGL/Texture.h"
#include "OPENGL/Matrix4f.h"
#include "INTERFACE/Font.h"
#include "INTERFACE/Assets.h"
#include "INTERFACE/Settings.h"
#include "INTERFACE/FileLooker.h"
#include "PROCESS/FolderWatcher.h"

#define MAX_EXPLORER_NODES_GENERATED 750

class ExplorerNode;

class ExplorerNode {
public:
    enum Kind {FOLDER, FILE};
    bool isCollapsed;

private:
    std::string filenameWithoutPath;
    Kind kind;
    TextModel textModel;
    Model *icon;

    std::string cachedFilenameExtension;
    bool hasCachedFilenameExtension;

public:
    ExplorerNode *parent;
    std::vector<ExplorerNode*> children;

    ExplorerNode(const std::string &name, Kind kind, Font *fontUsedToCreateTextModel, AdeptIDEAssets *assets);
    ~ExplorerNode();
    
    bool isFolder();
    std::string getFilename();
    const std::string& getFilenameWithoutPath();
    std::string getFilenameWithoutPathSafe();
    const std::string& getFilenameExtension();
    std::string getFilenameExtensionSafe();
    ExplorerNode::Kind getKind();

    void addChild(ExplorerNode *child);
    void sortChildren(Settings *settings);
    bool generateChildren(Settings *settings, Font *font, int *nodesAlreadyGenerated, AdeptIDEAssets *assets);
    bool generateAndSortChildren(Settings *settings, Font *font, int *nodesAlreadyGenerated, AdeptIDEAssets *assets);
    int countDescendants();

    void draw(Settings *settings, Font *font, Texture *fontTexture, Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, AdeptIDEAssets *assets, float drawOffsetX, float *drawOffsetY, float containerX, float containerWidth);
    bool propagateLeftClick(AdeptIDE *adeptide, Font *font, float clickX, float clickY, float testX, float *testY);
};

class Explorer {
    Settings *settings;
    Font *font;
    Texture *fontTexture;
    bool visible;

    std::string folderPath;
    ExplorerNode *rootNode;
    SolidModel *container;
    float containerWidth, containerHeight;
    Matrix4f transformationMatrix;
    FileLooker *fileLooker;
    FolderWatcher *folderWatcher;

    double scroll, maxScroll;
    float targetScrollXOffset, targetScrollYOffset, scrollXOffset, scrollYOffset;

    float calculateScrollOffset();

  public:
    // Used for animating appearing and disappearing of explorer
    float containerX;

    Explorer();
    ~Explorer();
    void load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight, FileLooker *fileLooker, AdeptIDEAssets *assets);
    void render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets);
    void resize(float height);
    bool leftClick(AdeptIDE* adeptide, double x, double y);
    bool scrollUpIfHovering(double x, double y, int lineCount);
    bool scrollDownIfHovering(double x, double y, int lineCount);
    bool isBeingHovered(double x, double y);
    float getContainerWidth();
    bool isVisible();
    void toggleVisibility();
    void setVisibility(bool visibility);
    void update(AdeptIDEAssets *assets);

    bool setRootFolder(const std::string& path, AdeptIDEAssets *assets);
    bool generateNodes(AdeptIDEAssets *assets);
    bool refreshNodes(AdeptIDEAssets *assets);
    std::string getFolderPath();
};

#endif // EXPLORER_H_INCLUDED

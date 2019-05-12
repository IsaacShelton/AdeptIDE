
#ifndef FILELOOKER_H_INCLUDED
#define FILELOOKER_H_INCLUDED

class AdeptIDE;
class ExplorerNode;

#include <string>
#include "OPENGL/Model.h"
#include "OPENGL/SolidModel.h"
#include "OPENGL/Shader.h"
#include "OPENGL/Texture.h"
#include "OPENGL/Matrix4f.h"
#include "INTERFACE/Font.h"
#include "INTERFACE/Assets.h"
#include "INTERFACE/Settings.h"
#include "INTERFACE/RichText.h"
#include "INTERFACE/Explorer.h"

class FileSearchEntry {
public:
    std::string withPath;
    std::string withoutPath;

    FileSearchEntry(const std::string withPath, const std::string withoutPath);
    bool operator<(const FileSearchEntry& other) const;
};

class FileLooker {
    Settings *settings;
    Font *font;
    Texture *fontTexture;
    bool visible;

    SolidModel *container;
    Matrix4f transformationMatrix;
    RichText inputRichText;

    std::vector<FileSearchEntry> possibilities;

    void addFiles(ExplorerNode *node);
    void clearFiles();
    void sortFiles();

  public:
    float containerWidth, containerHeight;
    float containerX, containerY;

    FileLooker();
    ~FileLooker();
    void load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight);
    void render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets);
    bool leftClick(AdeptIDE* adeptide, double x, double y);
    bool scrollUpIfHovering(double x, double y, int lineCount);
    bool scrollDownIfHovering(double x, double y, int lineCount);
    bool isBeingHovered(double x, double y);
    bool isVisible();
    void toggleVisibility();
    void setVisibility(bool visibility);
    bool type(std::string);
    bool type(unsigned int codepoint);
    bool backspace();
    void setFiles(ExplorerNode *rootNode);
    std::string look();
};

#endif // EXPLORER_H_INCLUDED

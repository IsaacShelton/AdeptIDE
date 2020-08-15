
#define CUTE_FILES_IMPLEMENTATION
#include "cute/cute_files.h"

#include <math.h>
#include <iostream>
#include "INTERFACE/AdeptIDE.h"
#include "INTERFACE/Explorer.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"

Explorer::Explorer(){
    this->folderPath = "";
    this->rootNode = NULL;
    this->container = NULL;
    this->fileLooker = NULL;
    this->folderWatcher = NULL;
}

Explorer::~Explorer(){
    delete this->container;
    delete this->folderWatcher;
}

void Explorer::load(Settings *settings, Font *font, Texture *fontTexture, float containerWidth, float containerHeight, FileLooker *fileLooker){
    this->settings = settings;
    this->font = font;
    this->fontTexture = fontTexture;
    this->containerWidth = containerWidth;
    this->containerHeight = containerHeight;
    this->container = createSolidModel(containerWidth, containerHeight);
    this->rootNode = NULL;
    this->scroll = 0;
    this->maxScroll = 0;
    this->targetScrollXOffset = 0.0f;
    this->targetScrollYOffset = 0.0f;
    this->scrollXOffset = 0.0f;
    this->scrollYOffset = 0.0f;
    this->visible = settings->explorer_default_show;
    this->containerX = this->visible ? 0.0f : -256.0f;
    this->fileLooker = fileLooker;

    this->setRootFolder(settings->explorer_default_folder);
}

void Explorer::render(Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, Shader *solidShader, AdeptIDEAssets *assets){
    this->targetScrollYOffset = this->calculateScrollOffset();

    if(fabs(this->scrollXOffset - this->targetScrollXOffset) > 0.01f){
        this->scrollXOffset += (this->targetScrollXOffset > this->scrollXOffset ? 1 : -1) * fabs(this->scrollXOffset - this->targetScrollXOffset) * clampedHalfDelta(this->settings->hidden.delta);
    } else this->scrollXOffset = this->targetScrollXOffset;

    if(fabs(this->scrollYOffset - this->targetScrollYOffset) > 0.01f){
        this->scrollYOffset += (this->targetScrollYOffset > this->scrollYOffset ? 1 : -1) * fabs(this->scrollYOffset - this->targetScrollYOffset) * clampedHalfDelta(this->settings->hidden.delta);
    } else this->scrollYOffset = this->targetScrollYOffset;

    float targetContainerX = this->visible ? 0.0f : -256.0f;

    if(fabs(this->containerX - targetContainerX) > 0.01f){
        this->containerX += (targetContainerX > this->containerX ? 1 : -1) * fabs(this->containerX - targetContainerX) * clampedHalfDelta(this->settings->hidden.delta);
    } else this->containerX = targetContainerX;

    if(this->containerX == -256.0f) return;

    // Draw container
    Vector4f color(0.17f * 0.8, 0.19 * 0.8, 0.2 * 0.8, 1.0f);
    this->transformationMatrix.translateFromIdentity(this->containerX, 0.0f, -0.4f);
    solidShader->bind();
    solidShader->giveMatrix4f("projection_matrix", projectionMatrix);
    solidShader->giveMatrix4f("transformation_matrix", this->transformationMatrix);
    solidShader->giveVector4f("color", color);
    this->container->draw();

    float x = this->containerX + 8.0f, y = 32.0f + 4.0f - this->scrollYOffset;

    if(this->rootNode){
        for(ExplorerNode *child : this->rootNode->children){
            child->draw(settings, font, fontTexture, projectionMatrix, shader, fontShader, assets, x, &y, this->containerX, this->containerWidth);
        }
    } else {
        transformationMatrix.translateFromIdentity(this->containerX + this->containerWidth / 2.0f - 8.0f, this->containerHeight / 2.0f - 8.0f, -0.399f);

        shader->bind();
        shader->giveMatrix4f("projection_matrix", projectionMatrix);
        shader->giveMatrix4f("transformation_matrix", transformationMatrix);
        renderModel(assets->openFolderModel);
    }
}

void ExplorerNode::draw(Settings *settings, Font *font, Texture *fontTexture, Matrix4f &projectionMatrix, Shader *shader, Shader *fontShader, AdeptIDEAssets *assets, float drawOffsetX, float *drawOffsetY, float containerX, float containerWidth){
    Matrix4f transformationMatrix;
    transformationMatrix.translateFromIdentity(drawOffsetX + 8.0f, 32.0f + 4.0f + *drawOffsetY, -0.2f);

    if(settings->explorer_show_icons){
        shader->bind();
        shader->giveMatrix4f("projection_matrix", projectionMatrix);
        shader->giveMatrix4f("transformation_matrix", transformationMatrix);
        renderModel(this->kind == ExplorerNode::Kind::FILE ? assets->plainTextModel : assets->folderModel);

        transformationMatrix.translate(24.0f, 0.0f, 0.0f);
    }

    fontShader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexture->getID());
    fontShader->giveMatrix4f("transformation_matrix", transformationMatrix);
    fontShader->giveFloat("width", 0.4f);
    fontShader->giveFloat("edge", 0.38f);
    fontShader->giveFloat("x_right_clip", containerX < 1 - containerWidth ? -containerWidth : containerX + containerWidth);
    this->textModel.draw();
    fontShader->giveFloat("x_right_clip", 0.0f);

    *drawOffsetY += font->line_height * FONT_SCALE * 1.5;

    if(!this->isCollapsed) for(ExplorerNode *child : this->children){
        child->draw(settings, font, fontTexture, projectionMatrix, shader, fontShader, assets, drawOffsetX + 16.0f, drawOffsetY, containerX, containerWidth);
    }
}

void Explorer::resize(float height){
    if(this->containerHeight == height) return;
    
    delete this->container;
    containerHeight = height;
    this->container = createSolidModel(containerWidth, containerHeight);
}

bool Explorer::leftClick(AdeptIDE *adeptide, double x, double y){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        this->scrollYOffset = this->calculateScrollOffset();
        float testX = 8.0f, testY = 32.0f + 4.0f + 24.0f - this->scrollYOffset;

        if(this->rootNode){
            for(ExplorerNode *child : this->rootNode->children){
                if(child->propagateLeftClick(adeptide, font, x, y, testX, &testY)) break;
            }
        } else if(TextEditor *textEditor = adeptide->getCurrentEditorAsTextEditor()){
            char *path = filename_path(textEditor->filename.c_str());
            this->setRootFolder(path);
            free(path);
        } else {
            adeptide->openFolder();
        }

        return true;
    }

    return false;
}

bool ExplorerNode::propagateLeftClick(AdeptIDE *adeptide, Font *font, float clickX, float clickY, float testX, float *testY){
    // We don't really need to worry about testX because we already established
    // that the user left clicked within the explorer area
    if(clickY >= *testY && clickY <= *testY + font->line_height * FONT_SCALE * 1.5){
        // This node is the node that was clicked
        
        if(this->kind == ExplorerNode::Kind::FILE){
            adeptide->openEditor(this->getFilename());
        } else if(this->kind == ExplorerNode::FOLDER){
            this->isCollapsed = !this->isCollapsed;
        }

        return true;
    }

    *testY += font->line_height * FONT_SCALE * 1.5;

    if(!this->isCollapsed) for(ExplorerNode *child : this->children){
        if(child->propagateLeftClick(adeptide, font, clickX, clickY, testX + 16.0f, testY)) return true;
    }

    return false;
}

bool Explorer::scrollUpIfHovering(double x, double y, int lineCount){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        this->scroll -= lineCount / 4.0;

        if(this->scroll < 0) this->scroll = 0;
        return true;
    }

    return false;
}

bool Explorer::scrollDownIfHovering(double x, double y, int lineCount){
    if(!this->visible) return false;

    if(this->isBeingHovered(x, y)){
        this->scroll += lineCount / 4.0;

        if(this->scroll > maxScroll) this->scroll = maxScroll;
        return true;
    }

    return false;
}

bool Explorer::isBeingHovered(double x, double y){
    return x <= 256.0 && x >= 0.0 && y > 32.0f + 4.0f && y <= containerHeight;
}

float Explorer::getContainerWidth(){
    return this->containerWidth;
}

bool Explorer::isVisible(){
    return this->visible;
}

void Explorer::toggleVisibility(){
    this->visible = !this->visible;
}

void Explorer::setVisibility(bool visibility){
    this->visible = visibility;
}

void Explorer::update(){
    if(this->folderWatcher && this->folderWatcher->changeOccured()){
        this->refreshNodes();

        if(this->fileLooker)
            this->fileLooker->setFiles(this->rootNode);
    }
}

bool Explorer::setRootFolder(const std::string& path){
    // NOTE: Returns whether too many files

    delete this->rootNode;

    if(path == ""){
        delete this->folderWatcher;

        this->rootNode = NULL;
        this->folderWatcher = NULL;
        return false;
    }

    this->folderPath = path;
    this->rootNode = new ExplorerNode(path, ExplorerNode::Kind::FOLDER, this->font);
    this->rootNode->parent = NULL;
    this->scroll = 0;
    bool too_many_files = this->generateNodes();

    if(this->fileLooker)
        this->fileLooker->setFiles(this->rootNode);

    if(this->folderWatcher == NULL){
        this->folderWatcher = new FolderWatcher();
    }

    this->folderWatcher->target(path);
    return too_many_files;
}

bool Explorer::generateNodes(){
    // Returns whether too many nodes

    if(this->rootNode){
        int nodesGenerated = 0;

        if(this->rootNode->generateAndSortChildren(this->settings, this->font, &nodesGenerated)){
            maxScroll = this->rootNode->countDescendants();
            return true; // Too many nodes
        }

        maxScroll = this->rootNode->countDescendants();
    }

    return false;
}

bool Explorer::refreshNodes(){
    // Returns whether too many nodes

    if(!this->rootNode) return false;

    for(ExplorerNode *child : this->rootNode->children)
        delete child;
    this->rootNode->children.resize(0);
    return this->generateNodes();
}

float Explorer::calculateScrollOffset(){
    return font->line_height * FONT_SCALE * 1.5 * scroll;
}

std::string Explorer::getFolderPath(){
    return this->folderPath;
}

ExplorerNode::ExplorerNode(const std::string &name, ExplorerNode::Kind kind, Font *fontUsedToCreateTextModel){
    this->filenameWithoutPath = name;
    this->kind = kind;

    if (kind == FOLDER){
        this->textModel = fontUsedToCreateTextModel->generatePlainTextModel(name + "/", FONT_SCALE);
    } else {
        this->textModel = fontUsedToCreateTextModel->generatePlainTextModel(name, FONT_SCALE);
    }

    this->hasCachedFilenameExtension = false;
}

ExplorerNode::~ExplorerNode(){
    for(ExplorerNode *child : this->children)
        delete child;
    this->textModel.free();
}

bool ExplorerNode::isFolder(){
    return this->kind == Kind::FOLDER;
}

std::string ExplorerNode::getFilename(){
    std::string path = this->filenameWithoutPath;
    ExplorerNode *root = this->parent;

    while(root != NULL){
        path = root->getFilenameWithoutPath() + "/" + path;
        root = root->parent;
    }
    return path;
}

const std::string& ExplorerNode::getFilenameWithoutPath(){
    // DANGEROUS: Could lead to dangling reference after
    // this node has been deleted
    return this->filenameWithoutPath;
}

std::string ExplorerNode::getFilenameWithoutPathSafe(){
    return this->filenameWithoutPath;
}

const std::string& ExplorerNode::getFilenameExtension(){
    if(hasCachedFilenameExtension) return cachedFilenameExtension;

    cachedFilenameExtension = filename_get_extension(this->filenameWithoutPath);
    hasCachedFilenameExtension = true;
    return cachedFilenameExtension;
}

std::string ExplorerNode::getFilenameExtensionSafe(){
    if(hasCachedFilenameExtension) return cachedFilenameExtension;

    cachedFilenameExtension = filename_get_extension(this->filenameWithoutPath);
    hasCachedFilenameExtension = true;
    return cachedFilenameExtension;
}

ExplorerNode::Kind ExplorerNode::getKind(){
    return this->kind;
}

void ExplorerNode::addChild(ExplorerNode *child){
    child->parent = this;
    this->children.push_back(child);
}

Settings *reallyHackyAndUnsafeWayOfPassingSettingsToExplorerNodeComparision;
void ExplorerNode::sortChildren(Settings *settings){
    // HACK: Seems like this is the only reasonable way to sort the nodes
    // while not having to use const& and not writing our own sorting function
    // NOTE: Because we are doing it this way, this function should only be run
    // from a single thread (as of now, the main thread) - Isaac, Dec 29 2018
    reallyHackyAndUnsafeWayOfPassingSettingsToExplorerNodeComparision = settings;

    int (*comparison)(const void*, const void*) = [](const void *a, const void *b) -> int {
        Settings *settings = reallyHackyAndUnsafeWayOfPassingSettingsToExplorerNodeComparision;
        ExplorerNode *nodeA = *((ExplorerNode**) a), *nodeB = *((ExplorerNode**) b);

        if(settings->explorer_prefer_folders && nodeA->getKind() != nodeB->getKind()){
            return nodeA->getKind() == ExplorerNode::Kind::FOLDER ? -1 : 1;
        }

        if(settings->explorer_prefer_code && nodeA->getKind() == ExplorerNode::Kind::FILE && nodeB->getKind() == ExplorerNode::Kind::FILE){
            std::string nodeAExtension = nodeA->getFilenameExtension();
            std::string nodeBExtension = nodeB->getFilenameExtension();

            bool nodeAIsCode = nodeAExtension == "adept" || nodeAExtension == "java";
            bool nodeBIsCode = nodeBExtension == "adept" || nodeBExtension == "java";

            if(nodeAIsCode != nodeBIsCode){
                return nodeAIsCode ? -1 : 1;
            }
        }
        
        return (*((ExplorerNode**) a))->getFilenameWithoutPath().compare((*((ExplorerNode**) b))->getFilenameWithoutPath());
    };

    qsort(children.data(), children.size(), sizeof(ExplorerNode*), comparison);
}

bool ExplorerNode::generateChildren(Settings *settings, Font *font, int *nodesAlreadyGenerated){
    // Returns whether too many nodes

    cf_dir_t dir;
    cf_dir_open(&dir, this->getFilename().c_str());

    while(dir.has_next){
        cf_file_t file;
        cf_read_file(&dir, &file);

        if( (settings->explorer_show_files && !file.is_dir) || (settings->explorer_show_folders && file.is_dir) ){
            if((settings->explorer_show_hidden && strcmp(file.name, ".") != 0 && strcmp(file.name, "..") != 0) || file.name[0] != '.'){
                ExplorerNode *child = new ExplorerNode(file.name, file.is_dir ? ExplorerNode::Kind::FOLDER : ExplorerNode::Kind::FILE, font);
                
                this->addChild(child);
                
                if( ((*nodesAlreadyGenerated)++ == MAX_EXPLORER_NODES_GENERATED)
                ||  (file.is_dir && child->generateAndSortChildren(settings, font, nodesAlreadyGenerated)))
                    return true; // Too many nodes
            }
        }

        cf_dir_next(&dir);
    }

    cf_dir_close(&dir);
    this->sortChildren(settings);

    this->isCollapsed = settings->explorer_default_collapse;
    return false;
}

bool ExplorerNode::generateAndSortChildren(Settings *settings, Font *font, int *nodesAlreadyGenerated){
    // Returns whether too many nodes

    bool tooManyNodes = this->generateChildren(settings, font, nodesAlreadyGenerated);
    this->sortChildren(settings);
    return tooManyNodes;
}

int ExplorerNode::countDescendants(){
    int descendants = 0;

    for(ExplorerNode *child : this->children){
        descendants += 1 + child->countDescendants();
    }

    return descendants;
}

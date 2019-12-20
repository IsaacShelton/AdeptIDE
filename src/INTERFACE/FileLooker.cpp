
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include "INTERFACE/FileLooker.h"
#include "UTIL/filename.h"
#include "UTIL/animationMath.h"
#include "UTIL/levenshtein.h" // NOTE: From INSIGHT

FileLooker::FileLooker(){
    this->container = NULL;
    this->constantText = "";
}

void FileLooker::setFiles(ExplorerNode *rootNode){
    this->clearFiles();
    this->addFiles(rootNode);
    this->sortFiles();
}

void FileLooker::addFiles(ExplorerNode *node){
    for(ExplorerNode *child : node->children)
        this->addFiles(child);
    if(!node->isFolder()){
        if(node->getFilenameExtension() == "adept" || node->getFilenameExtension() == "java"
        || node->getFilenameExtension() == "html" || node->getFilenameExtension() == "json")
            this->possibilities.push_back(FileSearchEntry(node->getFilename(), node->getFilenameWithoutPath()));
    }
}

void FileLooker::clearFiles(){
    this->possibilities.clear();
}

void FileLooker::sortFiles(){
    std::sort(this->possibilities.begin(), this->possibilities.end());
}

std::string FileLooker::look(){
    // NOTE: Returns "" if nothing found
    std::string text = this->getInput();

    for(size_t i = 0; i != this->possibilities.size(); i++){
        if(text.length() > this->possibilities[i].withPath.length()) continue;

        size_t where = this->possibilities[i].withPath.find(text);
        if(where == std::string::npos) continue;

        return this->possibilities[i].withPath;
    }

    return "";
}

FileSearchEntry::FileSearchEntry(const std::string withPath, const std::string withoutPath){
    this->withPath = withPath;
    this->withoutPath = withoutPath;
}

bool FileSearchEntry::operator<(const FileSearchEntry& other) const {
    return this->withPath < other.withPath;
}

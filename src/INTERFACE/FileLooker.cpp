
#include <math.h>
#include <assert.h>
#include <iostream>
#include <algorithm>
#include "INTERFACE/FileLooker.h"
#include "UTIL/strings.h"
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

void FileLooker::addFiles(ExplorerNode *node, std::string root){
    if(root == "") root = string_ending_with_slash(node->getFilename());

    for(ExplorerNode *child : node->children)
        this->addFiles(child, root);
    if(!node->isFolder()){
        if(node->getFilenameExtension() == "adept" || node->getFilenameExtension() == "java"
        || node->getFilenameExtension() == "html" || node->getFilenameExtension() == "json")
            this->possibilities.push_back(FileSearchEntry(node->getFilename(), node->getFilenameWithoutPath(), string_without_prefix(node->getFilename(), root)));
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
    std::string input = this->getInput();

    FileSearchEntry *match = this->getBestMatch(input);
    return match ? match->withPath : "";
}

void FileLooker::onType(){
    std::string input = this->getInput();
    std::string result = "";

    // Don't look for files if input is empty
    if(input == ""){
        this->setAdditionalText("");
        return;
    }

    this->recalculateMatches(input);

    size_t max_matches = 10;
    for(const FileSearchEntry& match : this->matches){
        result += match.shortPath + "\n";
        if(--max_matches == 0) break;
    }

    // Trim ending \n from result
    if(result.length() != 0) result = result.substr(0, result.length() - 1);

    // Set additional text bar text
    this->setAdditionalText(result == "" ? "Nothing Found" : result);
}

void FileLooker::recalculateMatches(const std::string& input){
    this->matches.clear();

    for(size_t i = 0; i != this->possibilities.size(); i++){
        if(input.length() > this->possibilities[i].shortPath.length()) continue;

        size_t where = this->possibilities[i].shortPath.find(input);
        if(where == std::string::npos) continue;

        this->matches.push_back(this->possibilities[i]);
    }

    for(FileSearchEntry &match : this->matches)
        match.calculateDeviance(input);

    std::sort(this->matches.begin(), this->matches.end());
}

FileSearchEntry *FileLooker::getBestMatch(const std::string& input){
    return this->matches.size() == 0 ? NULL : &this->matches[0];
}

FileSearchEntry::FileSearchEntry(const std::string withPath, const std::string withoutPath, const std::string shortPath){
    this->withPath = withPath;
    this->withoutPath = withoutPath;
    this->shortPath = shortPath;
    this->deviance = 0;
}

bool FileSearchEntry::operator<(const FileSearchEntry& other) const {
    return this->deviance != other.deviance ? this->deviance < other.deviance : this->withPath < other.withPath;
}

void FileSearchEntry::calculateDeviance(const std::string& input){
    this->deviance = levenshtein(this->withPath.c_str(), input.c_str());
}

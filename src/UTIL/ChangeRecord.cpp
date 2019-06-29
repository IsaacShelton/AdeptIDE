
#include <iostream>
#include "UTIL/ChangeRecord.h"

InsertionChange::InsertionChange(size_t position, const std::string& inserted){
    this->kind = ChangeKind::INSERTION;
    this->position = position;
    this->inserted = inserted;
}

DeletionChange::DeletionChange(size_t position, const std::string& deleted){
    this->kind = ChangeKind::DELETION;
    this->position = position;
    this->deleted = deleted;
}

GroupedChange::GroupedChange(const std::vector<Change*>& children){
    this->kind = ChangeKind::GROUPED;
    this->children = children;
}

GroupedChange::~GroupedChange(){
    for(Change* change : this->children) delete change;
}

ChangeRecord::~ChangeRecord(){
    for(Change* change : this->temporaryGroup) delete change;
    for(Change* change : this->changes) delete change;
}

void ChangeRecord::addInsertion(size_t position, const std::string& inserted){
    this->changes.push_back(new InsertionChange(position, inserted));
}

void ChangeRecord::addInsertion(size_t position, char inserted){
    this->changes.push_back(new InsertionChange(position, std::string(&inserted, 1)));
}

void ChangeRecord::addDeletion(size_t position, const std::string& deleted){
    this->changes.push_back(new DeletionChange(position, deleted));
}

void ChangeRecord::startGroup(){
    if(this->temporaryGroup.size() != 0){
        std::cerr << "ChangeRecord::startGroup called while already making group" << std::endl;
        return;
    }
}

void ChangeRecord::endGroup(){
    this->changes.push_back(new GroupedChange(temporaryGroup));
    this->temporaryGroup.clear();
}


#include <iostream>
#include "UTIL/ChangeRecord.h"

InsertionChange::InsertionChange(size_t position, const std::string& inserted){
    this->kind = ChangeKind::INSERTION;
    this->position = position;
    this->inserted = inserted;
}

DeletionChange::DeletionChange(size_t position, const std::string& deleted, bool backspace){
    this->kind = ChangeKind::DELETION;
    this->position = position;
    this->deleted = deleted;
    this->backspace = backspace;
}

GroupedChange::GroupedChange(const std::vector<Change*>& children){
    this->kind = ChangeKind::GROUPED;
    this->children = children;
}

GroupedChange::~GroupedChange(){
    for(Change* change : this->children) delete change;
}

ChangeRecord::ChangeRecord(){
    this->nextUndo = -1;
    this->insideGroup = false;
}

ChangeRecord::~ChangeRecord(){
    for(Change* change : this->temporaryGroup) delete change;
    for(Change* change : this->changes) delete change;
}

void ChangeRecord::addInsertion(size_t position, const std::string& inserted){
    this->addChange(new InsertionChange(position, inserted));
}

void ChangeRecord::addInsertion(size_t position, char inserted){
    this->addChange(new InsertionChange(position, std::string(&inserted, 1)));
}

void ChangeRecord::addDeletion(size_t position, const std::string& deleted, bool backspace){
    this->addChange(new DeletionChange(position, deleted, backspace));
}

void ChangeRecord::startGroup(){
    if(this->insideGroup){
        std::cerr << "ChangeRecord::startGroup called while already making group" << std::endl;
        return;
    }

    this->insideGroup = true;
}

void ChangeRecord::endGroup(){
    this->nextUndo = this->changes.size();
    this->changes.push_back(new GroupedChange(temporaryGroup));
    this->temporaryGroup.clear();
    this->insideGroup = false;
}

void ChangeRecord::addChange(Change *change){
    if(this->insideGroup){
        this->temporaryGroup.push_back(change);
        return;
    }

    if((size_t) this->nextUndo != this->changes.size() - 1){
        for(Change *c : this->changes) delete c;
        this->changes.clear();
        this->nextUndo = -1;
    }

    if(this->changes.size() == 128){
        this->forgetOldestChange();
    }

    this->nextUndo = this->changes.size();
    this->changes.push_back(change);
}

void ChangeRecord::forgetOldestChange(){
    delete this->changes[0];
    this->changes.erase(this->changes.begin());
}

Change *ChangeRecord::getChangeToUndo(){
    if(this->nextUndo < 0) return NULL;
    return this->changes[this->nextUndo];
}

Change *ChangeRecord::getChangeToRedo(){
    if((size_t) this->nextUndo + 1 >= this->changes.size()) return NULL;
    return this->changes[this->nextUndo + 1];
}

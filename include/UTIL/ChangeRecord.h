
#ifndef CHANGERECORD_H_INCLUDED
#define CHANGERECORD_H_INCLUDED

#include <string>
#include <vector>

enum ChangeKind {INSERTION, DELETION, GROUPED};

struct Change {
    ChangeKind kind;
};

struct InsertionChange : public Change {
    size_t position;
    std::string inserted;

    InsertionChange(size_t position, const std::string& inserted);
};

struct DeletionChange : public Change {
    size_t position;
    std::string deleted;
    bool backspace;

    DeletionChange(size_t position, const std::string& deleted, bool backspace);
};

struct GroupedChange : public Change {
    std::vector<Change*> children;

    GroupedChange(const std::vector<Change*>& children);
    ~GroupedChange();
};

class ChangeRecord {
    std::vector<Change*> temporaryGroup;
    bool insideGroup;

public:
    std::vector<Change*> changes;
    int nextUndo;

    ChangeRecord();
    ~ChangeRecord();
    void addInsertion(size_t position, const std::string& inserted);
    void addInsertion(size_t position, char inserted);
    void addDeletion(size_t position, const std::string& deleted, bool backspace);
    void startGroup();
    void endGroup();
    void addChange(Change *change);
    void forgetOldestChange();

    Change *getChangeToUndo();
    Change *getChangeToRedo();
};

#endif

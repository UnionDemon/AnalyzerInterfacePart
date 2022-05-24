#pragma once

#include "clang/AST/AST.h"
#include "clang/AST/Stmt.h"

#include <set>
#include <map>
#include <list>

class astComplement {
private:
    std::map<void*, std::list<void*>> predecessors;
    std::map<void*, std::list<void*>> followers;
    std::set<void*> fake_nodes;

    bool isConvertableToTetrad(void* st);

public:
    void addPredecessor(void* node, void* predecessor);
    void addFollower(void* node, void* follower);

    void* findFirstSubtreeNode(void* node);
    void* findLastSubtreeNode(void* node);

    bool isFakeNode(void* node);
    void addFakeNode(void* node);
};
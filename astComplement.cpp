#include "astComplement.h"

using namespace clang;

namespace {

int countChildren(Stmt* st)
{
	int childrenCount = 0;
	for (auto i = st->child_begin(); i != st->child_end(); i++)
	{
		childrenCount++;
	}
	return childrenCount;
}

}  // namespace

void astComplement::addPredecessor(void* node, void* predecessor) {
    predecessors[node].push_front(predecessor);
}

void astComplement::addFollower(void* node, void* follower) {
    followers[node].push_back(follower);
}

bool astComplement::isFakeNode(void* node) {
    auto it = fake_nodes.find(node);
    return it != fake_nodes.end();
}

void astComplement::addFakeNode(void* node) {
    fake_nodes.insert(node);
}

void* astComplement::findLastSubtreeNode(void* node) {
    void* follower = nullptr;

    for (auto it = followers[node].rbegin(); it != followers[node].rend(); it++) {
        follower = findLastSubtreeNode(*it);
        if (follower != nullptr) {
            return follower;
        }
    }

    if (isConvertableToTetrad(node)) {
        return node;
    }

    Stmt* realStmt = static_cast<Stmt*>(node);
    std::list<Stmt*> children;
    for (auto it = realStmt->child_begin(); it != realStmt->child_end(); it++) {
        children.push_back(*it);
    }

    for (auto it = children.rbegin(); it != children.rend(); it++) {
        void* lastNode = findLastSubtreeNode(*it);
        if (lastNode != nullptr) {
            return lastNode;
        }
    }

    return nullptr;
}

void* astComplement::findFirstSubtreeNode(void* node) {
    if (!isFakeNode(node)) {
        Stmt* realStmt = static_cast<Stmt*>(node);

        void* firstNode = nullptr;
        for (auto it = realStmt->child_begin(); it != realStmt->child_end(); it++) {
            firstNode = findFirstSubtreeNode(*it);
            if (firstNode != nullptr) {
                return firstNode;
            }
        }
    }

    void* pred = nullptr;
    for (auto it: predecessors[node]) {
        pred = findFirstSubtreeNode(it);
        if (pred != nullptr) {
            return pred;
        }
    }

    if (isConvertableToTetrad(node)) {
        return node;
    }

    return nullptr;
}

// Возвращает true, если st можно превратить в тертаду
bool astComplement::isConvertableToTetrad(void* node) {
	if (isFakeNode(node)) {
		return true;
	}

	Stmt* st = static_cast<Stmt*>(node);
	if (countChildren(st) == 0) {
		return false;
	}

	if (CompoundStmt* compound_st = dyn_cast<CompoundStmt>(st)) {
		return false;
	}

	if (IfStmt* if_st = dyn_cast<IfStmt>(st)) {
		return false;
	}

	if (WhileStmt* while_st = dyn_cast<WhileStmt>(st)) {
		return false;
	}

	if (DoStmt* do_st = dyn_cast<DoStmt>(st)) {
		return false;
	}

	if (ForStmt* for_st = dyn_cast<ForStmt>(st)) {
		return false;
	}

	if (SwitchCase* switch_case_st = dyn_cast<SwitchCase>(st)) {
		return false;
	}

	//TODO: добавлять новые по мере обнаружения

	return true;
}

astComplement g_ast_complement;
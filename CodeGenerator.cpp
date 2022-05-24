#include "Tetrad.h"

// Global object that generates pseudocode
pseudoCodeGenerator g_codegenerator;

std::unordered_map<Stmt*, std::string> node_names_;

std::string nodeName(Stmt* node) {
	if (node == nullptr) {
		return std::string();
	}

	if (node_names_.find(node) == node_names_.end()) {
		node_names_[node] = node->getStmtClassName();
	}

	return node_names_[node];
}
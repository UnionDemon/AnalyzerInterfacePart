#pragma once
#include "Tetrad.h"
#include <map>

class BasicBlock {
private:
	std::list<Tetrad*> instructions;
	int id;

	static int counter;
public:
	BasicBlock(const std::list<Tetrad*> &instruct);
	int getId();
	const std::list<Tetrad*>& getTetrads();

	void print();
};

class edge {
private:
	BasicBlock* destination;

	// Информация о том, является ли ребро - переходом по проверке указателя на nullptr
	bool isNullPtrCheck = false;
	CompareType compareT = CompareType::none;
	std::string variable;
public:
	edge(BasicBlock* dBB);
	BasicBlock* getDestination();

	void setNullPtrCheck(CompareType cT, std::string pointerName);
	bool isNullptrCheck();
	CompareType getCompareType();
	std::string getVarName();
};

class controlFlowGraph {
private:
	std::list<BasicBlock*> cfg;
	std::map<int, BasicBlock*> blocksByLabels;
	std::map<int, std::list<edge*>> edges;

	void initCFG(const std::list<Tetrad*> &pseudoCode);
	void addBasicBlock(const std::list<Tetrad*>& tetrads);
	void addBlockByLabel(BasicBlock* bb);
	void createEdges();
	void createEdgeForJmp(BasicBlock* bb);
	void handleJmpOnFalseEdge(Tetrad* tetrad, edge* e);

	// Информация о предыдущих созданных ребрах-прыжках
	bool prevEdgeIsNullptrCheck = false;
	std::string prevEdgeVariable = "";
	CompareType prevEdgeCompareType = CompareType::none;
	//==================================================

public: 
	controlFlowGraph(const std::list<Tetrad*> &pseudoCode);
	void print();

	const std::list<BasicBlock*>& getBlocks();
	std::map<int, std::list<edge*>>& getEdges();
}; 

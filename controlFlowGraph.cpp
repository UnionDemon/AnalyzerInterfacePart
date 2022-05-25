#include "Tetrad.h"
#include "controlFlowGraph.h"
#include <iostream>

int BasicBlock::counter = 0;

edge::edge(BasicBlock* dBB)
{
	destination = dBB;
}

BasicBlock* edge::getDestination()
{
	return destination;
}

BasicBlock::BasicBlock(const std::list<Tetrad*> &instruct)
{
	instructions = instruct;
	id = counter++;
}

int BasicBlock::getId() {
	return id;
}

const std::list<Tetrad*>& BasicBlock::getTetrads()
{
	return instructions;
}

controlFlowGraph::controlFlowGraph(const std::list<Tetrad*> &pseudoCode)
{
	initCFG(pseudoCode);
}

const std::list<BasicBlock*>& controlFlowGraph::getBlocks()
{
	return cfg;
}
std::map<int, std::list<edge*>>& controlFlowGraph::getEdges()
{
	return edges;
}

void controlFlowGraph::initCFG(const std::list<Tetrad*>& pseudoCode) {
	std::list<Tetrad*> current;
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++)
	{
		if ((*it)->operation == OperationType::label)
		{
			addBasicBlock(current);
			current.clear();
			current.push_back((*it));
			continue;
		}

		if ((*it)->operation == OperationType::returnStmt)
		{
			current.push_back((*it));
			addBasicBlock(current);
			current.clear();
			continue;
		}

		if (((*it)->operation == OperationType::jmp) || ((*it)->operation == OperationType::jmpOnFalse))
		{
			current.push_back((*it));
			addBasicBlock(current);
			current.clear();
			continue;
		}

		current.push_back((*it));
	}
	addBasicBlock(current);
	createEdges();
}

void controlFlowGraph::addBasicBlock(const std::list<Tetrad*>& tetrads) {
	if (tetrads.empty()) {
		return;
	}

	BasicBlock* newBB = new BasicBlock(tetrads);
	cfg.push_back(newBB);

	addBlockByLabel(newBB);
}

std::string controlFlowGraph::print()
{
	std::stringstream output;
	for (auto it = cfg.begin(); it != cfg.end(); it++)
	{
		output << (*it)->print();
	}

	return output.str();
}

std::string controlFlowGraph::printEdges()
{
	std::stringstream output;
	output << "\n\n\nEdges:" << std::endl;
	for (auto it = edges.begin(); it != edges.end(); it++) {
		output << it->first << ": ";
		for (auto e = it->second.begin(); e != it->second.end(); e++) {
			output << " " << (*e)->getDestination()->getId();
			output << " ";
		}
		output << std::endl;
	}
	return output.str();
}

void controlFlowGraph::addBlockByLabel(BasicBlock* bb)
{
	Tetrad* tetrad = *(bb->getTetrads().begin());
	if (tetrad->operation != OperationType::label)
	{
		return;
	}
	int lN = tetrad->labelNumber;
	blocksByLabels[lN] = bb;
}


CompareType invertCompare(CompareType type) {
	if (type == CompareType::none) {
		return CompareType::none;
	}

	if (type == CompareType::eq) {
		return CompareType::ne;
	}

	if (type == CompareType::ne) {
		return CompareType::eq;
	}

	return CompareType::none;
}

void controlFlowGraph::createEdges()
{
	for (auto it = cfg.begin(); it != cfg.end(); it++)
	{
		prevEdgeIsNullptrCheck = false;
		prevEdgeCompareType = CompareType::none;
		prevEdgeVariable = "";

		createEdgeForJmp(*it);

		auto lastTetrad = (*it)->getTetrads().rbegin();
		if (((*lastTetrad)->operation != OperationType::jmp) && ((*lastTetrad)->operation != OperationType::returnStmt)) {
			auto nextBasicBlock = it;
			nextBasicBlock++;

			if (nextBasicBlock == cfg.end()) {
				continue;
			}

			edge* simpleEdge = new edge(*nextBasicBlock);
			if (prevEdgeIsNullptrCheck) {
				simpleEdge->setNullPtrCheck(invertCompare(prevEdgeCompareType), prevEdgeVariable);
			}

			edges[(*it)->getId()].push_back(simpleEdge);
			continue;
		}

	}
}

void controlFlowGraph::createEdgeForJmp(BasicBlock* bb)
{
	auto lastTetrad = bb->getTetrads().rbegin();
	if ((*lastTetrad)->operation == OperationType::jmp || (*lastTetrad)->operation == OperationType::jmpOnFalse)
	{
		int destinationNumber = (*lastTetrad)->labelNumber;
		edge* e = new edge(blocksByLabels[destinationNumber]);
		int sourceBlock = bb->getId();
		edges[sourceBlock].push_back(e);

		handleJmpOnFalseEdge(*lastTetrad, e);
	}
}


void controlFlowGraph::handleJmpOnFalseEdge(Tetrad* tetrad, edge* e)
{
	if (tetrad->operation != OperationType::jmpOnFalse)
	{
		return;
	}
	Operand* operand = *(tetrad->operands.begin());
	if (operand->getTypeOp() != OperandType::ptrNullCheck)
	{
		return;
	}
	e->setNullPtrCheck(invertCompare(operand->getCompareType()), operand->getVarName());
	prevEdgeIsNullptrCheck = true;
	prevEdgeCompareType = invertCompare(operand->getCompareType());
	prevEdgeVariable = operand->getVarName();
}

void edge::setNullPtrCheck(CompareType cT, std::string pointerName)
{
	isNullPtrCheck = true;
	compareT = cT;
	variable = pointerName;
}

bool edge::isNullptrCheck() {
	return isNullPtrCheck;
}
CompareType edge::getCompareType() {
	return compareT;
}
std::string edge::getVarName() {
	return variable;
}

std::string BasicBlock::print()
{
	std::stringstream output;
	output << "Basic Block #" << id << std::endl;
	for (auto it = instructions.begin(); it != instructions.end(); it++)
	{
		output << (*it)->print() << std::endl;
	}
	return output.str();
}


#pragma once
#include "controlFlowGraph.h"
#include "Tetrad.h"
#include <string>
#include <list>
#include <map>



enum class errorType {
	other,
	nullPtrDereference,
	unitializedPointer,
	signedIntegerOverflow
};

struct error {
	errorType type;
	int line;
	int col;
	std::string file_name;
	std::string message;
};

enum class pointerValue
{
	any, 
	null,
	notNull
};

enum class pointerInit
{
	uninitialized,
	initialized
};

class Interpreter {
private:
	controlFlowGraph* graph;
	std::list<error*> errors;

	std::map<int, bool> visitedBlock;
	std::map<std::string, pointerValue> pointers;
	std::map<std::string, pointerInit> pointerInits; 
	void init();
	void walkOnGraph(BasicBlock* bb);
	void interpretTetrads(const std::list<Tetrad*>& tetrads);
	void handleTetrad(Tetrad* tetrad);
	void handleDereference(Tetrad* tetrad);
	void handleDereferenceArrow(Tetrad* tetrad);
	void handleAssign(Tetrad* tetrad);
	void handleSignedIntegerOverflow(Tetrad* tetrad);
	void addPointersToTable(Tetrad* tetrad);
public:
	Interpreter(controlFlowGraph* cfg);
	void run();
	const std::list<error*>& getErrors();
};
#pragma once

#include <list>
#include <unordered_map>
#include "clang/AST/AST.h"
#include "clang/AST/Stmt.h"
#include<string>
#include <sstream>

using namespace clang;

enum class OperationType {
	jmp,
	jmpOnFalse,
	label,
	dereference,
	returnStmt,
	assign,
	lessThan,
	arrowDeref,
	other
};

enum class OperandSource {
	stack,
	object
};

enum class OperandType {
	pointer,
	ptrNullCheck,
	nullptrLiteral,
	address, 
	integer,
	integerSum,
	other
};

enum class CompareType {
	none,
	eq,
	ne
};

class Operand {
private:
	OperandSource source;
	OperandType typeop;
	std::string variable;
	CompareType comparison = CompareType::none;
	Stmt* astNode;
public:
	Operand(OperandSource src, OperandType type, std::string var, Stmt* nd);
	Operand(const Operand& op) {
		*this = op;
	}

	Stmt* getAstNode();
	OperandType getTypeOp();
	std::string getVarName();
	OperandSource getOpSource();
	CompareType getCompareType();

	void setVarName(std::string varName);
	void setTypeOp(OperandType type);
	void setCompareType(CompareType type);

	std::string print();
};

struct Location {
	int line;
	int col;
	std::string fileName;
};

struct Tetrad
{
public:
	OperationType operation;
	std::list<Operand*> operands;
	int labelNumber;

	Location* location;

	std::string print();

	Tetrad();

	void* getAstNode();
	void setAstNode(void* node);



private:
	void* astNode;


};

class pseudoCodeGenerator {
private:
	std::list<Operand*> operandsStack;
	std::list<Tetrad*> pseudoCode;

	void handleValueStmt(ValueStmt* st);
	void handleNullptrLiteral(CXXNullPtrLiteralExpr* st);
	int countChildren(Stmt* st);
	void handleCompoundStmt(CompoundStmt* st);
	void handleIfStmt(IfStmt* st);
	void handleDefaultStatement(Stmt* st);
	int findElse(IfStmt* st);
	int labelCounter = 0;
	void jmpIfStmt(IfStmt* st);
	int findEndIfStmt(IfStmt* st);
	void handleForStmt(ForStmt* st);
	int findOrMakeLabel(Stmt* st);
	void insertForJumpBeforeLabel(int toLabel, int beforeLabel);
	int findEndCycleStmt(Stmt* st);
	void WHILE_makeJumpToCondition(Stmt* st, Stmt* cond);
	int getWhileCondLabel(Stmt* cond);
	void handleWhileStmt(WhileStmt* st);
	Tetrad* makeJmpOnFalseTetrad(void * conditionSt, int labelNumber);
	void handleUnaryOperator(UnaryOperator* unary_op);
	void handleAddress(UnaryOperator* op);
	void handleDeclRefExpr(DeclRefExpr* expr);
	void handleImplicitCastExpr(ImplicitCastExpr* expr);
	void handleNullPtrCheck(BinaryOperator* bin_op, Tetrad* tetrad, Operand* result);
	//void handleRecoveryExpr(RecoveryExpr* expr, Tetrad* tetrad, Operand* result);
	void handleAssignment(BinaryOperator* bin_op);
	void handleAdd(BinaryOperator* bin_op);
	void handleLessThanOrEqualTo(BinaryOperator* bin_op);
	void handleMemberExpr(MemberExpr* expr);
	void handleIntegerLiteral(IntegerLiteral* int_lit);
	void handleReturnStmt(ReturnStmt* st);

	int getOrMakeLabelToSubtreeBeginning(Stmt* subtree);
	int makeLabelAtTheEnd();
	void insertTetradAfterSubtree(Stmt* subtree, Tetrad* tetrad);

	void FOR_STMT_insertJumpsAfterCondSubtree(Stmt* condSubtree, int bodyLabel, int forEndLabel);
	void FOR_STMT_insertJumpAfterIncSubtree(Stmt* incSubtree, int condLabel);
	void FOR_STMT_insertJumpAfterBodySubtree(Stmt* bodySubtree, int incLabel);

	
public:
	void handleStatement(Stmt* st);
	void print();
	std::list<Tetrad*> getPseudoCode();
};
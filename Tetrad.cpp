#include "Tetrad.h"
#include "clang/AST/AST.h"
#include "astComplement.h"
#include "clang/Basic/SourceManager.h"

#include <string>
#include <iostream>
#include <sstream>

// global functions
extern std::string nodeName(Stmt* node);

// global vars
extern astComplement g_ast_complement;
extern ASTContext* g_ast_context;


Tetrad::Tetrad() {
	g_ast_complement.addFakeNode(this);
	astNode = this;
}

void* Tetrad::getAstNode() {
	return astNode;
}

Location* getLocation(Stmt* node) {
	auto begLoc = node->getBeginLoc();
	SourceManager& sm = g_ast_context->getSourceManager();

	auto ploc = sm.getPresumedLoc(begLoc);

	if (ploc.isInvalid()) {
		return nullptr;
	}

	Location* foundLoc = new Location();
	foundLoc->fileName = ploc.getFilename();
	foundLoc->line = ploc.getLine();
	foundLoc->col = ploc.getColumn();

	return foundLoc;
}

void Tetrad::setAstNode(void *astNode_)
{
	astNode = astNode_;

	if (g_ast_complement.isFakeNode(astNode)) {
		return;
	}

	this->location = getLocation(static_cast<Stmt*>(astNode));
}

Operand::Operand(OperandSource src, OperandType type, std::string var, Stmt* nd)
{
	source = src;
	typeop = type;
	variable = var;
	astNode = nd;
}

Stmt* Operand::getAstNode()
{
	return astNode;
}

CompareType Operand::getCompareType() {
	return comparison;
}

OperandType Operand::getTypeOp()
{
	return typeop;
}

std::string Operand::getVarName() {
	return variable;
}

OperandSource Operand::getOpSource() {
	return source;
}

std::list<Tetrad*> pseudoCodeGenerator::getPseudoCode() {
	return pseudoCode;
}

void Operand::setVarName(std::string varName) {
	this->variable = varName;
}
void Operand::setTypeOp(OperandType type) {
	this->typeop = type;
}
void Operand::setCompareType(CompareType type) {
	this->comparison = type;
}

void pseudoCodeGenerator::handleStatement(Stmt* st)
{
	//DEBUG
	std::cout << "DBG: [current node] " << nodeName(st) << std::endl;
	//===========

	if (UnaryOperator* unary_op = dyn_cast<UnaryOperator>(st))
	{
		handleUnaryOperator(unary_op);
		return;
	}
	
	if (DeclRefExpr* decl_ref_expr = dyn_cast<DeclRefExpr>(st))
	{
		handleDeclRefExpr(decl_ref_expr);
		return;
	}
	
	if (ImplicitCastExpr* implicit_cast_expr = dyn_cast<ImplicitCastExpr>(st))
	{
		handleImplicitCastExpr(implicit_cast_expr);
		return;
	}

	if (CXXNullPtrLiteralExpr* nullptr_stmt = dyn_cast<CXXNullPtrLiteralExpr>(st))
	{
		handleNullptrLiteral(nullptr_stmt);
		return;
	}

	if (ValueStmt* val_stmt = dyn_cast<ValueStmt>(st))
	{
		handleValueStmt(val_stmt);
		return;
	}

	if (CompoundStmt* cmpd_stmt = dyn_cast<CompoundStmt>(st))
	{
		handleCompoundStmt(cmpd_stmt);
		return;
	}
	
	if (IfStmt* if_stmt = dyn_cast<IfStmt>(st))
	{
		handleIfStmt(if_stmt);
		return;
	}

	if (ForStmt* for_stmt = dyn_cast<ForStmt>(st))
	{
		handleForStmt(for_stmt);
		return;
	}


	if (WhileStmt* while_stmt = dyn_cast<WhileStmt>(st))
	{
		handleWhileStmt(while_stmt);
		return;
	}


	if (ReturnStmt* return_stmt = dyn_cast<ReturnStmt>(st))
	{
		handleReturnStmt(return_stmt);
		return;
	}

	handleDefaultStatement(st);
}

void pseudoCodeGenerator::handleDefaultStatement(Stmt* st) {
	int childrens = countChildren(st);

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::other;
	tetrad->setAstNode(st);

	//DEBUG
	std::string node = nodeName(st);
	std::cout << "DBG: " << node << std::endl;
	//============

	for (int i = 0; i < childrens; i++) {
		Operand* op = operandsStack.back();
		operandsStack.pop_back();
		tetrad->operands.push_front(op); 
	}

	pseudoCode.push_back(tetrad);
}

void pseudoCodeGenerator::handleReturnStmt(ReturnStmt* st)
{
	int childrens = countChildren(st);

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::returnStmt;
	tetrad->setAstNode(st);

	for (int i = 0; i < childrens; i++) {
		Operand* op = operandsStack.back();
		operandsStack.pop_back();
		tetrad->operands.push_front(op);
	}

	pseudoCode.push_back(tetrad);
}

void pseudoCodeGenerator::handleNullPtrCheck(BinaryOperator* bin_op, Tetrad* tetrad, Operand* result)
{
	CompareType compare;
	if (bin_op->getOpcode() == BO_EQ) {
		compare = CompareType::eq;
	}
	else if (bin_op->getOpcode() == BO_NE)
	{
		compare = CompareType::ne;
	} 
	else 
	{
		return;
	}

	if (tetrad->operands.size() != 2)
	{
		return;
	}
	
	auto operandIt = tetrad->operands.begin();
	if ((*operandIt)->getTypeOp() != OperandType::pointer) {
		return;
	}

	std::string ptrName = (*operandIt)->getVarName();

	operandIt++;
	if ((*operandIt)->getTypeOp() != OperandType::nullptrLiteral) {
		return;
	}

	result->setTypeOp(OperandType::ptrNullCheck);
	result->setVarName(ptrName);
	result->setCompareType(compare);
}

void pseudoCodeGenerator::handleNullptrLiteral(CXXNullPtrLiteralExpr* st) {
	Operand* result = new Operand(OperandSource::object, OperandType::nullptrLiteral, "", st);
	operandsStack.push_back(result);
}
//создаем тетраду с типом Assign
void pseudoCodeGenerator::handleAssignment(BinaryOperator* bin_op) {
	int childrens = countChildren(bin_op);

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::assign;
	tetrad->setAstNode(bin_op);

	for (int i = 0; i != childrens; i++)
	{
		Operand* op = operandsStack.back();
		operandsStack.pop_back();
		tetrad->operands.push_front(op);
	}

	pseudoCode.push_back(tetrad);

	Operand* result = new Operand(OperandSource::stack, OperandType::other, "", bin_op);
	operandsStack.push_back(result);
}

//создаем тетраду с типом Add
void pseudoCodeGenerator::handleAdd(BinaryOperator* bin_op) {
	int childrens = countChildren(bin_op);

	Tetrad* tetrad = new Tetrad();
	tetrad->setAstNode(bin_op);
	//выт€гиваем из стека первый и второй операнды, кладем их в тетраду
	Operand* firstOp = operandsStack.back();
	operandsStack.pop_back();
	tetrad->operands.push_front(firstOp);

	Operand* secondOp = operandsStack.back();
	operandsStack.pop_back();
	tetrad->operands.push_front(secondOp);
	tetrad->operation = OperationType::other;

	pseudoCode.push_back(tetrad);

	Operand* result = new Operand(OperandSource::stack, OperandType::other, "", bin_op);
	//если первый и второй операнды тетрады имеют тип integer, то кладем в стек результат операции и присваиваем OperandType - integerSum
	if ((firstOp->getTypeOp() == OperandType::integer) && ((secondOp->getTypeOp() == OperandType::integer)))
	{
		result->setTypeOp(OperandType::integerSum);
	}
	operandsStack.push_back(result);
}
//обрабатываем операции меньше и меньше или равно - присваиваем OperationType - lessThan
void pseudoCodeGenerator::handleLessThanOrEqualTo(BinaryOperator* bin_op)
{
	int childrens = countChildren(bin_op);

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::lessThan;
	tetrad->setAstNode(bin_op);

	for (int i = 0; i != childrens; i++)
	{
		Operand* op = operandsStack.back();
		operandsStack.pop_back();
		tetrad->operands.push_front(op);
	}

	pseudoCode.push_back(tetrad);

	Operand* result = new Operand(OperandSource::stack, OperandType::other, "", bin_op);
	operandsStack.push_back(result);
}

void pseudoCodeGenerator::handleIntegerLiteral(IntegerLiteral* int_lit)
{
	Operand* result = new Operand(OperandSource::object, OperandType::integer, "", int_lit);
	operandsStack.push_back(result);
}

void pseudoCodeGenerator::handleMemberExpr(MemberExpr* expr)
{
	int childrens = countChildren(expr);

	Tetrad* tetrad = new Tetrad();
	if (expr->isArrow())
	{
		tetrad->operation = OperationType::arrowDeref;
	}
	else
	{
		tetrad->operation = OperationType::other;
	}
	
	tetrad->setAstNode(expr);

	for (int i = 0; i != childrens; i++)
	{
		Operand* op = operandsStack.back();
		operandsStack.pop_back();
		tetrad->operands.push_front(op);
	}

	pseudoCode.push_back(tetrad);

	Operand* result = new Operand(OperandSource::stack, OperandType::other, "", expr);
	operandsStack.push_back(result);
}


void pseudoCodeGenerator::handleValueStmt(ValueStmt* st)
{
	//проверка на присваивание - создание новой тетрады с типом Assign
	if (BinaryOperator* bin_op = dyn_cast<BinaryOperator>(st))
	{
		if (bin_op->getOpcode() == BO_Assign) {
			handleAssignment(bin_op);
			return;
		}
		if (bin_op->getOpcode() == BO_Add)
		{
			handleAdd(bin_op);
			return;
		}
		if ((bin_op->getOpcode() == BO_LE) || (bin_op->getOpcode() == BO_LT))
		{
			handleLessThanOrEqualTo(bin_op);
			return;
		}
	}
	
	if (IntegerLiteral* int_lit = dyn_cast<IntegerLiteral>(st))
	{
		handleIntegerLiteral(int_lit);
		return;
	}

	if (MemberExpr* memb_expr = dyn_cast<MemberExpr>(st))
	{
		handleMemberExpr(memb_expr);
		return;
	}

	int childrens = countChildren(st);
	
	if (childrens == 0)
	{
		Operand* op = new Operand(OperandSource::object, OperandType::other, "", st);
		operandsStack.push_back(op);
		return;
	}

	Tetrad* tetrad = new Tetrad();

	tetrad->operation = OperationType::other;
	tetrad->setAstNode(st);

	for (int i = 0; i != childrens; i++)
	{
		Operand* op = operandsStack.back();
		operandsStack.pop_back();
		tetrad->operands.push_front(op);
	}

	pseudoCode.push_back(tetrad);

	Operand* result = new Operand(OperandSource::stack, OperandType::other, "", st);

	if (BinaryOperator* bin_op = dyn_cast<BinaryOperator>(st))
	{
		handleNullPtrCheck(bin_op, tetrad, result);
	}

	operandsStack.push_back(result);
}

int pseudoCodeGenerator::countChildren(Stmt* st)
{
	int childrenCount = 0;
	for (auto i = st->child_begin(); i != st->child_end(); i++)
	{
		childrenCount++;
	}
	return childrenCount;
}

void pseudoCodeGenerator::handleCompoundStmt(CompoundStmt* st)
{
	
}

void pseudoCodeGenerator::handleIfStmt(IfStmt* st)
{
	int labelNumberElse = findElse(st);
	Stmt* condStmt = (Stmt*)st->getCond();

	//¬ставить jumpFalse после услови€
	void* endOfCond = g_ast_complement.findLastSubtreeNode(condStmt);
	makeJmpOnFalseTetrad(endOfCond, labelNumberElse);

	//¬ставить прыжок на конец условного оператора в конце then
	jmpIfStmt(st);
}

int pseudoCodeGenerator::findElse(IfStmt* st)
{
	Stmt* elseStmt = st->getElse();
	if (elseStmt == nullptr)
	{
		return findEndIfStmt(st);
	}
	
	void* firstElseStmt = g_ast_complement.findFirstSubtreeNode(elseStmt);

	auto firstElseTetrad = pseudoCode.end();

	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++)
	{
		if ((*it)->getAstNode() == firstElseStmt)
		{
			firstElseTetrad = it;
			break;
		}
	}

	if (firstElseTetrad == pseudoCode.end()) {
		return -1;
	}

	if ((*firstElseTetrad)->operation == OperationType::label)
	{
		return (*firstElseTetrad)->labelNumber;
	}

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::label;
	tetrad->labelNumber = labelCounter++;
	pseudoCode.insert(firstElseTetrad, tetrad);

	g_ast_complement.addPredecessor(firstElseStmt, tetrad);

	return tetrad->labelNumber;
}

// ¬ставл€ет прыжок на окончание условного оператора в конце then
void pseudoCodeGenerator::jmpIfStmt(IfStmt* st)
{
	Stmt* elseStmt = st->getElse();
	if (elseStmt == nullptr)
	{
		return;
	}

	Stmt* thenStmt = st->getThen();
	if (thenStmt == nullptr) {
		std::cout << "\n\n ============ ERROR: NO THEN STMT ==========\n\n";
		return;
	}

	void* lastThenStatement = g_ast_complement.findLastSubtreeNode(thenStmt);
	auto endOfThenStIterator = pseudoCode.end();
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++) {
		if ((*it)->getAstNode() == lastThenStatement) {
			endOfThenStIterator = it;
			break;
		}
	}

	if (endOfThenStIterator == pseudoCode.end()) {
		std::cout << "\n\n ============ ERROR: END  OF THEN STMT NOT FOUND ==========\n\n";
		return;
	}

	int labelNumberEnd = findEndIfStmt(st);
	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::jmp;
	tetrad->labelNumber = labelNumberEnd;

	endOfThenStIterator++;
	pseudoCode.insert(endOfThenStIterator, tetrad);

	g_ast_complement.addFollower(lastThenStatement, tetrad);
}

//конец IfStmt
int pseudoCodeGenerator::findEndIfStmt(IfStmt* st)
{
	auto endTetrad = pseudoCode.end();

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::label;
	tetrad->labelNumber = labelCounter++;
	pseudoCode.insert(endTetrad, tetrad);

	g_ast_complement.addFollower(st, tetrad);

	return tetrad->labelNumber;
}

void pseudoCodeGenerator::insertTetradAfterSubtree(Stmt* subtree, Tetrad* tetrad) {
	void* endOfsubtree = g_ast_complement.findLastSubtreeNode(subtree);

	if (endOfsubtree == nullptr) {
		std::cout << "\n\n\n ======= ERROR: could not find end of subtree for inserting tetrad after ========\n\n\n";
		return;
	}

	auto subtreeEndIterator = pseudoCode.end();

	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++) {
		if ((*it)->getAstNode() == endOfsubtree) {
			subtreeEndIterator = it;
			break;
		}
	}

	subtreeEndIterator++;
	pseudoCode.insert(subtreeEndIterator, tetrad);

	g_ast_complement.addFollower(endOfsubtree, tetrad);
}

void pseudoCodeGenerator::FOR_STMT_insertJumpAfterBodySubtree(Stmt* bodySubtree, int incLabel) {
	void* endOfBody = g_ast_complement.findLastSubtreeNode(bodySubtree);

	auto endOfBodyIterator = pseudoCode.end();
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++) {
		if ((*it)->getAstNode() == endOfBody) {
			endOfBodyIterator = it;
			break;
		}
	}

	if (endOfBodyIterator == pseudoCode.end()) {
		std::cout << "\n\n\n ============ ERROR: could not find FOR BODY end to insert jump after it ===========\n\n\n";
		return;
	}

	endOfBodyIterator++;
	Tetrad* jmpTetrad = new Tetrad();
	jmpTetrad->operation = OperationType::jmp;
	jmpTetrad->labelNumber = incLabel;
	pseudoCode.insert(endOfBodyIterator, jmpTetrad);
	
	g_ast_complement.addFollower(endOfBody, jmpTetrad);
}

void pseudoCodeGenerator::FOR_STMT_insertJumpAfterIncSubtree(Stmt* incSubtree, int condLabel) {
	Tetrad* jmpTetrad = new Tetrad();
	jmpTetrad->operation = OperationType::jmp;
	jmpTetrad->labelNumber = condLabel;
	insertTetradAfterSubtree(incSubtree, jmpTetrad);
}

// ¬ставить прыжок после услови€ - jmpFalse на конец цикла, а затем jmp на тело цикла
void pseudoCodeGenerator::FOR_STMT_insertJumpsAfterCondSubtree(Stmt* condSubtree, int bodyLabel, int forEndLabel) {
	void* endOfCond = g_ast_complement.findLastSubtreeNode(condSubtree);
	
	// ¬ставить после cond прыжок jmpFalse на конец всего цикла
	Tetrad* jmpFalseTetrad = makeJmpOnFalseTetrad(endOfCond, forEndLabel);
	
	auto endOfCondIterator = pseudoCode.end();
	for(auto it = pseudoCode.begin(); it != pseudoCode.end(); it++) {
		if ((*it)->getAstNode() == endOfCond) {
			endOfCondIterator = it;
			break;
		}
	}

	if (endOfCondIterator == pseudoCode.end()) {
		std::cout << "\n\n\n ========== ERROR: could not find end of FOR COND ========= \n\n\n";
		return;
	}

	endOfCondIterator++;

	// ѕосле конца услови€ мы уже вставили один прыжок, новый нужно вставить после него
	endOfCondIterator++;

	Tetrad* jmpTetrad = new Tetrad();
	jmpTetrad->operation = OperationType::jmp;
	jmpTetrad->labelNumber = bodyLabel;
	pseudoCode.insert(endOfCondIterator, jmpTetrad);

	g_ast_complement.addFollower(jmpFalseTetrad, jmpTetrad);
}

int pseudoCodeGenerator::makeLabelAtTheEnd() {
	Tetrad* labelTetrad = new Tetrad();
	labelTetrad->operation = OperationType::label;
	labelTetrad->labelNumber = labelCounter++;
	pseudoCode.push_back(labelTetrad);

	return labelTetrad->labelNumber;
}

//обработка ForStmt
void pseudoCodeGenerator::handleForStmt(ForStmt* st)
{
	int condLabelNumber = getOrMakeLabelToSubtreeBeginning(st->getCond());
	int incLabelNumber = getOrMakeLabelToSubtreeBeginning(st->getInc());
	int bodyLabelNumber = getOrMakeLabelToSubtreeBeginning(st->getBody());
	int endOfForLabelNumber = makeLabelAtTheEnd();

	FOR_STMT_insertJumpsAfterCondSubtree(st->getCond(), bodyLabelNumber, endOfForLabelNumber);
	FOR_STMT_insertJumpAfterIncSubtree(st->getInc(), condLabelNumber);
	FOR_STMT_insertJumpAfterBodySubtree(st->getBody(), incLabelNumber);
}

int pseudoCodeGenerator::getOrMakeLabelToSubtreeBeginning(Stmt* subtree) {
	void* beginningStmt = g_ast_complement.findFirstSubtreeNode(subtree);

	auto beginningIt = pseudoCode.end();
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++) {
		if ((*it)->getAstNode() == beginningStmt) {
			beginningIt = it;
			break;
		}
	}

	if (beginningIt == pseudoCode.end()) {
		std::cout << "\n\n\n ========= ERROR: CANNOT FIND BEGINNING OF SUBTREE IN PSEUDOCODE ======= \n\n\n";
		return -1;
	}

	if ((*beginningIt)->operation == OperationType::label) {
		return (*beginningIt)->labelNumber;
	}

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::label;
	tetrad->labelNumber = labelCounter++;
	pseudoCode.insert(beginningIt, tetrad);

	g_ast_complement.addPredecessor(beginningStmt, tetrad);

	return tetrad->labelNumber;
}

int pseudoCodeGenerator::findOrMakeLabel(Stmt* st)
{
	auto bodyBegin = pseudoCode.end();
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++)
	{
		if ((*it)->getAstNode() == st)
		{
			bodyBegin = it;
			break;
		}
	}

	if (bodyBegin == pseudoCode.end())
	{
		return -1;
	}

	if ((*bodyBegin)->operation == OperationType::label)
	{
		return (*bodyBegin)->labelNumber;
	}

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::label;
	tetrad->labelNumber = labelCounter++;
	pseudoCode.insert(bodyBegin, tetrad);

	g_ast_complement.addPredecessor(st, tetrad);

	return tetrad->labelNumber;
}

void pseudoCodeGenerator::insertForJumpBeforeLabel(int toLabel, int beforeLabel)
{
	auto beforeLabelIterator = pseudoCode.end();
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++)
	{
		if (((*it)->labelNumber == beforeLabel) && (*it)->operation == OperationType::label)
		{
			beforeLabelIterator = it;
			break;
		}
	}
	if (beforeLabelIterator == pseudoCode.end())
	{
		return;
	}
	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::jmp;
	tetrad->labelNumber = toLabel;
	pseudoCode.insert(beforeLabelIterator, tetrad);

	g_ast_complement.addPredecessor(*beforeLabelIterator, tetrad);
}

//конец WhileStmt
int pseudoCodeGenerator::findEndCycleStmt(Stmt* st)
{
	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::label;
	tetrad->labelNumber = labelCounter++;
	pseudoCode.push_back(tetrad);

	g_ast_complement.addFollower(st, tetrad);

	return tetrad->labelNumber;
}


void pseudoCodeGenerator::WHILE_makeJumpToCondition(Stmt* st, Stmt* cond)
{
	//вставл€ем label перед cond
	int labelNumberJmp = getWhileCondLabel(cond);
	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::jmp;
	tetrad->labelNumber = labelNumberJmp;

	pseudoCode.push_back(tetrad);

	g_ast_complement.addFollower(st, tetrad);
}


int pseudoCodeGenerator::getWhileCondLabel(Stmt* cond)
{
	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::label;
	tetrad->labelNumber = labelCounter++;
	
	void* condBeginning = g_ast_complement.findFirstSubtreeNode(cond);

	auto condBeginningIterator = pseudoCode.end();
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++)
	{
		if ((*it)->getAstNode() == condBeginning)
		{
			condBeginningIterator = it;
			break;
		}
	}
	if (condBeginningIterator == pseudoCode.end())
	{
		std::cout << "\n\n\n ========== ERROR: couldnot find while_cond beginnin in pseudocode ======== \n\n\n";
		return tetrad->labelNumber;
	}

	g_ast_complement.addPredecessor(condBeginning, tetrad);

	pseudoCode.insert(condBeginningIterator, tetrad);
	return tetrad->labelNumber;
}


//обработка WhileStmt
void pseudoCodeGenerator::handleWhileStmt(WhileStmt* st)
{
	Stmt* condStmt = (Stmt*)st->getCond();
	WHILE_makeJumpToCondition(st->getBody(), condStmt);

	int labelNumberEndWhileStmt = findEndCycleStmt(st->getBody());
	makeJmpOnFalseTetrad(condStmt, labelNumberEndWhileStmt);
}


// ¬ставить jumpOnFalse на labelNumber после st
Tetrad* pseudoCodeGenerator::makeJmpOnFalseTetrad(void* conditionSt, int labelNumber)
{
	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::jmpOnFalse;
	tetrad->labelNumber = labelNumber;

	//выдернуть операнд из стека
	for (auto it = operandsStack.begin(); it != operandsStack.end(); it++)
	{
		if ((*it)->getAstNode() == conditionSt)
		{
			Operand* conditionVal = new Operand(**it);
			operandsStack.erase(it);
			tetrad->operands.push_back(conditionVal);
			break;
		}
	}
	auto cond = pseudoCode.end();
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++)
	{
		if ((*it)->getAstNode() == conditionSt) {
			cond = it;
			break;
		}
	}
	if (cond == pseudoCode.end()) {
		std::cout << "\n\n\n =========== ERROR: could not find statement to makeJmpOnFalseTetrad =============== \n\n\n";
		return nullptr;
	}
	cond++;
	pseudoCode.insert(cond, tetrad);

	g_ast_complement.addFollower(conditionSt, tetrad);

	return tetrad;
}
//закидываем в стек операнд с типом address
void pseudoCodeGenerator::handleAddress(UnaryOperator* op)
{
	Operand* operand = operandsStack.back();
	operandsStack.pop_back();

	Operand* result = new Operand(OperandSource::object, OperandType::address, "", op);
	operandsStack.push_back(result);
}

void pseudoCodeGenerator::handleUnaryOperator(UnaryOperator* op)
{
	//обработка адреса
	if (op->getOpcode() == UO_AddrOf) {
		handleAddress(op);
		return;
	}
	int childrens = countChildren(op);
	Tetrad* tetrad = new Tetrad();
	//обработка разыменований
	if (op->getOpcode() == UO_Deref) {
		tetrad->operation = OperationType::dereference;
	} 
	else
	{
		tetrad->operation = OperationType::other;
	}
	tetrad->setAstNode(op);

	for (int i = 0; i < childrens; i++) {
		Operand* op = operandsStack.back();
		operandsStack.pop_back();
		tetrad->operands.push_front(op);
	}

	pseudoCode.push_back(tetrad);

	Operand* result = new Operand(OperandSource::stack, OperandType::other, "", op);
	operandsStack.push_back(result);
}

void pseudoCodeGenerator::handleDeclRefExpr(DeclRefExpr* expr)
{
	auto* varType = expr->getType().getTypePtrOrNull();
	std::string variable = expr->getNameInfo().getAsString();
	if (varType != nullptr && varType->isPointerType() == true)
	{
		Operand* result = new Operand(OperandSource::stack, OperandType::pointer, variable, expr);
		operandsStack.push_back(result);
	} 
	else if (expr->getDecl()->getType()->isIntegerType() == true) //провер€ем имеет ли операнд тип integer
	{
		Operand* result = new Operand(OperandSource::stack, OperandType::integer, variable, expr); 
		operandsStack.push_back(result);
	} 
	else {
		Operand* result = new Operand(OperandSource::stack, OperandType::other, variable, expr);
		operandsStack.push_back(result);
	}
}

void pseudoCodeGenerator::handleImplicitCastExpr(ImplicitCastExpr* expr)
{
	Operand* op = operandsStack.back();
	operandsStack.pop_back();

	std::string resultVarName = "";
	OperandType resultOperandType = OperandType::other;

	if (op->getTypeOp() == OperandType::pointer)
	{
		resultOperandType = OperandType::pointer;
		resultVarName = op->getVarName();
	}
	else if (op->getTypeOp() == OperandType::nullptrLiteral) 
	{
		resultOperandType = OperandType::nullptrLiteral;
	} 
	else if (op->getTypeOp() == OperandType::integer) //передаем переменную типа integer в тетраду ImplicitCastExpr
	{
		resultOperandType = OperandType::integer;
		resultVarName = op->getVarName();
	}

	Tetrad* tetrad = new Tetrad();
	tetrad->operation = OperationType::other;
	tetrad->setAstNode(expr);
	tetrad->operands.push_back(op);

	pseudoCode.push_back(tetrad);

	Operand* result = new Operand(OperandSource::stack, resultOperandType, resultVarName, expr);
	operandsStack.push_back(result);
}

void pseudoCodeGenerator::print()
{
	for (auto it = pseudoCode.begin(); it != pseudoCode.end(); it++) {
		(*it)->print();
	}
}

std::string Tetrad::print()
{
	std::stringstream output;
	if (!g_ast_complement.isFakeNode(astNode)) {
		Stmt* realStmt = static_cast<Stmt*>(astNode);
		output << nodeName(realStmt) << " ";
	}

	if (operation == OperationType::jmp) {
		output << "jmp " << labelNumber << " ";
	}
	if (operation == OperationType::jmpOnFalse) {
		output << "jmpOnFalse " << labelNumber << " ";
	}
	if (operation == OperationType::label) {
		output << "label "<< labelNumber << " ";
	}
	if (operation == OperationType::dereference) {
		output << "dereference " << " ";
	}
	if (operation == OperationType::returnStmt) {
		output << "return " << " ";
	}
	if (operation == OperationType::assign) {
		output << "assign " << " ";
	}
	if (operation == OperationType::lessThan) {
		output << "lessThan " << " ";
	}
	if (operation == OperationType::arrowDeref) {
		output << "arrowDeref " << " ";
	}
	if (operation == OperationType::other) {
		output << "other ";
	}
	for (auto it = operands.begin(); it != operands.end(); it++) {
		output << (*it)->print();
	}
	return output.str();
}

std::string Operand::print()
{
	std::stringstream output;
	output << "[";
	output << nodeName(astNode) << " ";
	if (source == OperandSource::stack)
	{
		output << "stack ";
	}
	if (source == OperandSource::object)
	{
		output << "object ";
	}
	if (typeop == OperandType::other)
	{
		output << "other ";
	}
	if (typeop == OperandType::pointer)
	{
		output << "pointer ";
	}
	if (typeop == OperandType::integer)
	{
		output << "integer ";
	}
	if (typeop == OperandType::nullptrLiteral)
	{
		output << "nullptrLiteral ";
	}
	if (typeop == OperandType::ptrNullCheck)
	{
		output << "ptrNullCheck ";
	}
	if (typeop == OperandType::address)
	{
		output << "address ";
	}
	if (typeop == OperandType::integerSum) {
		output << "integerSum " << " ";
	}
	if (comparison == CompareType::eq)
	{
		output << "equal ";
	}
	if (comparison == CompareType::ne)
	{
		output << "not equal ";
	}
	output << variable << " ";
	output << "]";
	return output.str();
}
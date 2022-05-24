#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/ASTConsumers.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "Tetrad.h"
#include "controlFlowGraph.h"
#include "Interpreter.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <list>

using namespace std;
using namespace clang;
using namespace clang::driver;
using namespace clang::tooling;
using namespace llvm;

extern pseudoCodeGenerator g_codegenerator;
extern ASTContext* g_ast_context;

class ExampleVisitor : public RecursiveASTVisitor<ExampleVisitor> {
private:
    ASTContext* astContext; // used for getting additional AST info
    pseudoCodeGenerator* generator;
public:
    explicit ExampleVisitor(CompilerInstance* CI)
        : astContext(&(CI->getASTContext())) // initialize private members
    {
        generator = &g_codegenerator;
        g_ast_context = astContext;
    }

    virtual bool VisitStmt(Stmt* st) {
        generator->handleStatement(st);
        
        return true;
    }

    bool shouldTraversePostOrder() const {
        return true;
    }  
};

class ExampleASTConsumer : public ASTConsumer {
private:
    ExampleVisitor* visitor; // doesn't have to be private

public:
    // override the constructor in order to pass CI
    explicit ExampleASTConsumer(CompilerInstance* CI)
        : visitor(new ExampleVisitor(CI)) // initialize the visitor
    {}

    // override this to call our ExampleVisitor on the entire source file
    virtual void HandleTranslationUnit(ASTContext& Context) {
        /* we can use ASTContext to get the TranslationUnitDecl, which is
             a single Decl that collectively represents the entire source file */
        visitor->TraverseDecl(Context.getTranslationUnitDecl());
    }
};

class ExampleFrontendAction : public ASTFrontendAction {
public:
	virtual std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance& CI,
		StringRef file) {
		return std::make_unique<ExampleASTConsumer>(
			&CI); // pass CI pointer to ASTConsumer
	}
};

/*
static cl::OptionCategory MyToolCategory("My tool options");

int main(int argc, const char** argv) {
    // parse the command-line args passed to your code
    llvm::Expected<CommonOptionsParser> op =
        CommonOptionsParser::create(argc, argv, MyToolCategory);
    // create a new Clang Tool instance (a LibTooling environment)
    ClangTool Tool(op->getCompilations(), op->getSourcePathList());

    // run the Clang Tool, creating a new FrontendAction (explained below)
    auto actionFactory = newFrontendActionFactory<ExampleFrontendAction>();
    int result = Tool.run(actionFactory.get());
    g_codegenerator.print();
    controlFlowGraph cfg(g_codegenerator.getPseudoCode());
    cfg.print();
    Interpreter interpreter(&cfg);
    interpreter.run();
    for (auto it = interpreter.getErrors().begin(); it != interpreter.getErrors().end(); it++)
    {
        std::cout << (*it)->location << " : " << (*it)->message << std::endl;
    }
    return result;
}*/

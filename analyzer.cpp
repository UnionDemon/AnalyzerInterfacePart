#include "analyzer.h"
#include "ui_analyzer.h"

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

#include <sstream>

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

static cl::OptionCategory MyToolCategory("My tool options");

void Analyzer::AnalyzerMain(int argc, const char** argv) {
    // parse the command-line args passed to your code
        llvm::Expected<CommonOptionsParser> op =
            CommonOptionsParser::create(argc, argv, MyToolCategory);
        // create a new Clang Tool instance (a LibTooling environment)
        ClangTool Tool(op->getCompilations(), op->getSourcePathList());

        // run the Clang Tool, creating a new FrontendAction (explained below)
        auto actionFactory = newFrontendActionFactory<ExampleFrontendAction>();
        int result = Tool.run(actionFactory.get());
        g_codegenerator.print();
        std::list<Tetrad*> tetrads = g_codegenerator.getPseudoCode();
        showTetrads(g_codegenerator.getPseudoCode());
        controlFlowGraph cfg(g_codegenerator.getPseudoCode());
        cfg.print();
        Interpreter interpreter(&cfg);
        interpreter.run();
        showErrors(interpreter.getErrors());
}

//========================================================================================

Analyzer::Analyzer(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Analyzer)
{
    ui->setupUi(this);
    model = new QStandardItemModel(0,4,this);
    model->setHeaderData(0, Qt::Horizontal, "File path");
    model->setHeaderData(1, Qt::Horizontal, "Line");
    model->setHeaderData(2, Qt::Horizontal, "Column");
    model->setHeaderData(3, Qt::Horizontal, "Message");
    ui->tableView->setModel(model);

    model1 = new QStandardItemModel(0,1,this);
    model1->setHeaderData(0, Qt::Horizontal, "Tetrad List");
    /*model->setHeaderData(1, Qt::Horizontal, "Operation type");
    model->setHeaderData(2, Qt::Horizontal, "First operand");
    model->setHeaderData(3, Qt::Horizontal, "First operand type");
    model->setHeaderData(4, Qt::Horizontal, "Second operand");
    model->setHeaderData(5, Qt::Horizontal, "Second operand type");*/
    ui->tableView_2->setModel(model1);
}

Analyzer::~Analyzer()
{
    delete ui;
}


void Analyzer::on_pushButton_2_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,"Открыть","..","File c++ (*.cpp)");

    if(!file_name.isNull())
        this->ui->lineEdit->setText(file_name);

}


void Analyzer::on_pushButton_clicked()
{
    QString file_name = this->ui->lineEdit->text();
    QFile program = QFile(file_name);
    program.open(QIODevice::ReadOnly | QIODevice::Text);
    QString data_file = QString(program.readAll());

    QByteArray arr = file_name.toLocal8Bit();
    const char * file = arr.constData();

    std::cout << file << std::endl;

    const char* argv[] = {
        "static_analyzer.exe",
        file
    };

    int argc = 2;

    AnalyzerMain(argc, argv);
    //ui->textEdit->setText(QString::fromLocal8Bit(result));
    ui->textEdit->setText(data_file);
}

void Analyzer::showErrors(std::list<error*> errors)
{
    QStandardItem* item = 0;
     int i = 0;
     for (auto& it:errors)
     {
        model->insertRow(model->rowCount());
        item = new QStandardItem(QString::fromLocal8Bit((it)->file_name));
        model->setItem(i,0,item);
        item = new QStandardItem(QString::number((it)->line));
        model->setItem(i,1,item);
        item = new QStandardItem(QString::number((it)->col));
        model->setItem(i,2,item);
        item = new QStandardItem(QString::fromLocal8Bit((it)->message));
        model->setItem(i,3,item);
        i++;
     }
        ui->tableView->setModel(model);
        ui->tableView->show();
        ui->tableView->resizeColumnsToContents();
        ui->tableView->horizontalHeader()->setStretchLastSection(true);
        ui->tableView->setShowGrid(true);
        ui->tableView->setGridStyle(Qt::DashLine);
}

void Analyzer::showTetrads(std::list<Tetrad*> tetrads)
{
    QStandardItem* item = 0;
     int i = 0;
     for (auto& it:tetrads)
     {
        model1->insertRow(model1->rowCount());
        QString tetradInfo =  QString::fromStdString((it)->print());
        item = new QStandardItem(tetradInfo);
        model1->setItem(i,0,item);
        /*
        item = new QStandardItem(QString::number((it)->line));
        model->setItem(i,1,item);
        item = new QStandardItem(QString::number((it)->col));
        model->setItem(i,2,item);
        item = new QStandardItem(QString::fromLocal8Bit((it)->message));
        model->setItem(i,3,item);*/
        i++;
     }
        ui->tableView_2->setModel(model1);
        ui->tableView_2->show();
        ui->tableView_2->resizeColumnsToContents();
        ui->tableView_2->horizontalHeader()->setStretchLastSection(true);
        ui->tableView_2->setShowGrid(true);
        ui->tableView_2->setGridStyle(Qt::DashLine);
}

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
#include <map>

#include <sstream>

#include <QDesktopServices>

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
        showBasicBlocks(cfg.getBlocks());
        showEdges(cfg.getEdges());
        Interpreter interpreter(&cfg);
        interpreter.run();
        showErrors(interpreter.getErrors());
}

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
    ui->tableView_2->setModel(model1);

    model2 = new QStandardItemModel(0,1,this);
    model2->setHeaderData(0, Qt::Horizontal, "Basic Block List");
    ui->tableView_4->setModel(model2);

    model3 = new QStandardItemModel(0,3,this);
    model3->setHeaderData(0, Qt::Horizontal, "From Basic Block");
    model3->setHeaderData(1, Qt::Horizontal, "To Basic Block");
    model3->setHeaderData(2, Qt::Horizontal, "To Basic Block");
    ui->tableView_3->setModel(model3);
}

Analyzer::~Analyzer()
{
    delete ui;
}


void Analyzer::on_pushButton_2_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this,"Открыть","..","File c/c++ (*.cpp *.c)");

    if(!file_name.isNull())
        this->ui->lineEdit->setText(file_name);

}


void Analyzer::on_pushButton_clicked()
{
    QString file_name = this->ui->lineEdit->text();

    QFile inputFile(file_name);
    int i=1;
    if (inputFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
       QTextStream in(&inputFile);
       QString code;

       while (!in.atEnd())
       {
          code += QString::number(i)+'\t'+in.readLine()+'\n';
          i++;
       }
       ui->textEdit->setText(code);
       inputFile.close();
    }

    QByteArray arr = file_name.toLocal8Bit();
    const char * file = arr.constData();

    std::cout << file << std::endl;

    const char* argv[] = {
        "static_analyzer.exe",
        file
    };

    int argc = 2;

    AnalyzerMain(argc, argv);
}

void Analyzer::showErrors(std::list<error*> errors)
{
    QStandardItem* item = 0;
     int i = 0;
     std::list<int> lineNumbers;
     for (auto& it:errors)
     {
        model->insertRow(model->rowCount());
        item = new QStandardItem(QString::fromLocal8Bit((it)->file_name));
        model->setItem(i,0,item);
        item = new QStandardItem(QString::number((it)->line));
        model->setItem(i,1,item);

        int lineNumber = (it)->line;
        lineNumber--;
        lineNumbers.push_back(lineNumber);
        errorBackground(lineNumbers);

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
        i++;
     }
        ui->tableView_2->setModel(model1);
        ui->tableView_2->show();
        ui->tableView_2->resizeColumnsToContents();
        ui->tableView_2->horizontalHeader()->setStretchLastSection(true);
        ui->tableView_2->setShowGrid(true);
        ui->tableView_2->verticalHeader()->setVisible(false);
}

void Analyzer::showBasicBlocks(std::list<BasicBlock*> basicBlocks)
{
    QStandardItem* item = 0;
     int i = 0;
     for (auto& it:basicBlocks)
     {
        model2->insertRow(model2->rowCount());
        QString BBInfo =  QString::fromStdString((it)->print());
        item = new QStandardItem(BBInfo);
        model2->setItem(i,0,item);
        i++;
     }
        ui->tableView_4->setModel(model2);
        ui->tableView_4->show();
        ui->tableView_4->resizeColumnsToContents();
        ui->tableView_4->resizeRowsToContents();
        ui->tableView_4->horizontalHeader()->setStretchLastSection(true);
        ui->tableView_4->setShowGrid(true);
        ui->tableView_4->verticalHeader()->setVisible(false);
}

void Analyzer::showEdges(std::map<int, std::list<edge*>>& edges)
{
    QStandardItem* item = 0;
     int i = 0;
     for (auto& it:edges)
     {
        model3->insertRow(model3->rowCount());
        item = new QStandardItem(QString::number(it.first));
        model3->setItem(i,0,item);

        auto firstIt = it.second.begin();
        auto secondIt = firstIt;
        secondIt++;

        item = new QStandardItem(QString::number((*firstIt)->getDestination()->getId()));
        model3->setItem(i,1,item);

        if (secondIt != it.second.end())
        {
            item = new QStandardItem(QString::number((*secondIt)->getDestination()->getId()));
            model3->setItem(i,2,item);
        }

        i++;
     }
        ui->tableView_3->setModel(model3);
        ui->tableView_3->show();
        ui->tableView_3->resizeColumnsToContents();
        //ui->tableView_3->horizontalHeader()->setStretchLastSection(true);
        ui->tableView_3->setShowGrid(true);
        ui->tableView_3->verticalHeader()->setVisible(false);
}

void Analyzer::errorBackground(std::list<int> lineNumbers)
{
    QList<QTextEdit::ExtraSelection> selection;
    QTextEdit::ExtraSelection ex;
    QTextCursor cursor = QTextCursor(ui->textEdit->document());
    for (auto it:lineNumbers)
    {
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, it);
        cursor.select(QTextCursor::LineUnderCursor);
        ex.cursor = cursor;
        QTextCharFormat format;
        format.setBackground(Qt::yellow);
        ex.cursor = cursor;
        ex.format = format;
        QList<QTextEdit::ExtraSelection>() << selection;
        selection.append(ex);
    }
    ui->textEdit->setExtraSelections(selection);
}

void Analyzer::on_action_triggered()
{
    QString link="D:\\analyzer\\help.html";
    QDesktopServices::openUrl(QUrl::fromLocalFile(link));
}


void Analyzer::on_action_2_triggered()
{
    QString link="D:\\analyzer\\about.html";
    QDesktopServices::openUrl(QUrl::fromLocalFile(link));
}


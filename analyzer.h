#ifndef ANALYZER_H
#define ANALYZER_H

#include <QMainWindow>
#include <QFileDialog>
#include <QString>
#include <QStandardItemModel>
#include <list>
#include "Interpreter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Analyzer; }
QT_END_NAMESPACE

class Analyzer : public QMainWindow
{
    Q_OBJECT

public:
    Analyzer(QWidget *parent = nullptr);
    ~Analyzer();

private slots:
    void AnalyzerMain(int argc, const char** argv);

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void showErrors(std::list<error*> errors);//функция обработки ошибок

    void showTetrads(std::list<Tetrad*> tetrads);//функция обработки тетрад

    void showBasicBlocks(std::list<BasicBlock*> basicBlocks);//функция обработки базовых блоков

    void showEdges(std::map<int, std::list<edge*>>& edges);//функция обработки дуг

    void errorBackground(std::list<int> lineNumber);//подсвет ошибок

    void on_action_triggered();//вызов справки "Руководство пользователя"

    void on_action_2_triggered();//вызов справки "О программе"

private:
    Ui::Analyzer *ui;
    QStandardItemModel *model;
    QStandardItemModel *model1;
    QStandardItemModel *model2;
    QStandardItemModel *model3;
};
#endif // ANALYZER_H

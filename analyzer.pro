QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += no_lflags_merge
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


LIBS += -L"D:\clang\llvm-project\build\Debug\lib"
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangAnalysis
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangAST
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangASTMatchers
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangBasic
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangDriver
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangEdit
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangFormat
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangFrontend
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangLex
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangParse
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangRewrite
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangRewriteFrontend
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangSema
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangSerialization
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangTooling
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangToolingCore
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lclangToolingInclusions
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMAggressiveInstCombine
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMAnalysis
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMBinaryFormat
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMBitReader
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMBitstreamReader
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMCore
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMDebugInfoCodeView
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMDebugInfoDWARF
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMDebugInfoMSF
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMDebugInfoPDB
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMDemangle
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMFrontendOpenMP
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMInstCombine
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMMC
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMMCParser
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMObject
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMOption
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMProfileData
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMRemarks
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMScalarOpts
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMSupport
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMSymbolize
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMTableGen
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMTableGenGlobalISel
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMTextAPI
win32: LIBS += -L$PWD/../clang/llvm-project/build/Debug/lib/ -lLLVMTransformUtils

win32: LIBS += -L'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64/' -lVersion

#LIBS += "C:\Program Files (x86)\National Instruments\LabVIEW 7.1\applibs\lvdllbuilder\sdk\lib\version.lib"
DEPENDPATH += "D:\clang\llvm-project\build\Debug\lib"
INCLUDEPATH += "D:\clang\llvm-project\clang\include"
INCLUDEPATH += D:\clang\llvm-project\llvm\include
INCLUDEPATH += "D:\clang\llvm-project\build\tools\clang\include"
INCLUDEPATH += "D:\clang\llvm-project\build\include"
INCLUDEPATH += "D:\clang\llvm-project\build\include"

SOURCES += \
    CodeGenerator.cpp \
    Interpreter.cpp \
    Tetrad.cpp \
    astComplement.cpp \
    controlFlowGraph.cpp \
    main.cpp \
    analyzer.cpp \
    staticanalyzer.cpp

HEADERS += \
    Interpreter.h \
    Tetrad.h \
    analyzer.h \
    astComplement.h \
    controlFlowGraph.h

FORMS += \
    analyzer.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

#win32: LIBS += -L$$PWD/../clang/llvm-project/build/Debug/lib/ -lclangAST

#INCLUDEPATH += $$PWD/../clang/llvm-project/build/Debug
#DEPENDPATH += $$PWD/../clang/llvm-project/build/Debug



#INCLUDEPATH += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64'
#DEPENDPATH += 'C:/Program Files (x86)/Windows Kits/10/Lib/10.0.18362.0/um/x64'

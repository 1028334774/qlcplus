include(../variables.pri)

# Default rules for deployment.
#include(../deployment.pri)

TEMPLATE = app
TARGET = qlcplus-qml

QT += qml quick widgets svg xml script
QT += multimedia multimediawidgets

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Engine
INCLUDEPATH     += ../engine/src ../engine/src/audio
INCLUDEPATH     += virtualconsole
DEPENDPATH      += ../engine/src
QMAKE_LIBDIR    += ../engine/src
LIBS            += -lqlcplusengine
win32:QMAKE_LFLAGS += -shared

# Plugins
INCLUDEPATH     += ../plugins/interfaces

HEADERS += \
    app.h \
    contextmanager.h \
    fixturebrowser.h \
    fixturemanager.h \
    functionmanager.h \
    inputoutputmanager.h \ 
    treemodel.h \
    treemodelitem.h \
    previewcontext.h \
    mainview2d.h \
    mainviewdmx.h \
    sceneeditor.h
    
HEADERS += \
    virtualconsole/virtualconsole.h \
    virtualconsole/vcwidget.h \
    virtualconsole/vcframe.h \
    virtualconsole/vcsoloframe.h \
    virtualconsole/vcbutton.h \
    virtualconsole/vclabel.h

SOURCES += main.cpp \
    app.cpp \
    contextmanager.cpp \
    fixturebrowser.cpp \
    fixturemanager.cpp \
    functionmanager.cpp \
    inputoutputmanager.cpp \
    treemodel.cpp \
    treemodelitem.cpp \
    previewcontext.cpp \
    mainview2d.cpp \
    mainviewdmx.cpp \
    sceneeditor.cpp
    
SOURCES += \
    virtualconsole/virtualconsole.cpp \
    virtualconsole/vcwidget.cpp \
    virtualconsole/vcframe.cpp \
    virtualconsole/vcsoloframe.cpp \
    virtualconsole/vcbutton.cpp \
    virtualconsole/vclabel.cpp

RESOURCES += qml.qrc ../resources/icons/svg/svgicons.qrc ../resources/fonts/fonts.qrc

macx {
    # This must be after "TARGET = " and before target installation so that
    # install_name_tool can be run before target installation
    include(../macx/nametool.pri)
}

# Installation
target.path = $$INSTALLROOT/$$BINDIR
INSTALLS   += target

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/../android-files

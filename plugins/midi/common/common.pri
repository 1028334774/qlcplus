INCLUDEPATH += ../../interfaces
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../common
INCLUDEPATH += ../../../engine/src
DEPENDPATH  += ../common

CONFIG  += qt
QT      += core xml

HEADERS     += ../../interfaces/qlcioplugin.h
HEADERS     += ../../../engine/src/qlcfile.h
HEADERS += ../common/mididevice.h \
           ../common/midiinputdevice.h \
           ../common/midioutputdevice.h \
           ../common/midiplugin.h \
           ../common/midiprotocol.h \
           ../common/miditemplate.h \
           ../common/midienumerator.h \
           ../common/configuremidiplugin.h

SOURCES += ../common/mididevice.cpp \
           ../common/midiinputdevice.cpp \
           ../common/midioutputdevice.cpp \
           ../common/midiplugin.cpp \
           ../common/midiprotocol.cpp \
           ../common/miditemplate.cpp \
           ../common/configuremidiplugin.cpp

SOURCES += ../../../engine/src/qlcfile.cpp

FORMS   += ../common/configuremidiplugin.ui

# This must be after "TARGET = " and before target installation so that
# install_name_tool can be run before target installation
macx:include(../../../macx/nametool.pri)

target.path = $$INSTALLROOT/$$PLUGINDIR
INSTALLS   += target


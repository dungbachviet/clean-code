include(../../../../3rdparty/nlohmann/nlohmann.pri)
include(../../../../3rdparty/piscada/piscada.pri)
include(../../../lib/PiscadaDB.pri)
include(../../PiscadaModule/piscadamodule_include.pri)

TEMPLATE = app
TARGET = ApiServer

QT += core network concurrent

CONFIG += console

DESTDIR = ../../../plugins/apps

include(views/views.pri)
include(templates/templates.pri)
include(systems/systems.pri)
include(subsystems/subsystems.pri)
include(popups/popups.pri)
include(viewtree/viewtree.pri)
include(documents/documents.pri)
include(systemutils/systemutils.pri)

HEADERS += \
    apierror.h \
    apihandler.h \
    apiserverdatabase.h \
    apiserverjson.h \
    apiserverinfo.h \
    apiservermodule.h

SOURCES += \
    apierror.cpp \
    apihandler.cpp \
    apiserverdatabase.cpp \
    apiserverinfo.cpp \
    apiservermodule.cpp

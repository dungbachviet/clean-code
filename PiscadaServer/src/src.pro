include(common.pri)

TEMPLATE = subdirs
SUBDIRS = PiscadaDB PiscadaModule PiscadaCommunication libs/driverinterface system apps drivers PiscadaLicensing
SUBDIRS += PiscadaServer
SUBDIRS += PiscadaController PiscadaGUI qtservice
SUBDIRS += PiscadaConsole
SUBDIRS += tools

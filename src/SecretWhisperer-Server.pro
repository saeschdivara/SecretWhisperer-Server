#-------------------------------------------------
#
# Project created by QtCreator 2015-08-05T18:17:45
#
#-------------------------------------------------

QT       += core network sql

QT       -= gui

TARGET = SecretWhisperer-Server

CONFIG += c++11
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    chat/chatcontroller.cpp \
    chat/chatdevicecontroller.cpp \
    chat/connector.cpp \
    chat/protocolcontroller.cpp \
    chat/helper/migrationhelper.cpp \
    chat/helper/sql.cpp \
    chat/user/identitycontroller.cpp \
    chat/encryptor.cpp

HEADERS += \
    chat/chatcontroller.h \
    chat/chatdevicecontroller.h \
    chat/connector.h \
    chat/protocolcontroller.h \
    chat/helper/migrationhelper.h \
    chat/helper/sql.h \
    chat/user/identitycontroller.h \
    chat/encryptor.h

RESOURCES += \
    data.qrc

# Lib Botan: https://github.com/randombit/botan
unix|win32: LIBS += -L/usr/local/lib/ -lbotan-1.11

INCLUDEPATH += /usr/local/include/botan-1.11/
DEPENDPATH += /usr/local/include/botan-1.11/

#-------------------------------------------------
#
# Project created by QtCreator 2015-08-05T18:17:45
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = SecretWhisperer-Server
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += main.cpp \
    chat/chatcontroller.cpp

HEADERS += \
    chat/chatcontroller.h

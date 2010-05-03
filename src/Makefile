#!/bin/make

OBJECTS=main.o qrc.o
OBJECTS+=MainWindow.o MainWindow_TabActions.o MainWindow_TreeViewActions.o LoginWindow.o
OBJECTS+=ServerInterface.o QtWddx.o TplSyntax.o TabEditor.o MainTreeView.o
OBJECTS+=TplModelRoot.o TplModelNode.o TplModelFilter.o TplModelDelegate.o TplMimeData.o
OBJECTS+=PropertiesPage.o PropertiesTpl.o PreferencesDialog.o
DIALOGS=MainWindow SearchWindow LoginWindow PropertiesPage PropertiesTpl PreferencesDialog
MOCOBJ=MainWindow LoginWindow PropertiesPage PropertiesTpl ServerInterface TplModelRoot
MOCOBJ+=TplSyntax TplModelNode TplModelDelegate TabEditor TplMimeData PreferencesDialog
QRC_FILES=$(ROOT)/res/tplv3.qrc

QT_MODS=QtXml QtNetwork QtGui QtCore
QT_PLUGINS=imageformats/libqgif imageformats/libqjpeg imageformats/libqico
#QT_SOLUTIONS=QtSoap QtSingleApplication

TARGET=$(TPLV3_BIN)

include $(CONFIG)/rules.mak


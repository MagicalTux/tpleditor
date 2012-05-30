######################################################################
# Automatically generated by qmake (2.01a) lun. mai 3 18:55:10 2010
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += includes res src
INCLUDEPATH += includes src src/qscintilla src/qscintilla/lexer src/qscintilla/lexlib
PROFILE = tibanne

UI_HEADERS_DIR = includes
UI_SOURCES_DIR = src
OBJECTS_DIR = tmp
MOC_DIR = tmp
RCC_DIR = res

QT += network xml script

!win32:VERSION = 8.0.1
mac:ICON = res/tibanne.icns

DEFINES += TPLV3_VERSION_MAJOR=1 TPLV3_VERSION_MINOR=2 TPLV3_VERSION_TYPE=1 TPLV3_VERSION_REVISION=2 QT SCI_LEXER

win32:RC_FILE += src/tpleditor.rc

QMAKE_CFLAGS_DEBUG += -include common.h -include profiles/$${PROFILE}.h
QMAKE_CFLAGS_RELEASE += -include common.h -include profiles/$${PROFILE}.h
QMAKE_CXXFLAGS += -include common.h -include profiles/$${PROFILE}.h -Wno-unused-parameter

# Input
HEADERS += includes/common.h \
           src/LoginWindow.hpp \
           src/MainTreeView.hpp \
           src/MainWindow.hpp \
           src/PreferencesDialog.hpp \
           src/PropertiesPage.hpp \
           src/PropertiesTpl.hpp \
           src/QtJson.hpp \
           src/ServerInterface.hpp \
           src/TabEditor.hpp \
           src/TplMimeData.hpp \
           src/TplModelDelegate.hpp \
           src/TplModelFilter.hpp \
           src/TplModelNode.hpp \
           src/TplModelRoot.hpp \
           includes/profiles/$${PROFILE}.h \
           src/qscintilla/Qsci/qsciglobal.h \
           src/qscintilla/Qsci/qsciscintilla.h \
           src/qscintilla/Qsci/qsciscintillabase.h \
           src/qscintilla/Qsci/qsciabstractapis.h \
           src/qscintilla/Qsci/qsciapis.h \
           src/qscintilla/Qsci/qscicommand.h \
           src/qscintilla/Qsci/qscicommandset.h \
           src/qscintilla/Qsci/qscidocument.h \
           src/qscintilla/Qsci/qscilexer.h \
           src/qscintilla/Qsci/qscilexercpp.h \
           src/qscintilla/Qsci/qscilexercss.h \
           src/qscintilla/Qsci/qscilexercustom.h \
           src/qscintilla/Qsci/qscilexerhtml.h \
           src/qscintilla/Qsci/qscilexerjavascript.h \
           src/qscintilla/Qsci/qscilexerproperties.h \
           src/qscintilla/Qsci/qscilexerxml.h \
           src/qscintilla/Qsci/qscilexeryaml.h \
           src/qscintilla/Qsci/qscimacro.h \
           src/qscintilla/Qsci/qsciprinter.h \
           src/qscintilla/Qsci/qscistyle.h \
           src/qscintilla/Qsci/qscistyledtext.h \
           src/qscintilla/ListBoxQt.h \
           src/qscintilla/SciClasses.h \
           src/qscintilla/SciNamespace.h \
           src/qscintilla/ScintillaQt.h \
           src/qscintilla/ILexer.h \
           src/qscintilla/Platform.h \
           src/qscintilla/SciLexer.h \
           src/qscintilla/Scintilla.h \
           src/qscintilla/ScintillaWidget.h \
           src/qscintilla/lexlib/Accessor.h \
           src/qscintilla/lexlib/CharacterSet.h \
           src/qscintilla/lexlib/LexAccessor.h \
           src/qscintilla/lexlib/LexerBase.h \
           src/qscintilla/lexlib/LexerModule.h \
           src/qscintilla/lexlib/LexerNoExceptions.h \
           src/qscintilla/lexlib/LexerSimple.h \
           src/qscintilla/lexlib/OptionSet.h \
           src/qscintilla/lexlib/PropSetSimple.h \
           src/qscintilla/lexlib/StyleContext.h \
           src/qscintilla/lexlib/WordList.h \
           src/qscintilla/AutoComplete.h \
           src/qscintilla/CallTip.h \
           src/qscintilla/Catalogue.h \
           src/qscintilla/CellBuffer.h \
           src/qscintilla/CharClassify.h \
           src/qscintilla/ContractionState.h \
           src/qscintilla/Decoration.h \
           src/qscintilla/Document.h \
           src/qscintilla/Editor.h \
           src/qscintilla/ExternalLexer.h \
           src/qscintilla/FontQuality.h \
           src/qscintilla/Indicator.h \
           src/qscintilla/KeyMap.h \
           src/qscintilla/LineMarker.h \
           src/qscintilla/Partitioning.h \
           src/qscintilla/PerLine.h \
           src/qscintilla/PositionCache.h \
           src/qscintilla/RESearch.h \
           src/qscintilla/RunStyles.h \
           src/qscintilla/ScintillaBase.h \
           src/qscintilla/Selection.h \
           src/qscintilla/SplitVector.h \
           src/qscintilla/Style.h \
           src/qscintilla/SVector.h \
           src/qscintilla/UniConversion.h \
           src/qscintilla/ViewStyle.h \
           src/qscintilla/XPM.h \
    src/qscintilla/Qsci/qscilexerdiff.h \
    src/qscintilla/Qsci/qscilexerpython.h

FORMS += src/LoginWindow.ui \
         src/MainWindow.ui \
         src/PreferencesDialog.ui \
         src/PropertiesPage.ui \
         src/PropertiesTpl.ui \
         src/SearchWindow.ui
SOURCES += src/LoginWindow.cpp \
           src/main.cpp \
           src/MainTreeView.cpp \
           src/MainWindow.cpp \
           src/MainWindow_TabActions.cpp \
           src/MainWindow_TreeViewActions.cpp \
           src/PreferencesDialog.cpp \
           src/PropertiesPage.cpp \
           src/PropertiesTpl.cpp \
           src/QtJson.cpp \
           src/ServerInterface.cpp \
           src/TabEditor.cpp \
           src/TplMimeData.cpp \
           src/TplModelDelegate.cpp \
           src/TplModelFilter.cpp \
           src/TplModelNode.cpp \
           src/TplModelRoot.cpp \
           src/qscintilla/qsciscintilla.cpp \
           src/qscintilla/qsciscintillabase.cpp \
           src/qscintilla/qsciabstractapis.cpp \
           src/qscintilla/qsciapis.cpp \
           src/qscintilla/qscicommand.cpp \
           src/qscintilla/qscicommandset.cpp \
           src/qscintilla/qscidocument.cpp \
           src/qscintilla/qscilexer.cpp \
           src/qscintilla/qscilexercpp.cpp \
           src/qscintilla/qscilexercss.cpp \
           src/qscintilla/qscilexercustom.cpp \
           src/qscintilla/qscilexerdiff.cpp \
           src/qscintilla/qscilexerhtml.cpp \
           src/qscintilla/qscilexerjavascript.cpp \
           src/qscintilla/qscilexerproperties.cpp \
           src/qscintilla/qscilexerxml.cpp \
           src/qscintilla/qscilexeryaml.cpp \
           src/qscintilla/qscimacro.cpp \
           src/qscintilla/qsciprinter.cpp \
           src/qscintilla/qscistyle.cpp \
           src/qscintilla/qscistyledtext.cpp \
           src/qscintilla/SciClasses.cpp \
           src/qscintilla/ListBoxQt.cpp \
           src/qscintilla/PlatQt.cpp \
           src/qscintilla/ScintillaQt.cpp \
           src/qscintilla/lexers/LexCPP.cpp \
           src/qscintilla/lexers/LexCSS.cpp \
           src/qscintilla/lexers/LexHTML.cpp \
           src/qscintilla/lexlib/Accessor.cpp \
           src/qscintilla/lexlib/CharacterSet.cpp \
           src/qscintilla/lexlib/LexerBase.cpp \
           src/qscintilla/lexlib/LexerModule.cpp \
           src/qscintilla/lexlib/LexerNoExceptions.cpp \
           src/qscintilla/lexlib/LexerSimple.cpp \
           src/qscintilla/lexlib/PropSetSimple.cpp \
           src/qscintilla/lexlib/StyleContext.cpp \
           src/qscintilla/lexlib/WordList.cpp \
           src/qscintilla/AutoComplete.cpp \
           src/qscintilla/CallTip.cpp \
           src/qscintilla/Catalogue.cpp \
           src/qscintilla/CellBuffer.cpp \
           src/qscintilla/CharClassify.cpp \
           src/qscintilla/ContractionState.cpp \
           src/qscintilla/Decoration.cpp \
           src/qscintilla/Document.cpp \
           src/qscintilla/Editor.cpp \
           src/qscintilla/ExternalLexer.cpp \
           src/qscintilla/Indicator.cpp \
           src/qscintilla/KeyMap.cpp \
           src/qscintilla/LineMarker.cpp \
           src/qscintilla/PerLine.cpp \
           src/qscintilla/PositionCache.cpp \
           src/qscintilla/RESearch.cpp \
           src/qscintilla/RunStyles.cpp \
           src/qscintilla/ScintillaBase.cpp \
           src/qscintilla/Selection.cpp \
           src/qscintilla/Style.cpp \
           src/qscintilla/UniConversion.cpp \
           src/qscintilla/ViewStyle.cpp \
           src/qscintilla/XPM.cpp \
    src/qscintilla/qscilexerpython.cpp


RESOURCES += res/tpleditor.qrc
TRANSLATIONS += res/locale/locale_fr.ts res/locale/locale_ja_jp.ts

OTHER_FILES += \
    src/tpleditor.rc

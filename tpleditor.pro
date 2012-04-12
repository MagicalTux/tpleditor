######################################################################
# Automatically generated by qmake (2.01a) lun. mai 3 18:55:10 2010
######################################################################

TEMPLATE = app
TARGET = 
DEPENDPATH += includes res src
INCLUDEPATH += includes src
PROFILE = degica

UI_HEADERS_DIR = includes
UI_SOURCES_DIR = src
OBJECTS_DIR = tmp
MOC_DIR = tmp
RCC_DIR = res

QT += network xml script

DEFINES += TPLV3_VERSION_MAJOR=1 TPLV3_VERSION_MINOR=0 TPLV3_VERSION_TYPE=1 TPLV3_VERSION_REVISION=3

win32:RC_FILE += src/tpleditor.rc

QMAKE_CFLAGS_DEBUG += -include common.h -include profiles/$${PROFILE}.h
QMAKE_CFLAGS_RELEASE += -include common.h -include profiles/$${PROFILE}.h
QMAKE_CXXFLAGS += -include common.h -include profiles/$${PROFILE}.h

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
           src/TplSyntax.hpp \
           includes/profiles/$${PROFILE}.h
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
           src/TplSyntax.cpp
RESOURCES += res/tpleditor.qrc
TRANSLATIONS += res/locale/locale_fr.ts res/locale/locale_ja_jp.ts

OTHER_FILES += \
    src/tpleditor.rc

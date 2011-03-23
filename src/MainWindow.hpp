/**
 * Copyright (c) 2010, Mark Karpeles
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Kabushiki Kaisha Tibanne nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *     * Sourcecode under ths license cannot be released under any license
 *       excessively restricting freedom to choose another license such as the
 *       GNU General Public License. This clause should be kept if another
 *       license is chosen.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __DLMGR_MAINWINDOW_HPP
#define __DLMGR_MAINWINDOW_HPP

#include <QObject>
#include <QMainWindow>
#include <QTranslator>
#include <QSettings>

#include "ui_MainWindow.h"

class LoginWindow;
class ServerInterface;
class TplModelRoot;
class TplModelNode;
class TplModelFilter;
class TplModelDelegate;
class QSortFilterProxyModel;
class QModelIndex;
class QTextEdit;
class TplSyntax;
class TabEditor;

class MainWindow: public QMainWindow {
	Q_OBJECT;

public:
	MainWindow(QWidget *parent = 0);
	virtual ~MainWindow();
	Ui::MainWindow ui;

	void initLogin(bool autoLogin = true);
	void initInterface(const QString &welcome);

	// too lazy to create readonly settings accessors, we'll do like this for now
	QSettings settings; // application settings

private:
	void setAppLocale(QLocale _locale, bool retranslate = true);

	LoginWindow *win_login;
	ServerInterface *srv;

	TplModelRoot *model_root;
//	TplModelFilter *model_proxy;
	TplModelDelegate *model_delegate;
	QMap<QString, TabEditor *> tabs;

private slots:
	void closeEvent(QCloseEvent *event);
	void writeSettings();
	void closeTab(int);

	void subTabStatusChanged();

	void on_actionUpdate_syntax_coloration_triggered();
	void on_action_Save_triggered();
	void on_mainTreeView_doubleClicked(const QModelIndex &index);
	void on_mainTreeView_customContextMenuRequested(const QPoint &pos);
	void on_action_Close_triggered();
	void on_action_Logout_triggered();

	void on_action_NewPage_triggered();
	void on_action_RenamePage_triggered();
	void on_action_DeletePage_triggered();
	void on_action_NewTemplate_triggered();
	void on_action_RenameTemplate_triggered();
	void on_action_DeleteTemplate_triggered();
	void on_action_NewLanguage_triggered();
	void on_action_ChangeLanguage_triggered();
	void on_action_DeleteLanguage_triggered();
	void on_action_RenameFile_triggered();
	void on_action_DeleteFile_triggered();
	void on_action_CreateDirectory_triggered();
	void on_action_RenameDirectory_triggered();
	void on_action_DeleteDirectory_triggered();
	void on_action_PageProperties_triggered();
	void on_action_TplProperties_triggered();
	void on_action_SaveServer_triggered();
	void on_action_RestoreServer_triggered();
	void on_action_AboutQt_triggered();
	void on_action_AboutTemplatesEditor_triggered();
	void on_action_Preferences_triggered();
	void on_action_CompilePage_triggered();
	void on_action_NextTab_triggered();
	void on_action_PreviousTab_triggered();

	void on_action_English_triggered();
	void on_action_French_triggered();
	void on_action_Japanese_triggered();

	void on_mainTreeView_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

	void event_reloadSettings();
signals:
	void reloadSettings();
};

#endif /* __DLMGR_MAINWINDOW_HPP */

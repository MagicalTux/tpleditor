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

#include <QMessageBox>
#include <QCloseEvent>
#include <QModelIndex>
#include <QToolButton>
#include <QTextEdit>

#include "MainWindow.hpp"
#include "LoginWindow.hpp"
#include "ServerInterface.hpp"
#include "TplModelRoot.hpp"
#include "TplSyntax.hpp"
#include "TplModelDelegate.hpp"
#include "TplModelFilter.hpp"
#include "TabEditor.hpp"
#include "PreferencesDialog.hpp"

MainWindow::MainWindow(QWidget *parent) {
	// init locale, install translator
	setAppLocale(QLocale::system());

	// init some stuff
	srv = new ServerInterface(this);
	win_login = new LoginWindow(this, *srv);
	model_root = new TplModelRoot(this, *srv);
	model_proxy = new TplModelFilter(this);
	model_proxy->setSourceModel(model_root);
	model_delegate = new TplModelDelegate(this);

	// Setup UI
	ui.setupUi(this);

	// Configure treeview
	ui.mainTreeView->setModel(model_proxy);
	ui.mainTreeView->setItemDelegate(model_delegate);
	connect(ui.mainTreeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(on_mainTreeView_selectionChanged(QItemSelection,QItemSelection)));

	// Put the profile logo here
	setWindowIcon(QIcon(":/images/" WINDOW_ICON));
	ui.img->setText("");
	ui.img->setPixmap(QPixmap(":/images/" WINDOW_LOGO));

	connect(ui.tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));

	// Restore UI settings
	settings.beginGroup("MainWindow");
	resize(settings.value("size", QSize(800, 600)).toSize());
	move(settings.value("pos", QPoint(200, 200)).toPoint());
	ui.splitter->restoreState(settings.value("splitter").toByteArray());
	settings.endGroup();
}

void MainWindow::on_action_Logout_triggered() {
	if (QMessageBox::question(this, tr("Really logout?"), tr("Do you really want to logout from this server? All unsaved data will be lost."), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
		writeSettings();

		// close tabs
		for(int i = ui.tabWidget->count() - 1; i >= 0; i--) {
			QString text = ui.tabWidget->tabText(i);
			TabEditor *editor = qobject_cast<TabEditor*>(ui.tabWidget->widget(i));

			if (editor == NULL) continue;

			ui.tabWidget->removeTab(i);
			tabs.remove(editor->getKey());
			delete editor;
		}
		//QMap<QString, TabEditor *>

		delete model_delegate;
		delete model_proxy;
		delete model_root;
		delete srv;

		srv = new ServerInterface(this);
		win_login = new LoginWindow(this, *srv);
		model_root = new TplModelRoot(this, *srv);
		model_proxy = new TplModelFilter(this);
		model_proxy->setSourceModel(model_root);
		model_delegate = new TplModelDelegate(this);

		ui.mainTreeView->setModel(model_proxy);
		ui.mainTreeView->setItemDelegate(model_delegate);

		initLogin(false);
	}
}

MainWindow::~MainWindow() {
	delete model_root;
	delete win_login;
	//delete syncolor;
	delete srv;
}

void MainWindow::setAppLocale(QLocale _locale) {
	QString locale = _locale.name().toLower();
	static QTranslator qt_translator, translator; // string translator

	qDebug("Locale: %s", qPrintable(locale));

	QLocale::setDefault(_locale);

	if (qt_translator.load(QString("qt_") + locale, ":/locale/")) {
		QCoreApplication::installTranslator(&qt_translator);
	}

	if (translator.load(QString("locale_") + locale, ":/locale/")) {
		qDebug("load success");
		QCoreApplication::installTranslator(&translator);
	}

}

void MainWindow::writeSettings() {
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("splitter", ui.splitter->saveState());
	settings.endGroup();
}

void MainWindow::closeEvent(QCloseEvent *event) {
	// Check if really want
	// TODO: check if any unsaved content is present in tabs, and close all tabs
	if (QMessageBox::question(this, tr("Really quit?"), tr("Do you really want to quit the templates editor?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
		writeSettings();
		event->accept();
	} else {
		event->ignore();
	}
}

void MainWindow::initLogin(bool autoLogin) {
	win_login->show();
	win_login->initLogin(autoLogin);
}

void MainWindow::initInterface(const QString &welcome) {
	ui.welcome_label->setText(welcome);
	srv->sendRequest("Session.getLogin");

	model_root->refresh();
}

void MainWindow::on_action_AboutQt_triggered() {
	QMessageBox::aboutQt(this);
}
void MainWindow::on_action_AboutTemplatesEditor_triggered() {
	QMessageBox::about(this, tr("About Templates Editor"), tr("<b>Templates Editor version %1</b> (opensource)<br/>By <a href=\"mailto:magicaltux@gmail.com\">MagicalTux (mail)</a><br /><br />More informations at <a href=\"http://en.wiki.gg.st/wiki/TplEditor\">http://en.wiki.gg.st/wiki/TplEditor</a><br /><br />Code is released under BSD-no-GPL standard license.").arg(TPLV3_VERSION));
}

void MainWindow::on_action_Preferences_triggered() {
	PreferencesDialog *prefs = new PreferencesDialog(settings);
	connect(prefs, SIGNAL(reloadSettings()), this, SLOT(event_reloadSettings()));
}

void MainWindow::event_reloadSettings() {
	reloadSettings();
}

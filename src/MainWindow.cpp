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
#include <QDebug>

#include "MainWindow.hpp"
#include "LoginWindow.hpp"
#include "ServerInterface.hpp"
#include "TplModelRoot.hpp"
#include "TplModelDelegate.hpp"
#include "TplModelFilter.hpp"
#include "TabEditor.hpp"
#include "PreferencesDialog.hpp"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent) {
	// Setup UI
	ui.setupUi(this);

	// init some stuff
	srv = new ServerInterface(this);
	win_login = new LoginWindow(this, *srv);
	model_root = new TplModelRoot(this, *srv);
//	model_proxy = new TplModelFilter(this);
//	model_proxy->setSourceModel(model_root);
	model_delegate = new TplModelDelegate(this);

	// Wordwrap configuration
	wordwrap_group = new QActionGroup(this);
	wordwrap_group->addAction(ui.action_WrapNone);
	wordwrap_group->addAction(ui.action_WrapCharacter);
	wordwrap_group->addAction(ui.action_WrapWord);
	wordwrap_group->setExclusive(true);

	// Configure treeview
	ui.mainTreeView->setModel(model_root);
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
	wrap_mode = (QsciScintilla::WrapMode)settings.value("wordwrap", (int)QsciScintilla::WrapWord).toInt();

	switch (wrap_mode) {
	case QsciScintilla::WrapNone:
		ui.action_WrapNone->setChecked(true);
		break;
	case QsciScintilla::WrapCharacter:
		ui.action_WrapCharacter->setChecked(true);
		break;
	case QsciScintilla::WrapWord:
		ui.action_WrapWord->setChecked(true);
		break;
	}

	// init locale, install translator, check for saved locale
	setAppLocale(settings.value("locale", QLocale::system()).toLocale(), false);

	settings.endGroup();
}

void MainWindow::updateWordWrap() {
	foreach(TabEditor *tab, tabs) {
		tab->setWrapMode(wrap_mode);
	}
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

		delete model_delegate;
		delete srv;

		srv = new ServerInterface(this);
		win_login = new LoginWindow(this, *srv);
		model_root = new TplModelRoot(this, *srv);
		model_delegate = new TplModelDelegate(this);

		ui.mainTreeView->setModel(model_root);
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

void MainWindow::on_action_English_triggered() {
	setAppLocale(QLocale("en_us"));
}

void MainWindow::on_action_French_triggered() {
	setAppLocale(QLocale("fr_fr"));
}

void MainWindow::on_action_Japanese_triggered() {
	setAppLocale(QLocale("ja_jp"));
}

void MainWindow::setAppLocale(QLocale _locale, bool retranslate) {
	if (retranslate) {
		settings.beginGroup("MainWindow");
		settings.setValue("locale", _locale);
		settings.endGroup();
	}
	QString locale = _locale.name().toLower();
	static QTranslator qt_translator, translator; // string translator

	QCoreApplication::removeTranslator(&qt_translator);
	QCoreApplication::removeTranslator(&translator);

	qDebug("Locale: %s", qPrintable(locale));

	QLocale::setDefault(_locale);

	if (qt_translator.load(QString("qt_") + locale, ":/locale/")) {
		QCoreApplication::installTranslator(&qt_translator);
	}

	if (translator.load(QString("locale_") + locale, ":/locale/")) {
		qDebug("load success");
		QCoreApplication::installTranslator(&translator);
	}

	QString lang = tr("en_us");
	qDebug("Locale tr: %s", qPrintable(lang));
	ui.action_English->setChecked(false);
	ui.action_French->setChecked(false);
	ui.action_Japanese->setChecked(false);
	if (lang == "fr") {
		ui.action_French->setChecked(true);
	} else if (lang == "ja_jp") {
		ui.action_Japanese->setChecked(true);
	} else {
		ui.action_English->setChecked(true);
	}

	ui.retranslateUi(this);
	win_login->retranslateUi();
}

void MainWindow::on_action_NextTab_triggered() {
	int pos = ui.tabWidget->currentIndex();
	pos++;
	if (pos >= ui.tabWidget->count()) pos = 0;
	ui.tabWidget->setCurrentIndex(pos);
}

void MainWindow::on_action_PreviousTab_triggered() {
	int pos = ui.tabWidget->currentIndex();
	pos--;
	if (pos < 0) pos = ui.tabWidget->count()-1;
	ui.tabWidget->setCurrentIndex(pos);
}

void MainWindow::writeSettings() {
	settings.beginGroup("MainWindow");
	settings.setValue("size", size());
	settings.setValue("pos", pos());
	settings.setValue("splitter", ui.splitter->saveState());
	settings.setValue("wordwrap", wrap_mode);
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
    setWindowTitle(win_login->ui.txt_server->text() + " - " + windowTitle());
	if (win_login->ui.proxy_config->isChecked()) {
        setWindowTitle(windowTitle() + " (" + tr("Proxy enabled") + ")");
	}

	ui.welcome_label->setText(welcome);
	srv->sendRequest("Session.getLogin");

	model_root->refresh();
}

void MainWindow::on_action_AboutQt_triggered() {
	QMessageBox::aboutQt(this);
}
void MainWindow::on_action_AboutTemplatesEditor_triggered() {
	QMessageBox::about(this, tr("About Templates Editor"), tr("<b>Templates Editor version %1</b> (opensource)<br/>By <a href=\"mailto:magicaltux@gmail.com\">MagicalTux (mail)</a><br /><br />More informations at <a href=\"http://en.wiki.gg.st/wiki/TplEditor\">http://en.wiki.gg.st/wiki/TplEditor</a><br /><br />Code is released under GPLv2.").arg(TPLV3_VERSION));
}

void MainWindow::on_action_Preferences_triggered() {
	PreferencesDialog *prefs = new PreferencesDialog(settings);
	connect(prefs, SIGNAL(reloadSettings()), this, SLOT(event_reloadSettings()));
}

void MainWindow::event_reloadSettings() {
	reloadSettings();
}

void MainWindow::on_action_WrapNone_triggered() {
	wrap_mode = QsciScintilla::WrapNone;
	updateWordWrap();
}

void MainWindow::on_action_WrapCharacter_triggered() {
	wrap_mode = QsciScintilla::WrapCharacter;
	updateWordWrap();
}
void MainWindow::on_action_WrapWord_triggered() {
	wrap_mode = QsciScintilla::WrapWord;
	updateWordWrap();
}

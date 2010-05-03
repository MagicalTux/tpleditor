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

#include "LoginWindow.hpp"
#include "MainWindow.hpp"
#include "ServerInterface.hpp"

#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent, ServerInterface &_srv): mw(dynamic_cast<MainWindow&>(*parent)), srv(_srv) {
	// Setup UI
	ui.setupUi(this);
}

void LoginWindow::closeEvent(QCloseEvent *event) {
	if (!srv.hasSession()) QApplication::quit();
}


void LoginWindow::initLogin(bool autoLogin) {
	// load login/password from settings
	mw.settings.beginGroup("Authentication");
	ui.txt_login->setText(mw.settings.value("Login", QString()).toString());
	ui.txt_password->setText(mw.settings.value("Password", QString()).toString());
	ui.txt_server->setText(mw.settings.value("Server", QString()).toString());
	ui.check_keeppw->setCheckState((mw.settings.value("Keep", "No") == "Yes") ? Qt::Checked : Qt::Unchecked);
	mw.settings.endGroup();
	// test: if we have all the info, try to login with that
	if (autoLogin && (ui.check_keeppw->checkState() == Qt::Checked)) on_loginButton_clicked();
}

void LoginWindow::getSessionId(int id, QVariant result, QObject*) {
	if (!(result.toMap()["Success"].toBool())) {
		QMessageBox::critical(this, "Login error", result.toMap()["Message"].toString());
		setDisabled(false);
		return;
	}

	setDisabled(false);
	srv.setSession(result.toMap()["SessionID"].toString());
	mw.initInterface(result.toMap()["Welcome"].toString());
	this->close();
}

void LoginWindow::setDisabled(bool disa) {
	ui.txt_server->setDisabled(disa);
	ui.txt_login->setDisabled(disa);
	ui.txt_password->setDisabled(disa);
	ui.check_keeppw->setDisabled(disa);
	ui.quitButton->setDisabled(disa);
	ui.loginButton->setDisabled(disa);
}

void LoginWindow::on_loginButton_clicked() {
	// Save the infos
	mw.settings.beginGroup("Authentication");
	mw.settings.setValue("Server", ui.txt_server->text());
	mw.settings.setValue("Login", ui.txt_login->text());
	if (ui.check_keeppw->checkState() == Qt::Checked) {
		mw.settings.setValue("Password", ui.txt_password->text());
		mw.settings.setValue("Keep", "Yes");
	} else {
		mw.settings.setValue("Password", "");
		mw.settings.setValue("Keep", "No");
	}
	mw.settings.endGroup();

	if (ui.txt_server->text().isEmpty()) return; // :(

	setDisabled(true);

	QMap<QString, QVariant> loginreq;
	loginreq["login"] = ui.txt_login->text();
	loginreq["password"] = ui.txt_password->text();
	loginreq["language"] = QLocale::system().name();
	srv.setGateway(ui.txt_server->text() + QString("/wddx"));
	srv.sendRequest("Session.login", loginreq, this, "getSessionId", NULL);
}

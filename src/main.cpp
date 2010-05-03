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

#include <QApplication>
#include <QtPlugin>
#include <QFontDatabase>

#include "MainWindow.hpp"

#include <iostream>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);

#ifndef QT_SHARED
	Q_IMPORT_PLUGIN(qjpeg);
	Q_IMPORT_PLUGIN(qgif);
	Q_IMPORT_PLUGIN(qico);
#endif

	QCoreApplication::setOrganizationName(APPLICATION_COMPANY);
	QCoreApplication::setOrganizationDomain(APPLICATION_COMPANY_DOMAIN);
	QCoreApplication::setApplicationName(APPLICATION_NAME);
	QCoreApplication::setApplicationVersion(TPLV3_VERSION);

	/*
	int id = QFontDatabase::addApplicationFont(":/binary/ProFontWindows.ttf");
	if (id != -1) {
		QStringList lst = QFontDatabase::applicationFontFamilies(id);
		qDebug("added font: %s", qPrintable(lst.join(",")));

		QFont test(lst.at(0), 12, 1);
		qDebug("exact match: %s", test.exactMatch()?"true":"false");
		qDebug("font family: %s", qPrintable(test.family()));
	} */

	qRegisterMetaType<QVariant>("QVariant");

	MainWindow mw;
	mw.show();
	mw.initLogin();

	return app.exec();
}


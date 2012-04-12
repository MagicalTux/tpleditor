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

#ifndef SERVERINTERFACE_HPP
#define SERVERINTERFACE_HPP

#include <QObject>

#include <QUrl>
#include <QNetworkAccessManager>
#include <QBuffer>
#include <QPointer>
#include <QVariant>
#include <QTimer>
#include <QDialog>
#include <QFile>
#include <QProgressDialog>

class ServerInterface: public QObject {
	Q_OBJECT;

public:
	ServerInterface(QObject *parent = 0);

	QNetworkReply* sendRequest(const QString func, const QVariant &req = QVariant(), QObject *obj = NULL, const char *member = NULL, QObject *extra = NULL);
	QNetworkReply* sendRequestDownload(const QString func, const QVariant &req, QString save_as);

	void setSession(QString);
	bool hasSession();

	void setUpProgressReceiver(QNetworkReply*reply, QObject *obj);

	void setGateway(QUrl url);

protected slots:
	void on_requestFinished(QNetworkReply*);
	void on_dataReadProgress(qint64 done, qint64 total);
	void on_dataSendProgress(qint64 done, qint64 total);
	void timer_keepAlive();
	void on_sslErrors(QNetworkReply*, const QList<QSslError> & errors);
	void copyToFile();

private:
	QUrl uri;
	QNetworkAccessManager http;
	QString session;
	QTimer timer;
	QFile *file;

	struct request {
		QBuffer *buffer;
		// slot infos
		QPointer<QObject> obj;
		QPointer<QObject> up_progress;
		QPointer<QProgressDialog> down_progress;
		QPointer<QFile> file;
		QObject *extra;
		const char *member;
	};
};

#endif // SERVERINTERFACE_HPP

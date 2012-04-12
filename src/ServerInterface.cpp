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

#include "ServerInterface.hpp"
#include "QtJson.hpp"

#include <QMessageBox>
#include <QSslSocket>
#include <QNetworkReply>

ServerInterface::ServerInterface(QObject *) {
	// define some stuff thanks to uri
	connect(&http, SIGNAL(finished(QNetworkReply*)), this, SLOT(on_requestFinished(QNetworkReply*)));
//	connect(&http, SIGNAL(requestFinished(int,bool)), this, SLOT(on_requestFinished(int,bool)));
//	connect(&http, SIGNAL(requestStarted(int)), this, SLOT(on_requestStarted(int)));
//	connect(&http, SIGNAL(dataSendProgress(int,int)), this, SLOT(on_dataSendProgress(int,int)));
//	connect(&http, SIGNAL(dataReadProgress(int,int)), this, SLOT(on_dataReadProgress(int,int)));
	connect(&http, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(on_sslErrors(QNetworkReply*, const QList<QSslError>&)));
//	http.setHost(uri.host(), uri.port(80));

	timer.setInterval(30000);
	timer.setSingleShot(false);
	timer.start();
	connect(&timer, SIGNAL(timeout()), this, SLOT(timer_keepAlive()));
}

void ServerInterface::on_sslErrors(QNetworkReply*reply, const QList<QSslError> &) {
	reply->ignoreSslErrors();
}

void ServerInterface::timer_keepAlive() {
	if (session.isEmpty()) return;
	sendRequest("Session.keepAlive", QVariant(), NULL, NULL, NULL);
}

void ServerInterface::setGateway(QUrl url) {
	qDebug("url=%s scheme=%s", qPrintable(url.toString()), qPrintable(url.scheme()));
	if (url.scheme() == "http") {
		uri = url;
//		http.setHost(uri.host(), QHttp::ConnectionModeHttp, uri.port(80));
	} else if (url.scheme() == "https") {
		uri = url;
                if (!QSslSocket::supportsSsl()) {
                    QMessageBox::warning(NULL, tr("Configuration Error"), tr("Impossible to complete the requested operation, SSL support not available."), QMessageBox::Cancel);
                    // configure anyway, we know it'll fail
                }
//		http.setHost(uri.host(), QHttp::ConnectionModeHttps, uri.port(443));
	}

	return;
}

QNetworkReply *ServerInterface::sendRequestDownload(const QString func, const QVariant &req, QString save_as) {
	// open file for output
	QFile *file = new QFile(save_as);
	if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QMessageBox::critical(NULL, tr("File save error"), tr("Could not open file %1 for writing").arg(save_as), QMessageBox::Cancel);
		return NULL;
	}

	QNetworkRequest request(uri);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("User-Agent", "TPL/1.0 (" TPLV3_VERSION ")");
	request.setRawHeader("X-Tpl-Editor", TPLV3_VERSION);
	request.setRawHeader("X-Ipc", "TPL_JSON");
	if (!session.isEmpty()) request.setRawHeader("Tpl-Session", session.toUtf8());

	QMap<QString, QVariant> real_req;

	real_req["action"] = func;
	real_req["parameters"] = req;
	real_req["editor"] = TPLV3_VERSION;

	QProgressDialog *down_progress = new QProgressDialog(tr("Downloading..."), tr("Cancel"), 0, 0);
	down_progress->setLabelText(tr("Downloading to file %1").arg(save_as));
	down_progress->setMinimumDuration(100);
	down_progress->setValue(-1);

	struct request *rq = new struct request;
	rq->down_progress = down_progress;
	rq->file = file;

	QNetworkReply *reply = http.post(request, QtJson::encode(real_req));
	connect(reply, SIGNAL(readyRead()), this, SLOT(copyToFile()));
	connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(on_dataSendProgress(qint64,qint64)));
	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(on_dataReadProgress(qint64,qint64)));

	reply->setProperty("rq_object", (quint64)rq);

	return reply;
}

void ServerInterface::on_dataReadProgress(qint64 done, qint64 total) {
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
	if (reply == NULL) return;

	struct request *rq = (struct request*)(reply->property("rq_object").toULongLong());

	QProgressDialog *down_progress = rq->down_progress;
	if (down_progress == NULL)
		return;

	down_progress->setMaximum(total);
	down_progress->setValue(done);
}

QNetworkReply *ServerInterface::sendRequest(const QString func, const QVariant &req, QObject *obj, const char *member, QObject *extra) {
	QNetworkRequest request(uri);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
	request.setRawHeader("User-Agent", "TPL/1.0 (" TPLV3_VERSION ")");
	request.setRawHeader("X-Tpl-Editor", TPLV3_VERSION);
	request.setRawHeader("X-Ipc", "TPL_JSON");
	if (!session.isEmpty()) request.setRawHeader("Tpl-Session", session.toUtf8());

	QMap<QString, QVariant> real_req;

	real_req["action"] = func;
	real_req["parameters"] = req;
	real_req["editor"] = TPLV3_VERSION;

	struct request *rq = new struct request;
	rq->obj = obj;
	rq->member = member;
	rq->extra = extra;

	qDebug("sending query: %s", qPrintable(QtJson::encode(real_req)));

	QNetworkReply *reply = http.post(request, QtJson::encode(real_req));
	connect(reply, SIGNAL(uploadProgress(qint64,qint64)), this, SLOT(on_dataSendProgress(qint64,qint64)));
	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(on_dataReadProgress(qint64,qint64)));

	reply->setProperty("rq_object", (quint64)rq);

	return reply;
}

void ServerInterface::setUpProgressReceiver(QNetworkReply*reply, QObject *obj) {
	struct request *rq = (struct request*)(reply->property("rq_object").toULongLong());

	rq->up_progress = obj;
}

void ServerInterface::on_dataSendProgress(qint64 done, qint64 total) {
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
	if (reply == NULL) return;

	struct request *rq = (struct request*)(reply->property("rq_object").toULongLong());

	QObject *obj = rq->up_progress;
	if (obj == NULL) return;

	QMetaObject::invokeMethod(obj, "dataSendProgress", Qt::QueuedConnection, Q_ARG(int, done), Q_ARG(int, total));
}

void ServerInterface::setSession(QString s) {
	session = s;
}

bool ServerInterface::hasSession() {
	return (!(session.isEmpty()));
}

void ServerInterface::copyToFile() {
	// copy from QNetworkReply*
	QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
	if (reply == NULL) return;

	struct request *rq = (struct request*)(reply->property("rq_object").toULongLong());

	rq->file->write(reply->readAll());
}

void ServerInterface::on_requestFinished(QNetworkReply *reply) {
	struct request *rq = (struct request*)(reply->property("rq_object").toULongLong());

	QBuffer *buf = rq->buffer;
	QObject *extra = rq->extra;
	QObject *obj = rq->obj;
	QProgressDialog *down_progress = rq->down_progress;
	QFile *file = rq->file;
	const char *member = rq->member;
	delete rq;

	if (reply->error() != QNetworkReply::NoError) {
		qDebug("ERROR (error=%d)", reply->error());
		if (buf) delete buf;
		return;
	}

	if (down_progress != NULL) {
		// chances are we were saving to a file
		file->close();
		delete file;
		delete down_progress;
		return;
	}

	QByteArray data = reply->readAll();

	qDebug("result=%s", data.data());

	QVariant result = QtJson::decode(data);

	if (!result.isValid()) {
		qDebug("Invalid answer from server: %s", data.data());
		QMessageBox::warning(NULL, "Server Error", "Invalid answer from server:\n" + buf->buffer(), QMessageBox::Cancel);
	}

	delete buf;

	if ((result.isValid()) && (!result.toMap()["Executed"].toBool())) {
		qDebug("Error from Server: %s", qPrintable(result.toMap()["Message"].toString()));
		QMessageBox::warning(NULL, "Server Error", result.toMap()["Message"].toString(), QMessageBox::Cancel);
		result = QVariant();
	}

	if ((obj != NULL) && (member != NULL)) {
		if (!QMetaObject::invokeMethod(obj, member, Qt::QueuedConnection, Q_ARG(QVariant, result.toMap()["Result"]), Q_ARG(QObject*, extra))) qDebug("Call slot: failed!");
	}
}


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
#include "QtWddx.hpp"

#include <QMessageBox>
#include <QSslSocket>

ServerInterface::ServerInterface(QObject *parent) {
	// define some stuff thanks to uri
	connect(&http, SIGNAL(requestFinished(int,bool)), this, SLOT(on_requestFinished(int,bool)));
	connect(&http, SIGNAL(requestStarted(int)), this, SLOT(on_requestStarted(int)));
	connect(&http, SIGNAL(dataSendProgress(int,int)), this, SLOT(on_dataSendProgress(int,int)));
	connect(&http, SIGNAL(dataReadProgress(int,int)), this, SLOT(on_dataReadProgress(int,int)));
	connect(&http, SIGNAL(sslErrors(const QList<QSslError>&)), this, SLOT(on_sslErrors(const QList<QSslError>&)));
	http.setHost(uri.host(), uri.port(80));

	timer.setInterval(30000);
	timer.setSingleShot(false);
	timer.start();
	connect(&timer, SIGNAL(timeout()), this, SLOT(timer_keepAlive()));
}

void ServerInterface::on_sslErrors(const QList<QSslError> & errors) {
	http.ignoreSslErrors();
}

void ServerInterface::timer_keepAlive() {
	if (session.isEmpty()) return;
	sendRequest("Session.keepAlive", QVariant(), NULL, NULL, NULL);
}

void ServerInterface::setGateway(QUrl url) {
	qDebug("url=%s scheme=%s", qPrintable(url.toString()), qPrintable(url.scheme()));
	if (url.scheme() == "http") {
		uri = url;
		http.setHost(uri.host(), QHttp::ConnectionModeHttp, uri.port(80));
	} else if (url.scheme() == "https") {
		uri = url;
                if (!QSslSocket::supportsSsl()) {
                    QMessageBox::warning(NULL, tr("Configuration Error"), tr("Impossible to complete the requested operation, SSL support not available."), QMessageBox::Cancel);
                    // configure anyway, we know it'll fail
                }
		http.setHost(uri.host(), QHttp::ConnectionModeHttps, uri.port(443));
	}

	return;
}

int ServerInterface::sendRequestDownload(const QString func, const QVariant &req, QString save_as) {
	// open file for output
	QFile *file = new QFile(save_as);
	if (!file->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QMessageBox::critical(NULL, tr("File save error"), tr("Could not open file %1 for writing").arg(save_as), QMessageBox::Cancel);
		return 0;
	}

	QHttpRequestHeader header("POST", uri.encodedPath());
	header.setValue("Host", uri.host());
	header.setValue("Content-Type", "text/xml+wddx");
	header.setValue("User-Agent", "TPL/1.0 (" TPLV3_VERSION ")");
	header.setValue("X-Tpl-Editor", TPLV3_VERSION);
	header.setValue("X-Ipc", "TPL_WDDX");
	if (!session.isEmpty()) header.setValue("Tpl-Session", session);

	QMap<QString, QVariant> real_req;

	real_req["action"] = func;
	real_req["parameters"] = req;
	real_req["editor"] = TPLV3_VERSION;

	QProgressDialog *down_progress = new QProgressDialog(tr("Downloading..."), tr("Cancel"), 0, 0);
	down_progress->setLabelText(tr("Downloading to file %1").arg(save_as));
	down_progress->setMinimumDuration(100);
	down_progress->setValue(-1);

	int id = http.request(header, QtWddx::serialize(real_req).toUtf8(), file);

	struct request *rq = new struct request;
	rq->down_progress = down_progress;
	rq->file = file;
	requests.insert(id, rq);

	return id;
}

void ServerInterface::on_dataReadProgress(int done, int total) {
	if (!requests.contains(cur_req)) return;
	QProgressDialog *down_progress = requests[cur_req]->down_progress;
	if (down_progress == NULL)
		return;

	down_progress->setMaximum(total);
	down_progress->setValue(done);
}

int ServerInterface::sendRequest(const QString func, const QVariant &req, QObject *obj, const char *member, QObject *extra) {
	QHttpRequestHeader header("POST", uri.encodedPath());
	header.setValue("Host", uri.host());
	header.setValue("Content-Type", "text/xml+wddx");
	header.setValue("User-Agent", "TPL/1.0 (" TPLV3_VERSION ")");
	header.setValue("X-Tpl-Editor", TPLV3_VERSION);
	if (!session.isEmpty()) header.setValue("Tpl-Session", session);

	// This buffer will hold the result
	QBuffer *buf = new QBuffer();
	buf->open(QBuffer::ReadWrite);

	QMap<QString, QVariant> real_req;

	real_req["action"] = func;
	real_req["parameters"] = req;
	real_req["editor"] = TPLV3_VERSION;

	//qDebug("sending query: %s", qPrintable(QtWddx::serialize(real_req)));

	int id = http.request(header, QtWddx::serialize(real_req).toUtf8(), buf);

	struct request *rq = new struct request;
	rq->buffer = buf;
	rq->obj = obj;
	rq->member = member;
	rq->extra = extra;
	requests.insert(id, rq);

	return id;
}

void ServerInterface::setUpProgressReceiver(int rq, QObject *obj) {
	if (!requests.contains(rq)) return;

	requests[rq]->up_progress = obj;
}

void ServerInterface::on_dataSendProgress(int done, int total) {
	if (!requests.contains(cur_req)) return;
	QObject *obj = requests[cur_req]->up_progress;
	if (obj == NULL) return;

	QMetaObject::invokeMethod(obj, "dataSendProgress", Qt::QueuedConnection, Q_ARG(int, cur_req), Q_ARG(int, done), Q_ARG(int, total));
}

void ServerInterface::setSession(QString s) {
	session = s;
}

bool ServerInterface::hasSession() {
	return (!(session.isEmpty()));
}

void ServerInterface::on_requestStarted(int id) {
	cur_req = id;
}

void ServerInterface::on_requestFinished(int id, bool error) {
	if (!requests.contains(id)) {
//		qDebug("Ignoring result for request %d", id);
		return;
	}

	struct request *rq = requests.value(id);
	requests.remove(id);
	QBuffer *buf = rq->buffer;
	QObject *extra = rq->extra;
	QObject *obj = rq->obj;
	QProgressDialog *down_progress = rq->down_progress;
	QFile *file = rq->file;
	const char *member = rq->member;
	delete rq;

	if (error) {
		qDebug("ERROR (error=%s)", qPrintable(http.errorString()));
		delete buf;
		return;
	}

	if (down_progress != NULL) {
		// chances are we were saving to a file
		file->close();
		delete file;
		delete down_progress;
		return;
	}

	QString data = QString::fromUtf8(buf->buffer().data());

	//qDebug("result=%s", qPrintable(data));

	QVariant result = QtWddx::unserialize(buf->buffer());

	if (!result.isValid()) {
		qDebug("Invalid answer from server: %s", buf->buffer().data());
		QMessageBox::warning(NULL, "Server Error", "Invalid answer from server:\n" + buf->buffer(), QMessageBox::Cancel);
	}

	delete buf;

	if ((result.isValid()) && (!result.toMap()["Executed"].toBool())) {
		qDebug("Error from Server: %s", qPrintable(result.toMap()["Message"].toString()));
		QMessageBox::warning(NULL, "Server Error", result.toMap()["Message"].toString(), QMessageBox::Cancel);
		result = QVariant();
	}

	if ((obj != NULL) && (member != NULL)) {
		if (!QMetaObject::invokeMethod(obj, member, Qt::QueuedConnection, Q_ARG(int, id), Q_ARG(QVariant, result.toMap()["Result"]), Q_ARG(QObject*, extra))) qDebug("Call slot: failed!");
	}
}


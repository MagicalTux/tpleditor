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

#include "MainTreeView.hpp"
#include "TplModelNode.hpp"
#include "TplMimeData.hpp"

#include <QDragEnterEvent>
#include <QFile>
#include <QUrl>
#include <QFileInfo>
#include <QPersistentModelIndex>

MainTreeView::MainTreeView(QWidget *parent): QTreeView(parent) {
	setAutoExpandDelay(1500);

}

void MainTreeView::dragEnterEvent(QDragEnterEvent *event) {
	// TODO: handle copy of multiple files/folders
	qDebug("Format: %s", qPrintable(event->mimeData()->formats().join(",")));

	if ((event->source() == this) && (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
		QByteArray datalist = event->mimeData()->data("application/x-qabstractitemmodeldatalist");

		event->setDropAction(Qt::MoveAction);
		event->accept();
		return;
	}

	if (event->mimeData()->hasUrls()) {
		QList<QUrl> urlz = event->mimeData()->urls();
		bool accept = true;
		for(int i = 0; i < urlz.size(); i++) {
			if (urlz.at(i).scheme() != "file") accept = false;
			qDebug("found url: %s", qPrintable(urlz.at(i).toString()));
		}
		if (accept) {
			event->setDropAction(Qt::CopyAction);
			event->accept();
		}
	}
}

void MainTreeView::dragMoveEvent(QDragMoveEvent *event) {
	QModelIndex idx = indexAt(event->answerRect().center());
	if (!idx.isValid()) {
		event->ignore();
		return;
	}

	// TODO: make the current skin/folder highlighted to mark where the drop will happen
	//setSelection(pos, QItemSelectionModel::SelectCurrent);
//	if (currentIndex().isValid()) {
//		qDebug("valid: %s!", qPrintable(currentIndex().data(Qt::DisplayRole).toString()));
//	}

	event->accept(visualRect(idx));
}

void MainTreeView::dropEvent(QDropEvent *event) {
	if ((event->source() == this) && (event->mimeData()->hasFormat("application/x-qabstractitemmodeldatalist"))) {
		// Mov3d!
		const TplMimeData *mime = qobject_cast<const TplMimeData*>(event->mimeData());
		if (mime == NULL) {
			event->ignore();
			return;
		}

		QModelIndex idx = indexAt(event->pos());
		TplModelNode *node = qobject_cast<TplModelNode*>(idx.data(Qt::UserRole).value<QObject*>());

		// Fetch  the closest subfolder, or the skin if none
		while ((node != NULL) && (node->getType() != TplModelNode::SKIN) && (node->getType() != TplModelNode::FOLDER))
			node = node->getParent();

		if (node == NULL) { // could not find a node, cancel the drag&drop event
			event->ignore();
			return;
		}

		event->setDropAction(Qt::MoveAction);
		event->accept();

		QList<QPersistentModelIndex> moved = mime->indexes();
		for(int i = 0; i<moved.size(); i++) {
			TplModelNode *child = qobject_cast<TplModelNode*>(moved.at(i).data(Qt::UserRole).value<QObject*>());
			if (child->getType() != TplModelNode::FILE) continue;
			child->serverRelocate(node);
		}
		return;
	}

	if (event->mimeData()->hasUrls()) {
		QList<QUrl> urlz = event->mimeData()->urls();
		QList<QString> fileslist;
		bool accept = true;
		for(int i = 0; i < urlz.size(); i++) {
			if (urlz.at(i).scheme() != "file") accept = false;
			
			// build QFile and check if file exists
			QFile f(urlz.at(i).toLocalFile());
			if (!f.exists()) {
				accept = false;
			} else {
				fileslist.append(f.fileName());
			}
		}

		if (!accept) {
			event->ignore();
			return;
		}

		QModelIndex idx = indexAt(event->pos());
		TplModelNode *node = qobject_cast<TplModelNode*>(idx.data(Qt::UserRole).value<QObject*>());

		// Fetch  the closest subfolder, or the skin if none
		while ((node != NULL) && (node->getType() != TplModelNode::SKIN) && (node->getType() != TplModelNode::FOLDER))
			node = node->getParent();

		if (node == NULL) { // could not find a node, cancel the drag&drop event
			event->ignore();
			return;
		}

		qDebug("accepting event!");
		event->setDropAction(Qt::CopyAction);
		event->acceptProposedAction();

		for(int i = 0; i < fileslist.size(); i++) {
			QFile f(fileslist.at(i));
			f.open(QIODevice::ReadOnly);
			QByteArray filedata = f.readAll();
			f.close();

			QMap<QString, QVariant> params;
			params["FileName"] = QFileInfo(f).fileName();
			params["data"] = filedata; // will be encoded to base64 by wddx

			node->sendFile(params);
		}
	}
}

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

#include "TplModelNode.hpp"
#include "TplModelRoot.hpp"
#include "ServerInterface.hpp"
#include "PropertiesPage.hpp"
#include "PropertiesTpl.hpp"

#include <QWidget>
#include <QMimeData>
#include <QMessageBox>

TplModelNode::TplModelNode(const QString _name, const QVariant _idx, NodeType _type, TplModelNode *_parent, TplModelRoot &_model, ServerInterface &_srv): name(_name), model(_model), srv(_srv) {
	idx = _idx;
	parent = _parent;
	type = _type;

	has_data = false;
	dummy_created = false;
	icon = model.getNodeIcon(this);
	file_progress = false;
}

TplModelNode::~TplModelNode() {
	// Free items
	int size = children.size();
	for(int i = 0; i<size; i++) {
		delete children.at(i);
	}
	children.clear();
}

TplModelNode::NodeType TplModelNode::getType() const {
	return type;
}

TplModelNode *TplModelNode::addChild(int key, QString childname, NodeType childtype, QVariant childextra) {
	return addChild<int>(key, childname, childtype, childextra);
}

TplModelNode *TplModelNode::addChild(QString key, QString childname, NodeType childtype, QVariant childextra) {
	return addChild<QString>(key, childname, childtype, childextra);
}

void TplModelNode::dataLoaded() {
	if (dummy_created) {
		// Delete only child, if any
		while(!children.isEmpty()) {
			model.beginRemoveRows(getIndex(), 0, 0);
			delete children.takeFirst();
			model.endRemoveRows();
		}
		dummy_created = false;
	}
	has_data = true;
}

QMimeData *TplModelNode::mimeData(QMimeData *mime) {
	if (extra.toMap()["TextRepresentation"].isValid()) {
		mime->setText(extra.toMap()["TextRepresentation"].toString());
	}

	return mime;
}

TplModelNode *TplModelNode::findChild(QString key) {
	return findChild<QString>(key);
}

TplModelNode *TplModelNode::findChild(int key) {
	return findChild<int>(key);
}

template<typename T> TplModelNode *TplModelNode::findChild(T key) {
	// TODO: use an index for faster results (QMap)
	for(QList<TplModelNode*>::iterator i=children.begin(); i != children.end(); i++) {
		if (key == (*i)->getKey().value<T>()) return (*i);
	}

	return NULL;
}

template<typename T> TplModelNode *TplModelNode::addChild(T key, QString childname, NodeType childtype, QVariant childextra) {
	dataLoaded();

	TplModelNode *foundChild = findChild(key);
	if (foundChild != NULL) return foundChild;

	//TODO: handle deletion

	TplModelNode *newnode = new TplModelNode(childname, key, childtype, this, model, srv);
	if (childextra.isValid()) newnode->extra = childextra;
	addChild(newnode);

	return newnode;
}

void TplModelNode::addChild(TplModelNode *child) {
	model.beginInsertRows(getIndex(), size(), size());
	children.append(child);
	model.endInsertRows();
}

TplModelNode *TplModelNode::createTemporaryChild(NodeType type) {
	if (!has_data) return NULL;

	model.beginInsertRows(getIndex(), size(), size());
	TplModelNode *newnode = new TplModelNode(QString(""), QString(""), type, this, model, srv);
	children.append(newnode);
	newnode->has_data = true;
	model.endInsertRows();

	return newnode;
}

void TplModelNode::removeChild(TplModelNode *node) {
	int id = children.indexOf(node);
	qDebug("child id=%d, size=%d", id, children.size());
	if (id == -1) return;
	model.beginRemoveRows(getIndex(), id, id);
	children.takeAt(id);
	model.endRemoveRows();
}

bool TplModelNode::edit(const QVariant &value, int role) {
	if (role != Qt::EditRole) return false; // you can't edit other roles
	if (idx.type() != QVariant::String) {
		qDebug("Can't edit a non-string index node");
		return false;
	}
	if (idx == value) return true; // ?

	old_idx = idx;
	old_name = name;

	idx = value;
	name = value.toString();

	QModelIndex idx = getIndex();
	model.dataChanged(idx, idx);

	QMap<QString, QVariant> query = getNodeData().toMap();

	switch(type) {
		case PAGE:
			if (old_idx.toString().isEmpty()) {
				query["action"] = "Skins.createPage";
			} else {
				query["action"] = "Skins.renamePage";
				query["OldPage"] = old_idx;
			}
			break;
		case TEMPLATE:
			if (old_idx.toString().isEmpty()) {
				query["action"] = "Skins.createTemplate";
			} else {
				query["action"] = "Skins.renameTemplate";
				query["OldTemplate"] = old_idx;
			}
			break;
		case LANGUAGE:
			if (old_idx.toString().isEmpty()) {
				query["action"] = "Skins.createLanguage";
			} else {
				query["action"] = "Skins.changeLanguage";
				query["OldLanguage"] = old_idx;
			}
			break;
		case FOLDER:
			if (old_idx.toString().isEmpty()) {
				query["action"] = "System.createDir";
			} else {
				query["action"] = "System.renameDir";
				query["OldName"] = old_idx;
			}
			break;
		case FILE:
			if (old_idx.toString().isEmpty()) {
				// Nothing can happen, this will fail anyway
				query["action"] = "System.createFile";
			} else {
				query["action"] = "System.renameFile";
				query["OldName"] = old_idx;
			}
			break;
		default:
			qDebug("unhandled type, do me now!");
			return false;
	}
	srv.sendRequest(query["action"].toString(), query, this, "handleRenameResult", NULL);
	return true;
}

void TplModelNode::getTplType(QObject *target, const char *func) {
	QMap<QString, QVariant> query = getNodeData().toMap();
	query["action"] = "TplType.getTemplateType";

	srv.sendRequest(query["action"].toString(), query, target, func, NULL);
}

void TplModelNode::getAllTplTypes(QObject *target, const char *func) {
	srv.sendRequest("TplType.listAll", QVariant(), target, func, NULL);
}

void TplModelNode::saveToFile(QString filename) {
	srv.sendRequestDownload("Skins.save", getNodeData().toMap(), filename);
}

void TplModelNode::testEditableOption(QObject *target, const char *func, int tpl_type, QString options, QString option, QVariant value) {
	QMap<QString, QVariant> query = getNodeData().toMap();
	QMap<QString, QVariant> set;
	set["var"] = option;
	set["val"] = value;
	query["action"] = "TplType.editableOptions";
	query["Tpl_Type__"] = tpl_type;
	query["Options"] = options;
	query["set"] = set;

	srv.sendRequest(query["action"].toString(), query, target, func, NULL);
}

void TplModelNode::getEditableOptions(QObject *target, const char *func, int tpl_type, QString options) {
	QMap<QString, QVariant> query = getNodeData().toMap();
	query["action"] = "TplType.editableOptions";
	query["Tpl_Type__"] = tpl_type;
	query["Options"] = options;

	srv.sendRequest(query["action"].toString(), query, target, func, NULL);
}

void TplModelNode::setTemplateType(QObject *target, const char *func, int tpl_type, QString options) {
	QMap<QString, QVariant> query = getNodeData().toMap();
	query["action"] = "TplType.setTemplateType";
	query["Tpl_Type__"] = tpl_type;
	query["Options"] = options;

	srv.sendRequest(query["action"].toString(), query, target, func, NULL);
}

void TplModelNode::getPageProperties(QObject *target, const char *func) {
	QMap<QString, QVariant> query = getNodeData().toMap();
	query["action"] = "Skins.getPageProperties";

	srv.sendRequest(query["action"].toString(), query, target, func, NULL);
}

void TplModelNode::setPageProperties(QObject *target, const char *func, QString charset, QString content_type) {
	QMap<QString, QVariant> query = getNodeData().toMap();
	query["action"] = "Skins.setPageProperties";
	query["Charset"] = charset;
	query["Content_Type"] = content_type;

	srv.sendRequest(query["action"].toString(), query, target, func, NULL);
}

void TplModelNode::putInProd() {
	QMap<QString, QVariant> query = getNodeData().toMap();
	query["action"] = "Skins.compilePage";

	srv.sendRequest(query["action"].toString(), query, NULL, NULL, NULL);
}

void TplModelNode::serverDelete() {
	QMap<QString, QVariant> query = getNodeData().toMap();
	switch(type) {
		case PAGE:
			query["action"] = "Skins.deletePage";
			break;
		case TEMPLATE:
			query["action"] = "Skins.deleteTemplate";
			break;
		case LANGUAGE:
			query["action"] = "Skins.deleteLanguage";
			break;
		case FILE:
			query["action"] = "System.deleteFile";
			break;
		case FOLDER:
			query["action"] = "System.deleteDir";
			break;
		default:
			qDebug("Unhandled type for deletion :(");
			return;
	}
	srv.sendRequest(query["action"].toString(), query, this, "handleDeletion", NULL);
}

void TplModelNode::serverRelocate(TplModelNode *newparent) {
	if (getType() != FILE) return;
	if (newparent == parent) return; // ignore it

	QMap<QString, QVariant> req = getNodeData().toMap();
	req["NewLocation"] = newparent->getNodeData();

	srv.sendRequest("System.moveFile", req, this, "serverRelocateResult", newparent);
}

void TplModelNode::serverRelocateResult(QVariant data, QObject *_newparent) {
	TplModelNode *newparent = qobject_cast<TplModelNode*>(_newparent);
	if (newparent == NULL) return;
	if (!data.isValid()) return;
	if (!data.toMap()["Success"].toBool()) return;

	parent->removeChild(this);
	parent = newparent;
	newparent->addChild(this);
	extra = data;
}

void TplModelNode::handleDeletion(QVariant data, QObject *) {
	if (data.toMap()["Success"].toBool()) {
		// ok, server said "OK", so delete myself!
		getParent()->removeChild(this);
		deleteLater();
	}
}

void TplModelNode::handleRenameResult(QVariant data, QObject *) {
	if (!data.isValid()) {
		qDebug("xxx error");
		if (!old_idx.toString().isEmpty()) {
			qDebug("back to old name");
			// back to my old name
			idx = old_idx;
			name = old_name;
			QModelIndex idx = getIndex();
			model.dataChanged(idx, idx);
		} else {
			qDebug("removing self");
			// let me die alone
			getParent()->removeChild(this);
			deleteLater();
		}
		return;
	}

	if (!data.toMap()["Success"].toBool()) {
		if (!old_idx.toString().isEmpty()) {
			// back to my old name
			idx = old_idx;
			name = old_name;
			QModelIndex idx = getIndex();
			model.dataChanged(idx, idx);
		} else {
			// let me die alone
			getParent()->removeChild(this);
			deleteLater();
		}
		return;
	}

	// refresh icon
	icon = model.getNodeIcon(this);

	if (old_idx.toString().isEmpty()) {
		// we need to say we don't know what we own
		has_data = false;
		QModelIndex idx = getIndex();
		model.dataChanged(idx, idx);
	}

	// save extra
	extra = data;
}

void TplModelNode::setEditor(QWidget *editor) {
	connect(editor, SIGNAL(destroyed()), this, SLOT(editorDestroyed()));
}

void TplModelNode::editorDestroyed() {
	switch(idx.type()) {
		case QVariant::Int:
			if (idx.value<int>() != 0) return;
			break;
		case QVariant::String:
			if (!idx.toString().isEmpty()) return;
			break;
		default:
			return;
	}
	getParent()->removeChild(this);
	deleteLater();
}

QVariant TplModelNode::getKey() {
	return idx;
}

QVariant TplModelNode::getData(int role) {
	switch(role) {
		case Qt::DisplayRole: // text
			return getDisplayName();
		case Qt::DecorationRole: // icon
			if (icon != NULL) return QVariant(*icon);
			return QVariant();
		case Qt::SizeHintRole:
			return QSize(20, 20);
		case Qt::ToolTipRole: // tooltip, if any
			return QString("test");
		case Qt::EditRole:
			return name;
		case Qt::UserRole:
			return qVariantFromValue(qobject_cast<QObject*>(this));
	}

	return QVariant();
}

QVariant TplModelNode::getNodeData(QVariant initial) {
	QMap<QString, QVariant> res(initial.toMap());

	if (getParent() != NULL) res = getParent()->getNodeData(res).toMap();

	switch(type) {
		case FILE:
			res["File"] = getKey();
			break;
		case FOLDER:
			// we are only appending
			res["Folder"] = res["Folder"].toString() + QString("/") + getKey().toString();
			break;
		case LANGUAGE:
			res["Language"] = getKey();
			break;
		case TEMPLATE:
			res["Template"] = getKey();
			break;
		case PAGE:
			res["Page"] = getKey();
			break;
		case SKIN:
			res["Skin"] = getKey();
			break;
		case LOADING:
			return QVariant();
		case ROOT:
			// nothing
			break;
	}

	return res;
}

QVariant TplModelNode::getExtra() {
	return extra;
}

Qt::ItemFlags TplModelNode::flags() {
	if (type == LOADING) return 0; // no flags for loading

	Qt::ItemFlags flags;

	flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

	switch(type) {
		case TEMPLATE:
		case PAGE:
		case LANGUAGE:
		case FILE:
			flags |= Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
			break;
		case FOLDER:
			flags |= Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
			break;
		default:
			break;
			// nothing
	}

	return flags;
	// return Qt::ItemIsDragEnabled and Qt::ItemIsDropEnabled too?
	// also return Qt::ItemIsEditable if page name/template name
}

int TplModelNode::getRowId() {
	if (parent == NULL) return 0;
	return parent->getChildRow(this);
}

int TplModelNode::getChildRow(TplModelNode *chld) const {
	return children.indexOf(chld);
}

const QString TplModelNode::getDisplayName() const {
	if (idx.type() == QVariant::String) return idx.toString();
	return QString("%1 (%2)").arg(name).arg(idx.toString());
}

bool TplModelNode::hasData() const {
	return has_data;
}

bool TplModelNode::hasChildren() {
	if (type == LOADING) return false; // a loading has no children
	if (type == LANGUAGE) return false; // languages don't have children either
	if (type == FILE) return false; // files have no reason to have children
	if (has_data) return !children.isEmpty();
	return true;
}

QModelIndex TplModelNode::getIndex() {
	if (type == ROOT) return QModelIndex();
	return model.createIndex(getRowId(), 0, this);
}

int TplModelNode::size() {
	if (has_data) return children.size();

	if ((type != LOADING) && (!dummy_created)) {
		dummy_created = true;
		model.beginInsertRows(getIndex(), 0, 0);
		children.append(new TplModelNode(tr("Loading..."), 0, LOADING, this, model, srv));
		model.endInsertRows();

		model.loadNodeData(this);
	}

	return children.size(); // "loading"
}

TplModelNode *TplModelNode::getChild(int row) {
	if (row < 0) {
		qDebug("Attempt to access minus row %d", row);
		return NULL;
	}

	if (row >= size()) {
		qDebug("Attempt to access too high row %d", row);
		return NULL;
	}

	return children.at(row);
}

TplModelNode *TplModelNode::getParent() const {
	return parent;
}

TplModelNode *TplModelNode::getParentOfType(NodeType type) {
	TplModelNode *cur = this;
	while(cur->getType() != type) {
		cur = cur->getParent();
		if (cur == NULL) return NULL;
	}
	return cur;
}

void TplModelNode::restoreFromFile(QString filename, bool merge) {
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(NULL, tr("Server restore"), tr("Failed to open file %1 for server restoration").arg(filename), QMessageBox::Cancel);
		return;
	}

	// need to clear all children
	model.beginRemoveRows(getIndex(), 0, children.size()-1);

	for(int i = children.size()-1; i >= 0; i--) {
		TplModelNode *t = children.takeAt(i);
		delete t;
	}

	model.endRemoveRows();

	file_progress = true;
	file_pos = 0;
	file_total = 1;

	model.dataChanged(getIndex(), getIndex());

	QMap<QString, QVariant> request = getNodeData().toMap();
	request["data"] = file.readAll().toBase64();
	request["merge"] = merge;

	QNetworkReply *reply = srv.sendRequest("Skins.restore", request, this, "handleRestoreResult", NULL);
	srv.setUpProgressReceiver(reply, this);
}

void TplModelNode::handleRestoreResult(QVariant, QObject *) {
	has_data = false;
	dummy_created = false;
	size(); // will trigger creation of "Loading" dummy and reloading of pages list/etc
}

void TplModelNode::sendFile(QVariant filedata) {
	QString filename = filedata.toMap()["FileName"].toString();

	TplModelNode *child = findChild(filename);
	if (child == NULL) {
		child = addChild(filename, filename, FILE);
		child->old_idx = QVariant(QString());
	} else {
		// Let's get a confirmation from user first!
		if (QMessageBox::warning(NULL, "Overwrite file?", "You are about to overwrite a file, do you confirm?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No) {
			return;
		}
		child->old_idx = child->idx;
	}

	child->file_progress = true;
	child->file_pos = 0;
	child->file_total = 1;
	QModelIndex idx = child->getIndex();
	model.dataChanged(idx, idx);

	QNetworkReply *reply = srv.sendRequest("System.addFile", getNodeData(filedata), child, "handleRenameResult", NULL);
	srv.setUpProgressReceiver(reply, child);
}

void TplModelNode::dataSendProgress(int pos, int total) {
	file_progress = pos < total;
	file_pos = pos;
	file_total = total;

	QModelIndex idx = getIndex();
	model.dataChanged(idx, idx);
}

bool TplModelNode::hasProgress() {
	return file_progress;
}

int TplModelNode::getProgress() {
	return file_pos;
}

int TplModelNode::getProgressMax() {
	return file_total;
}

void TplModelNode::editProperties() {
	switch(type) {
		case PAGE:
			new PropertiesPage(NULL, this);
			break;
		case TEMPLATE:
			new PropertiesTpl(NULL, this);
			break;
		default:
			return;
	}
}

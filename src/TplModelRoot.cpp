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

#include "MainWindow.hpp"
#include "ServerInterface.hpp"
#include "TplModelRoot.hpp"
#include "TplModelNode.hpp"
#include "TplMimeData.hpp"

#include <QList>

TplModelRoot::TplModelRoot(QWidget *parent, ServerInterface &_srv): srv(_srv), mw(dynamic_cast<MainWindow&>(*parent)) {
	skin_icon = QIcon(":/images/database.png");
	page_icon = QIcon(":/images/script_code.png");
	tpl_icon = QIcon(":/images/puzzle.png");
	dir_icon = QIcon(":/images/folder.png");
	file_icon = QIcon(":/images/notebook.png");

	root = new TplModelNode("[root]", 0, TplModelNode::ROOT, NULL, *this, srv);
}

TplModelRoot::~TplModelRoot() {
	// Free items
	delete root;
}

QIcon *TplModelRoot::getNodeIcon(TplModelNode *node) {
	switch(node->getType()) {
		case TplModelNode::SKIN:
			return &skin_icon;
		case TplModelNode::PAGE:
			return &page_icon;
		case TplModelNode::TEMPLATE:
			return &tpl_icon;
		case TplModelNode::FOLDER:
			return &dir_icon;
		case TplModelNode::FILE:
			return &file_icon;
		default:
			return NULL;
	}
}

TplModelNode *TplModelRoot::getNode(const QModelIndex &idx) const {
	if (!idx.isValid()) return root;

	return (TplModelNode*)idx.internalPointer();
}

Qt::ItemFlags TplModelRoot::flags(const QModelIndex & index) const {
	TplModelNode *node = getNode(index);
	if (node == NULL) return 0; // no flags since unknown node

	return node->flags();
}

QMimeData *TplModelRoot::mimeData(const QModelIndexList &indexes) const {
	if (indexes.count() != 1) return NULL;

	TplModelNode *node = getNode(indexes.at(0));

	TplMimeData *mime = new TplMimeData(indexes);
	return node->mimeData(mime);
}

QVariant TplModelRoot::data(const QModelIndex &index, int role) const {
	TplModelNode *node = getNode(index);
	if (node == NULL) return QVariant(); // wtf?

	return node->getData(role);
}

bool TplModelRoot::setData(const QModelIndex &index, const QVariant &value, int role) {
	TplModelNode *node = getNode(index);
	if (node == NULL) return false; // wtf?

	return node->edit(value, role);
}

QModelIndex TplModelRoot::index(int row, int column, const QModelIndex &parent) const {
	//qDebug("index of (%d, %d): %d", parent.row(), parent.column(), row);
	TplModelNode *node = getNode(parent);

	TplModelNode *child = node->getChild(row);
	if (child == NULL) return QModelIndex();

	return createIndex(row, column, child);
}

QModelIndex TplModelRoot::parent(const QModelIndex &index) const {
	//qDebug("parent of %d %d (internalId=%d)", index.row(), index.column(), (int)index.internalId());
	if (!index.isValid()) return QModelIndex();

	TplModelNode *node = getNode(index);
	TplModelNode *parent = node->getParent();

	if (parent == root) return QModelIndex();

	if (parent == NULL) {
		qDebug("Crash is coming, this should never happen!");
		return QModelIndex();
	}

	return createIndex(parent->getRowId(), index.column(), parent);
}

QVariant TplModelRoot::headerData(int section, Qt::Orientation orientation, int role) const {
	if (orientation == Qt::Vertical) return QVariant();

	if (role != Qt::DisplayRole) return QVariant();

	switch(section) {
		case 0: return QString(tr("Name"));
	}

	return QVariant();
}

int TplModelRoot::rowCount(const QModelIndex &parent) const {
	TplModelNode *node = getNode(parent);

	return node->size();
}

int TplModelRoot::columnCount(const QModelIndex &parent) const {
	return 1;
}

void TplModelRoot::refresh() {
	srv.sendRequest("Skins.listAll", QVariant(), this, "updateServersList", NULL);
}

void TplModelRoot::updateServersList(int id, QVariant data, QObject *extra) {
	// Data is an array (list), containing struct (qmap) of values, including Server__ (id) and Name (string)
	QList<QVariant> list = data.toList();
	int max = list.size();

	for(int i=0; i<max; i++) {
		QMap<QString, QVariant> serv = list.at(i).toMap();
		int id = serv["Skin_Id"].toInt();
		QString name = serv["Name"].toString();
		serv["Languages"] = serv["Languages"].toString().split(",", QString::SkipEmptyParts);

		root->addChild(id, name, TplModelNode::SKIN, serv);
	}
}

bool TplModelRoot::hasChildren(const QModelIndex &parent) const {
	if (!parent.isValid()) return true; // what should I do?
	TplModelNode *node = (TplModelNode*)parent.internalPointer();

	return node->hasChildren();
}

void TplModelRoot::setPagesList(int id, QVariant data, QObject *extra) {
	TplModelNode *node = dynamic_cast<TplModelNode*>(extra);
	if (node == NULL) {
		qDebug("Could not cast our node back to TplModelNode");
		return;
	}

	node->dataLoaded();

	QList<QVariant> list = data.toList();
	int max = list.size();

	for(int i=0; i<max; i++) {
		QMap<QString, QVariant> serv = list.at(i).toMap();
		QString name = serv["Page"].toString();

		node->addChild(name, name, TplModelNode::PAGE);
	}
}

void TplModelRoot::setTemplatesList(int id, QVariant data, QObject *extra) {
	TplModelNode *node = dynamic_cast<TplModelNode*>(extra);
	if (node == NULL) {
		qDebug("Could not cast our node back to TplModelNode");
		return;
	}

	node->dataLoaded();

	QList<QVariant> list = data.toList();
	int max = list.size();

	for(int i=0; i<max; i++) {
		QMap<QString, QVariant> tpl = list.at(i).toMap();
		QString name = tpl["Name"].toString();
		// NB: we also got "Languages" as a string like "es_ES,fr_BE,fr_FR,nl_NL,pt_PT"
		QStringList lngs = tpl["Languages"].toString().split(",", QString::SkipEmptyParts);

		QMap<QString, QString> lngs_map;

		TplModelNode *subnode = node->addChild(name, name, TplModelNode::TEMPLATE);
		if (subnode == NULL) continue;
		for(int j=0; j<lngs.size(); j++) {
			subnode->addChild(lngs.at(j), lngs.at(j), TplModelNode::LANGUAGE);
		}
	}
}

void TplModelRoot::setFilesList(int id, QVariant data, QObject *extra) {
	TplModelNode *node = dynamic_cast<TplModelNode*>(extra);
	if (node == NULL) {
		qDebug("Could not cast our node back to TplModelNode");
		return;
	}

	node->dataLoaded();

	QList<QVariant> list = data.toList();
	int max = list.size();

	for(int i=0; i<max; i++) {
		QMap<QString, QVariant> entry = list.at(i).toMap();
		QString name = entry["FileName"].toString();
		TplModelNode::NodeType type = TplModelNode::FOLDER;
		if (entry["Type"].toString() == "FILE") type = TplModelNode::FILE;

		node->addChild(name, name, type, entry);
	}
}

void TplModelRoot::loadNodeData(TplModelNode *node) {
	QMap<QString, QVariant> query;

	switch(node->getType()) {
		case TplModelNode::ROOT:
		case TplModelNode::LOADING:
			return; // already got the data at this point... in theory, or no need for data
		case TplModelNode::SKIN:
			srv.sendRequest("Skins.getPages", node->getNodeData(), this, "setPagesList", node);
			srv.sendRequest("System.listFiles", node->getNodeData(), this, "setFilesList", node);
			break;
		case TplModelNode::FOLDER:
			srv.sendRequest("System.listFiles", node->getNodeData(), this, "setFilesList", node);
			break;
		case TplModelNode::PAGE:
			srv.sendRequest("Skins.getTemplates", node->getNodeData(), this, "setTemplatesList", node);
			break;
		case TplModelNode::TEMPLATE:
		case TplModelNode::LANGUAGE:
			// get templates for parent page
			while(node->getType() != TplModelNode::PAGE) node = node->getParent();
			srv.sendRequest("Skins.getTemplates", node->getNodeData(), this, "setTemplatesList", node);
			break;
		default:
			qDebug("unhandled node type asked for loading, denied!");
	}
}


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

#ifndef TPLMODELNODE_HPP
#define TPLMODELNODE_HPP

#include <QString>
#include <QList>
#include <QObject>
#include <QMap>
#include <QVariant>
#include <QProgressDialog>

class TplModelRoot;
class QModelIndex;
class ServerInterface;
class QIcon;
class QMimeData;

class TplModelNode: public QObject {
	Q_OBJECT;

public:
	enum NodeType {
		ROOT,
		SKIN,
		PAGE,
		TEMPLATE,
		LANGUAGE,
		LOADING, /* special type, disabled index saying "Loading in progress..." */
		FOLDER, /* filesystem folder */
		FILE /* filesystem file */
	};

	TplModelNode(const QString _name, const QVariant _idx, enum TplModelNode::NodeType _type, TplModelNode *_parent, TplModelRoot &_model, ServerInterface &_srv);
	virtual ~TplModelNode();

	NodeType getType() const;

	void dataLoaded();
	TplModelNode *addChild(int key, QString name, TplModelNode::NodeType childtype, QVariant extra = QVariant());
	TplModelNode *addChild(QString key, QString name, TplModelNode::NodeType childtype, QVariant extra = QVariant());

	TplModelNode *findChild(QString key);
	TplModelNode *findChild(int key);

	void removeChild(TplModelNode *node);
	void serverDelete(); // ask to server for deletion, then delete
	void serverRelocate(TplModelNode *newparent); // relocate child to new parent

	void setEditor(QWidget *editor);
	bool edit(const QVariant &value, int role = Qt::EditRole);

	QVariant getData(int role);
	Qt::ItemFlags flags();
	QVariant getNodeData(QVariant initial = QVariant());

	TplModelNode *createTemporaryChild(NodeType type);

	int getRowId(); // Will call parent's getChildRow with "this"
	virtual const QString getDisplayName() const;

	virtual bool hasData() const;

	TplModelNode *getParent() const;
	TplModelNode *getParentOfType(TplModelNode::NodeType type);
	TplModelNode *getChild(int row);

	const int getChildRow(TplModelNode * const chld) const;

	int size();
	bool hasChildren();
	QModelIndex getIndex();

	QVariant getKey();
	QVariant getExtra();

	void sendFile(QVariant filedata);
	bool hasProgress();
	int getProgress();
	int getProgressMax();

	QMimeData *mimeData(QMimeData*);

	void editProperties();

	void getPageProperties(QObject *target, const char *func);
	void setPageProperties(QObject *target, const char *func, QString, QString);

	void getTplType(QObject *target, const char *func);
	void getAllTplTypes(QObject *target, const char *func);
	void getEditableOptions(QObject *target, const char *func, int tpl_type, QString options);
	void testEditableOption(QObject *target, const char *func, int tpl_type, QString options, QString option, QVariant value);
	void setTemplateType(QObject *target, const char *func, int tpl_type, QString options);

	void saveToFile(QString filename);
	void restoreFromFile(QString filename);

	void putInProd();

public slots:
	void editorDestroyed();
	void handleRenameResult(int id, QVariant data, QObject *extra);
	void handleDeletion(int id, QVariant data, QObject *extra);
	void serverRelocateResult(int id, QVariant data, QObject *extra);
	void handleRestoreResult(int id, QVariant data, QObject *);

	// files upload
	void dataSendProgress(int, int, int);

protected:
	QVariant idx;
	NodeType type;
	TplModelNode *parent;
	QString name;
	QVariant extra;

	TplModelRoot &model;
	ServerInterface &srv;

	QVariant old_idx;
	QString old_name;

	bool has_data;
	bool dummy_created;
	QList<TplModelNode*> children;

	QIcon *icon;

	bool file_progress;
	int file_pos, file_total;

private:
	template<typename T> TplModelNode *addChild(T key, QString name, TplModelNode::NodeType childtype, QVariant extra = QVariant());
	template<typename T> TplModelNode *findChild(T key);

	void addChild(TplModelNode *child);
};

#endif // TPLMODELNODE_HPP

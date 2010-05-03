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

#include <QAbstractItemModel>
#include <QIcon>

class ServerInterface;
class MainWindow;
class TplModelNode;

class TplModelRoot: public QAbstractItemModel {
	Q_OBJECT;

public:
	TplModelRoot(QWidget *parent, ServerInterface &_srv);
	virtual ~TplModelRoot();

	virtual Qt::ItemFlags flags(const QModelIndex &index) const;
	virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
	virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
	virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
	virtual bool hasChildren(const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
	virtual QModelIndex parent(const QModelIndex &index) const;
	virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
	virtual QMimeData *mimeData(const QModelIndexList &indexes) const;

	void refresh();

protected slots:
	void updateServersList(int id, QVariant data, QObject *extra);
	void setPagesList(int id, QVariant data, QObject *extra);
	void setTemplatesList(int id, QVariant data, QObject *extra);
	void setFilesList(int id, QVariant data, QObject *extra);

protected:
	friend class TplModelNode; /* berk */

	void loadNodeData(TplModelNode *node);
	QIcon *getNodeIcon(TplModelNode *node);

	TplModelNode *getNode(const QModelIndex &idx) const;

	ServerInterface &srv;
	MainWindow &mw;

	QIcon skin_icon;
	QIcon page_icon;
	QIcon tpl_icon;
	QIcon dir_icon;
	QIcon file_icon;

	TplModelNode *root;
//	QList<int> serv_mapping;
//	QMap<int, TplModelNode*> serv_list;
};


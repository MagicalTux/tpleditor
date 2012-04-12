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

#ifndef PROPERTIESTPL_HPP
#define PROPERTIESTPL_HPP

#include <QObject>
#include <QDialog>

#include "TplModelNode.hpp"
#include "ui_PropertiesTpl.h"

class PropertiesTpl : public QDialog
{
	Q_OBJECT;

public:
	PropertiesTpl(QWidget *parent, TplModelNode *_node);
	Ui::PropertiesTpl ui;

private:
	TplModelNode &node;
	QMap<int, QTreeWidgetItem*> tpltypes;
	QTreeWidgetItem *curtpltype;
	int curtplstuff;

	QString options;

	struct stuff_layout {
		QLayout *layout;
		int refcount;
	};

	struct stuff_desc {
		QObject *obj;
		struct PropertiesTpl::stuff_layout *layout;
		QVariant value;
	};

	QMap<QString, struct PropertiesTpl::stuff_desc*> stuff;

	void refreshTplOptions();
	void cleanupTplOptions();
	void addBaseTplOptions();
	struct PropertiesTpl::stuff_layout *registerLayout(QLayout *layout);
	void appendTplStuff(QString name, struct PropertiesTpl::stuff_layout *l, QObject *obj);

private slots:
	void accept();
	void setTemplateTypeComplete(QVariant data, QObject *);

	void stuffLayerObjectDestroyed(QObject*);

	void on_tree_itemChanged(QTreeWidgetItem * item, int column);

	void getTplTypes(QVariant data, QObject *);
	void gotTplType(QVariant data, QObject *);
	void gotEditableOptions(QVariant data, QObject *);

	void stuff_checkBoxStatusChange(int);
	void stuff_comboBoxStatusChange(int index);
	void stuff_textBoxEditFinished();

	void setOptions(QString);
//	void setPageProperties(int id, QVariant data, QObject *);
};

#endif // PROPERTIESTPL_HPP

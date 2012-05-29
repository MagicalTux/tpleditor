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

#include "PropertiesTpl.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>

PropertiesTpl::PropertiesTpl(QWidget *parent, TplModelNode *_node): QDialog(parent), node(*_node) {
	ui.setupUi(this);
	ui.tree->sortByColumn(0, Qt::AscendingOrder);

	// ask for current page properties

	show();
	setEnabled(false);
	// load current stuff from the node
	node.getAllTplTypes(this, "getTplTypes");
	node.getTplType(this, "gotTplType");
	curtplstuff = -1;
	curtpltype = NULL;
	curtplid = -1;
}

void PropertiesTpl::accept() {
	setEnabled(false);
	int tpltype = 0;
	if (curtpltype != NULL)
		tpltype = curtpltype->data(0, Qt::UserRole).toInt();
	node.setTemplateType(this, "setTemplateTypeComplete", tpltype, options);
	//node.setPageProperties(this, "setPageProperties", ui.page_charset->currentText(), ui.page_content_type->currentText());
}

void PropertiesTpl::setTemplateTypeComplete(QVariant data, QObject *) {
	if (!data.isValid()) {
		QMessageBox::critical(NULL, "Server error", "Failed to set defined options", QMessageBox::Ok);
		setEnabled(true);
		return;
	}
	close();
	deleteLater();
}

void PropertiesTpl::gotTplType(QVariant data, QObject *) {
	int type = data.toMap()["Tpl_Type__"].toInt();

	// set options text
	options = data.toMap()["Options"].toString();
	curtplid = type;

	// if tree is already rendered, try to update right now
	if (tpltypes.contains(type)) {

		curtpltype = tpltypes.value(type);
		curtpltype->setCheckState(0, Qt::Checked); // this will call refreshTplOptions()
		ui.tree->setCurrentItem(curtpltype, 0, QItemSelectionModel::Clear); // display it
	}

	refreshTplOptions();
}

void PropertiesTpl::on_tree_itemChanged(QTreeWidgetItem * item, int) {
	bool state = item->checkState(0);
	if (item == curtpltype) {
		if (!state)
			curtpltype = NULL;
	} else if (state) { // something else has been selected
		if (curtpltype != NULL) { // uncheck our previous selection if any
			curtpltype->setCheckState(0, Qt::Unchecked);
		}
		curtpltype = item;

		if (isEnabled())
			refreshTplOptions();
	}
}

void PropertiesTpl::cleanupTplOptions() {
	QMap<QString, struct stuff_desc*>::iterator i = stuff.begin();
	while(i != stuff.end()) {
		struct stuff_desc *tmp = *i;
		tmp->obj->deleteLater();
		delete tmp;
		stuff.erase(i);
		++i;
	}
}

void PropertiesTpl::addBaseTplOptions() {
	// Build: QHBoxLayout, and inside: Label, and QLineEdit

	if (stuff.contains("_baselabel")) return; // avoid breaking stuff

	QHBoxLayout *layout = new QHBoxLayout();
	layout->setContentsMargins(8, 3, 8, 3); // int left, int top, int right, int bottom
	QLabel *lbl = new QLabel("Options", this);
	QLineEdit *editor = new QLineEdit(this);
	editor->setText(options);

	struct stuff_layout *l = registerLayout(layout);

	appendTplStuff("_baselabel", l, lbl);
	appendTplStuff("_baseeditor", l, editor);

	connect(editor, SIGNAL(textChanged(QString)), this, SLOT(setOptions(QString)));
}

struct PropertiesTpl::stuff_layout *PropertiesTpl::registerLayout(QLayout *layout) {
	struct stuff_layout *tmp = new struct stuff_layout;
	tmp->refcount = 0;
	tmp->layout = layout;

	ui.optionslayout->addLayout(layout);

	return tmp;
}

void PropertiesTpl::refreshTplOptions() {
	if (curtpltype == NULL) {
		setEnabled(true);
		cleanupTplOptions();
		addBaseTplOptions();
		return;
	}

	// let's build a layout with the tpl option
	int tpltype = curtpltype->data(0, Qt::UserRole).toInt();

	node.getEditableOptions(this, "gotEditableOptions", tpltype, options);

	if (tpltype == curtplstuff) return;
	setEnabled(false);

	curtplstuff = tpltype;
}

void PropertiesTpl::stuff_comboBoxStatusChange(int index) {
	if (!isEnabled()) return;
	QString name = sender()->objectName();
	QComboBox *cb = static_cast<QComboBox*>(sender());
	QVariant value = cb->itemData(index, Qt::UserRole);

	if (stuff.value(name)->value == value) return;

	int tpltype = curtpltype->data(0, Qt::UserRole).toInt();
	setEnabled(false);
	node.testEditableOption(this, "gotEditableOptions", tpltype, options, name, value);
}

void PropertiesTpl::stuff_textBoxEditFinished() {
	if (!isEnabled()) return;
	QString name = sender()->objectName();
	QString value = static_cast<QLineEdit*>(sender())->text();

	if (stuff.value(name)->value == value) return;

	int tpltype = curtpltype->data(0, Qt::UserRole).toInt();
	setEnabled(false);
	node.testEditableOption(this, "gotEditableOptions", tpltype, options, name, value);
}

void PropertiesTpl::stuff_checkBoxStatusChange(int status) {
	if (!isEnabled()) return;
	QString name = sender()->objectName();
	int value = status ? 1 : 0;

	if (stuff.value(name)->value == value) return;

	int tpltype = curtpltype->data(0, Qt::UserRole).toInt();
	curtplid = tpltype;
	setEnabled(false);
	node.testEditableOption(this, "gotEditableOptions", tpltype, options, name, value);
}

void PropertiesTpl::gotEditableOptions(QVariant data, QObject *) {
	if (!data.isValid()) {
		setEnabled(true);
		return; // :(
	}

	options = data.toMap()["Options"].toString();
	qDebug("New options string: %s", qPrintable(options));

	if (data.toMap()["editableFields"].toList().size() == 0) {
		cleanupTplOptions();
		addBaseTplOptions();
		setEnabled(true);
		return;
	}

	// ok we got some tpl options...
	QList<QVariant> ef = data.toMap()["editableFields"].toList();
	QMap<QString, bool> ef_idx;
	for(int i = 0; i < ef.size(); i++)
		ef_idx.insert(ef.at(i).toMap()["name"].toString(), true);

	// first, iterate into existing stuff and clear useless things
	QMap<QString, struct stuff_desc*>::iterator i = stuff.begin();
	while(i != stuff.end()) {
		struct stuff_desc *tmp = *i;
		if (ef_idx.contains(i.key())) {
			++i;
			continue;
		}
		tmp->obj->deleteLater();
		++i;
	}

	// now do the other one
	struct stuff_layout *l = NULL;
	for(int i = 0; i < ef.size(); i++) {
		QString name = ef.at(i).toMap()["name"].toString();
		QString type = ef.at(i).toMap()["type"].toString();

		if (stuff.contains(name)) {
			// already got it - update its value to make sure it matches what we want
			if (type == "string") {
				QLineEdit *led = static_cast<QLineEdit*>(stuff.value(name)->obj);
				led->setText(ef.at(i).toMap()["value"].toString());
				stuff.value(name)->value = ef.at(i).toMap()["value"];
			} else if (type == "checkbox") {
				QCheckBox *cb = static_cast<QCheckBox*>(stuff.value(name)->obj);
				bool value = ef.at(i).toMap()["value"].toBool();
				cb->setCheckState(value?Qt::Checked:Qt::Unchecked);
				stuff.value(name)->value = ef.at(i).toMap()["value"];
			} else if (type == "select") {
				QComboBox *cb = static_cast<QComboBox*>(stuff.value(name)->obj);
				// reset combobox options as they may have changed
				cb->clear();
				QList<QVariant> cb_items = ef.at(i).toMap()["params"].toMap()["values"].toList();
				QString value = ef.at(i).toMap()["value"].toString();
				for(int j = 0; j < cb_items.size(); j++) {
					QMap<QString, QVariant> it = cb_items.at(j).toMap();
					cb->addItem(it["value"].toString(), it["key"]);
					if (it["key"].toString() == value)
						cb->setCurrentIndex(j);
				}
				stuff.value(name)->value = ef.at(i).toMap()["value"];
			}
			continue;
		}
		if ((type == "break") && (l == NULL)) continue; // no need for that :)
		if (l == NULL) {
			QHBoxLayout *layout = new QHBoxLayout();
			l = registerLayout(layout);
		}

		if (type == "break") {
			QHBoxLayout *layout = new QHBoxLayout();
			l = registerLayout(layout);
			continue;
		}
		if (type == "label") {
			QLabel *lbl = new QLabel(ef.at(i).toMap()["params"].toMap()["caption"].toString(), this);
			appendTplStuff(name, l, lbl);
			continue;
		}
		if (type == "string") {
			QLineEdit *led = new QLineEdit(this);
			led->setText(ef.at(i).toMap()["value"].toString());
			connect(led, SIGNAL(editingFinished()), this, SLOT(stuff_textBoxEditFinished()));
			appendTplStuff(name, l, led);
			stuff.value(name)->value = ef.at(i).toMap()["value"];
			continue;
		}
		if (type == "checkbox") {
			QCheckBox *cb = new QCheckBox(ef.at(i).toMap()["params"].toMap()["caption"].toString(), this);
			appendTplStuff(name, l, cb);
			stuff.value(name)->value = ef.at(i).toMap()["value"];
			bool value = ef.at(i).toMap()["value"].toBool();
			cb->setCheckState(value?Qt::Checked:Qt::Unchecked);
			connect(cb, SIGNAL(stateChanged(int)), this, SLOT(stuff_checkBoxStatusChange(int)));
			continue;
		}
		if (type == "select") {
			QComboBox *cb = new QComboBox(this);
			QString value = ef.at(i).toMap()["value"].toString();
			QList<QVariant> cb_items = ef.at(i).toMap()["params"].toMap()["values"].toList();
			for(int j = 0; j < cb_items.size(); j++) {
				QMap<QString, QVariant> it = cb_items.at(j).toMap();
				cb->addItem(it["value"].toString(), it["key"]);
				if (it["key"].toString() == value)
					cb->setCurrentIndex(j);
			}
			connect(cb, SIGNAL(currentIndexChanged(int)), this, SLOT(stuff_comboBoxStatusChange(int)));
			appendTplStuff(name, l, cb);
			stuff.value(name)->value = ef.at(i).toMap()["value"];
			continue;
		}

		qDebug("Unhandled type: %s", qPrintable(type));
	}

	// avoid leaking layouts
	if ((l != NULL) && (l->refcount == 0)) {
		l->layout->deleteLater();
		delete l;
	}
	setEnabled(true);
}

void PropertiesTpl::setOptions(QString opt) {
	options = opt;
}

void PropertiesTpl::stuffLayerObjectDestroyed(QObject *obj) {
	// ok, thing was destroyed
	QString name = obj->objectName();
	struct stuff_desc *tmp = stuff.take(name);
	if (tmp == NULL) return;
	struct stuff_layout *layout = tmp->layout;
	layout->refcount--;

	qDebug("object destroyed=%s layout refcount=%d", qPrintable(name), layout->refcount);

	if (layout->refcount == 0) {
		layout->layout->deleteLater();
		delete layout;
	}
}

void PropertiesTpl::appendTplStuff(QString name, struct stuff_layout *layout, QObject *obj) {
	struct stuff_desc *tmp = new struct stuff_desc;
	obj->setObjectName(name);
	tmp->obj = obj;
	tmp->layout = layout;
	layout->refcount++;
	layout->layout->addWidget(static_cast<QWidget*>(obj));
	stuff.insert(name, tmp);
	connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(stuffLayerObjectDestroyed(QObject*)));
}

void PropertiesTpl::getTplTypes(QVariant data, QObject *) {
	if (!data.isValid()) {
		close();
		return;
	}
	// we got two things in data: tree & list
	QList<QVariant> tree = data.toMap()["tree"].toList();
	QList<QVariant> types = data.toMap()["list"].toList();

	QMap<int, QTreeWidgetItem*> dirs;

	for(int i = 0; i < tree.size(); i++) {
		QMap<QString, QVariant> item = tree.at(i).toMap();
		int id = item["Id"].toInt();
		int parent = item["Parent"].toInt();

		if (!dirs.contains(parent)) parent = 0;

		QTreeWidgetItem *tmp;

		if (parent == 0) {
			tmp = new QTreeWidgetItem(ui.tree, QStringList() << item["Name"].toString(), 0);
			ui.tree->addTopLevelItem(tmp);
		} else {
			tmp = new QTreeWidgetItem(dirs.value(parent), QStringList() << item["Name"].toString(), 0);
			dirs.value(parent)->addChild(tmp);
		}

		tmp->setFlags(Qt::ItemIsEnabled);

		dirs.insert(id, tmp);
	}

	for(int i = 0; i < types.size(); i++) {
		QMap<QString, QVariant> item = types.at(i).toMap();
		int id = item["Id"].toInt();
		int parent = item["Parent"].toInt();

		if (!dirs.contains(parent)) parent = 0;

		QTreeWidgetItem *tmp;

		if (parent == 0) {
			tmp = new QTreeWidgetItem(ui.tree, QStringList() << item["Name"].toString(), 0);
			ui.tree->addTopLevelItem(tmp);
		} else {
			tmp = new QTreeWidgetItem(dirs.value(parent), QStringList() << item["Name"].toString(), 0);
			dirs.value(parent)->addChild(tmp);
		}
		tmp->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
		tmp->setCheckState(0, Qt::Unchecked);
		tmp->setData(0, Qt::UserRole, QVariant(id));
		tpltypes.insert(id, tmp);
	}

	if (curtplid >= 0 && tpltypes.contains(curtplid)) {
		curtpltype = tpltypes.value(curtplid);
		curtpltype->setCheckState(0, Qt::Checked); // this will call refreshTplOptions()
		ui.tree->setCurrentItem(curtpltype, 0, QItemSelectionModel::Clear); // display it
		refreshTplOptions();
	}
}

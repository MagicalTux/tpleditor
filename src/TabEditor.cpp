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


#include "TabEditor.hpp"
#include "ServerInterface.hpp"
#include "TplModelNode.hpp"
#include "TplSyntax.hpp"

#include <QHBoxLayout>
#include <QTextEdit>
#include <QIcon>
#include <QVariant>
#include <QEvent>
#include <QKeyEvent>
#include <QTextDocument>
#include <QLabel>
#include <QComboBox>
#include <QDateTime>

TabEditor::TabEditor(QWidget *parent, ServerInterface *_srv, TplModelNode *_node, QSettings &_settings): QWidget(parent), settings(_settings) {
	node = _node;
	srv = _srv;
	changed = false;

	QVariant info = node->getNodeData();
	name = info.toMap()["Skin"].toString()+QString("/")+info.toMap()["Page"].toString()+QString("/")+info.toMap()["Template"].toString()+QString("/")+info.toMap()["Language"].toString();

	setObjectName(name);
	textEdit = new QTextEdit(this);
	textEdit->setAcceptRichText(false);
	textEdit->setReadOnly(true); // avoid edits until it's loaded...
	textEdit->setTabStopWidth(25);
	textEdit->installEventFilter(this);

	layout_tplopt = new QHBoxLayout();
	layout_tplopt->setMargin(1);
	layout_history = new QHBoxLayout();
	layout_history->setMargin(1);
	layout_main = new QVBoxLayout(this);
	layout_main->setMargin(1);
	layout_main->addLayout(layout_tplopt);
	layout_main->addLayout(layout_history);
	layout_main->addWidget(textEdit);

	// tplopt layout
	layout_tplopt->addWidget(btn_savetpl = new QPushButton(tr("Save template")));
	btn_savetpl->setEnabled(false);
	layout_tplopt->addWidget(btn_tplprop = new QPushButton(tr("Template properties")));
	layout_tplopt->addWidget(btn_pageprop = new QPushButton(tr("Page properties")));
	layout_tplopt->addWidget(btn_putinprod = new QPushButton((info.toMap()["Page"].toString() == "__common") ? tr("Compile all pages") : tr("Compile this page")));

	// history layout
	layout_history->addWidget(combo_history = new QComboBox());
	layout_history->addWidget(btn_histo_reload = new QPushButton(tr("Refresh")));
	layout_history->addWidget(btn_histo_restore = new QPushButton(tr("Restore")));
	combo_history->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	if (info.toMap()["Page"].toString() == "__common") {
		btn_putinprod->setIcon(QIcon(":/images/exclamation.png"));
//		btn_putinprod->setStyleSheet("font-weight: bold; border: 1px solid black; background-color: white;");
	}

	connect(btn_savetpl, SIGNAL(clicked()), this, SLOT(doSave()));
	connect(btn_tplprop, SIGNAL(clicked()), this, SLOT(action_TplProperties()));
	connect(btn_pageprop, SIGNAL(clicked()), this, SLOT(action_PageProperties()));
	connect(btn_putinprod, SIGNAL(clicked()), this, SLOT(action_PutInProd()));
	connect(btn_histo_reload, SIGNAL(clicked()), this, SLOT(reloadHistory()));
	connect(btn_histo_restore, SIGNAL(clicked()), this, SLOT(restoreHistoryEntry()));

	settings.beginGroup("Editor");
	QFont myfont("Courier New", 10);
	myfont.fromString(settings.value("Font", myfont.toString()).toString());
	textEdit->setCurrentFont(myfont);
	textEdit->setFont(myfont);
	settings.endGroup();

	connect(textEdit->document(), SIGNAL(modificationChanged(bool)), this, SLOT(tabTextChanged(bool)));

	// Syntax colors
	syncolor = new TplSyntax(textEdit);
	syncolor->setTextFormat("HTML");

	// Refresh syntax color
	syncolor->loadXmlSyntax("syntax.xml");
	srv->sendRequest("Skins.getColorSyntax", QVariant(), syncolor, "setRemoteSyntaxRules", NULL);

	// Icon for tab?
	QIcon icon5;
	icon5.addPixmap(QPixmap(QString::fromUtf8(":/images/notebooks.png")), QIcon::Normal, QIcon::Off);

	srv->sendRequest("Skins.getTemplateContents", info, this, "setTabContents", NULL);
	reloadHistory();
}

void TabEditor::restoreHistoryEntry() {
	int idx = combo_history->currentIndex();
	QVariantMap m = combo_history->itemData(idx).toMap();

	settings.beginGroup("Editor");
	QFont myfont("Courier New", 10);
	myfont.fromString(settings.value("Font", myfont.toString()).toString());
	textEdit->setCurrentFont(myfont);
	textEdit->setFont(myfont);
	settings.endGroup();

	textEdit->setPlainText(m["Content"].toString());
	textEdit->setReadOnly(false);

	changed = (idx != 0);
	btn_savetpl->setEnabled(changed);
	tabStatusChanged();
}

void TabEditor::reloadHistory() {
	// reload tab editor history
	srv->sendRequest("Skins.getTemplateHistory", node->getNodeData(), this, "reloadHistoryResult", NULL);
}

void TabEditor::reloadHistoryResult(QVariant data, QObject *) {
	QVariantList h = data.toMap()["TemplateHistory"].toList();

	combo_history->clear();
	for(int i = 0; i < h.length(); i++) {
		QVariantMap m = h[i].toMap();
		combo_history->addItem(tr("Version as of %1 by %2 %3").arg(QDateTime::fromTime_t(m["Date"].toLongLong()).toString(Qt::DefaultLocaleLongDate)).arg(m["Admin"].toMap()["First_Name"].toString()).arg(m["Admin"].toMap()["Last_Name"].toString()), m);
	}
}

void TabEditor::event_reloadSettings() {
	bool _changed = changed;
	settings.beginGroup("Editor");
	QFont myfont("Courier New", 10);
	myfont.fromString(settings.value("Font", myfont.toString()).toString());
	textEdit->setCurrentFont(myfont);
	textEdit->setFont(myfont);
	QTextCharFormat format;
	format.setFont(myfont);
	QTextCursor selall(textEdit->document());
	selall.select(QTextCursor::Document);
	selall.setCharFormat(format);
	settings.endGroup();
	tabTextChanged(_changed);
}

void TabEditor::action_TplProperties() {
	node->getParentOfType(TplModelNode::TEMPLATE)->editProperties();
}

void TabEditor::action_PageProperties() {
	node->getParentOfType(TplModelNode::PAGE)->editProperties();
}

void TabEditor::action_PutInProd() {
	node->getParentOfType(TplModelNode::PAGE)->putInProd();
}

bool TabEditor::eventFilter(QObject *obj, QEvent *event) {
	if (event->type() != QEvent::KeyPress) {
		return QWidget::eventFilter(obj, event);
	}

	QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

	if ( (keyEvent->key() == Qt::Key_Enter) || (keyEvent->key() == Qt::Key_Return) ) {
		// Ok, we are about to do something bad

		QTextCursor cursor = textEdit->textCursor();

		QString curline = cursor.block().text();
		int tabs = 0;
		while((tabs < curline.size()) && (curline.at(tabs) == '\t')) tabs++;

		cursor.insertText(QString("\n") + QString(tabs, '\t'));

		return true;
	}

	if (keyEvent->key() == Qt::Key_Backtab) {
		// Do we have a selection?
		if ( (!textEdit->textCursor().hasSelection()) || (textEdit->textCursor().hasComplexSelection())) {
			// Nope? Then let this event flow
			return QWidget::eventFilter(obj, event);
		}

		// selection extend
		QTextCursor cursor = textEdit->textCursor();
		int sel_start = cursor.selectionStart();
		int sel_end = cursor.selectionEnd();

		// Get the first block
		QTextBlock i = textEdit->document()->findBlock(sel_start);
		QTextBlock i_end = textEdit->document()->findBlock(sel_end).next();

		// here, we do some magic
		cursor.beginEditBlock();
		for(; i != i_end; i = i.next()) {
			if ((i.text().length() == 0) || (i.text().at(0) != '\t')) continue;
			QTextCursor blockstart(i);
			blockstart.deleteChar();
		}
		cursor.endEditBlock();
		return true;
	}

	if (keyEvent->key() == Qt::Key_Tab) {
		// Do we have a selection?
		if ( (!textEdit->textCursor().hasSelection()) || (textEdit->textCursor().hasComplexSelection())) {
			// Nope? Then let this event flow
			return QWidget::eventFilter(obj, event);
		}

		// selection extend
		QTextCursor cursor = textEdit->textCursor();
		int sel_start = cursor.selectionStart();
		int sel_end = cursor.selectionEnd();

		// Get the first block
		QTextBlock i = textEdit->document()->findBlock(sel_start);
		QTextBlock i_end = textEdit->document()->findBlock(sel_end).next();

		// here, we do some magic
		cursor.beginEditBlock();
		for(; i != i_end; i = i.next()) {
			QTextCursor blockstart(i);
			blockstart.insertText(QString("\t"));
		}
		cursor.endEditBlock();
		return true;
	}
	return QWidget::eventFilter(obj, event);
}

QIcon TabEditor::getTabIcon() {
	return QIcon(":/images/notebooks.png"); // TODO: use server's
}

QString TabEditor::getTabName() {
	if (changed) return QString("* ")+name;
	return name;
}

QString TabEditor::getKey() {
	return name;
}

void TabEditor::setTabContents(QVariant data, QObject *) {
	settings.beginGroup("Editor");
	QFont myfont("Courier New", 10);
	myfont.fromString(settings.value("Font", myfont.toString()).toString());
	textEdit->setCurrentFont(myfont);
	textEdit->setFont(myfont);
	settings.endGroup();

	textEdit->setPlainText(data.toMap()["TemplateData"].toString());
	textEdit->setReadOnly(false);

	changed = false;
	btn_savetpl->setEnabled(changed);
	tabStatusChanged();
}

void TabEditor::doSave() {
	QMap<QString, QVariant> req = node->getNodeData().toMap();
	req["Content"] = textEdit->toPlainText();

	srv->sendRequest("Skins.saveTemplate", req, this, "saveTemplateDone", NULL);
	textEdit->document()->setModified(false);
}

void TabEditor::saveTemplateDone(QVariant data, QObject *) {
	if (data.toMap()["Success"].toBool() != true) {
		textEdit->document()->setModified(true); // could not save?
	}
	btn_savetpl->setEnabled(changed);
	tabStatusChanged();
	reloadHistory();
}

void TabEditor::tabTextChanged(bool _changed) {
	changed = _changed;
	btn_savetpl->setEnabled(changed);
	tabStatusChanged();
}

void TabEditor::updateSyntax() {
	// Refresh syntax highlighting
	syncolor->loadXmlSyntax("syntax.xml");
	srv->sendRequest("Skins.getColorSyntax", QVariant(), syncolor, "setRemoteSyntaxRules", NULL);
}

bool TabEditor::isModified() {
	return changed;
}

TabEditor::~TabEditor() {
	// TODO Auto-generated destructor stub
}

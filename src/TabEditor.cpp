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
#include <QLineEdit>
#include <QSpacerItem>

#include <Qsci/qscilexercss.h>
#include <Qsci/qscilexerhtml.h>
#include <Qsci/qscilexerjavascript.h>

TabEditor::TabEditor(QWidget *parent, ServerInterface *_srv, TplModelNode *_node, QSettings &_settings): QWidget(parent), settings(_settings) {
	node = _node;
	srv = _srv;
	changed = false;

	QVariant info = node->getNodeData();
	name = info.toMap()["Skin"].toString()+QString("/")+info.toMap()["Page"].toString()+QString("/")+info.toMap()["Template"].toString()+QString("/")+info.toMap()["Language"].toString();

	setObjectName(name);
	textEdit = new QsciScintilla(this);
	textEdit->setReadOnly(true);
	textEdit->setTabWidth(4);
	textEdit->setAutoIndent(true);
	textEdit->setMarginLineNumbers(QsciScintilla::NumberMargin, true);
	textEdit->setMarginWidth(QsciScintilla::NumberMargin, "100000");
	textEdit->setUtf8(true);

	QString ext = info.toMap()["Page"].toString().split(".").last();
	if (ext == "js") {
		textLexer = new QsciLexerJavaScript(textEdit);
		tabIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/text-x-javascript.png")), QIcon::Normal, QIcon::Off);
	} else if (ext == "css") {
		textLexer = new QsciLexerCSS(textEdit);
		tabIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/text-css.png")), QIcon::Normal, QIcon::Off);
	} else {
		QsciLexerHTML *htmlLexer = new QsciLexerHTML(textEdit);
		htmlLexer->setDjangoTemplates(true);
		textLexer = (QsciLexer*)htmlLexer;
		tabIcon.addPixmap(QPixmap(QString::fromUtf8(":/images/text-xml.png")), QIcon::Normal, QIcon::Off);
	}
	textEdit->setLexer(textLexer);
	textEdit->setBraceMatching(QsciScintilla::SloppyBraceMatch);
	textEdit->setCaretLineVisible(true);
	textEdit->setCaretLineBackgroundColor(QColor(0xf5, 0xf5, 0xf5));
	textEdit->setMatchedBraceForegroundColor(QColor(0xff, 0x00, 0x00));
	textEdit->setMatchedBraceBackgroundColor(QColor(0xff, 0xfa, 0xfa));

	layout_tplopt = new QHBoxLayout();
	layout_tplopt->setMargin(1);
	layout_history = new QHBoxLayout();
	layout_history->setMargin(1);
	layout_find = new QHBoxLayout();
	layout_find->setMargin(1);
	layout_main = new QVBoxLayout(this);
	layout_main->setMargin(1);
	layout_main->addLayout(layout_tplopt);
	layout_main->addLayout(layout_history);
	layout_main->addWidget(textEdit);
	layout_main->addWidget(find_container = new QWidget());

	// find layout
	find_container->setLayout(layout_find);
	find_container->hide();
	layout_find->addWidget(btn_find_close = new QPushButton(QIcon(":/images/close.png"), ""));
	layout_find->addWidget(find_label = new QLabel("Find: "));
	find_edit = new QLineEdit();
	find_edit->setMinimumWidth(350);
	layout_find->addWidget(find_edit);
	layout_find->addWidget(btn_find_next = new QPushButton("Next"));
	layout_find->addWidget(btn_find_previous = new QPushButton("Previous"));
	layout_find->addStretch(1);

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
	connect(find_edit, SIGNAL(textEdited(QString)), this, SLOT(findChanged(QString)));
	connect(btn_find_next, SIGNAL(clicked()), this, SLOT(findNext()));
	connect(btn_find_previous, SIGNAL(clicked()), this, SLOT(findPrevious()));
	connect(btn_find_close, SIGNAL(clicked()), this, SLOT(toggleFind()));

	settings.beginGroup("Editor");
	QFont myfont("Courier New", 10);
	myfont.fromString(settings.value("Font", myfont.toString()).toString());
	textEdit->setFont(myfont);
	textLexer->setDefaultFont(myfont);
	settings.endGroup();

	connect(textEdit, SIGNAL(modificationChanged(bool)), this, SLOT(tabTextChanged(bool)));

	srv->sendRequest("Skins.getTemplateContents", info, this, "setTabContents", NULL);
	reloadHistory();
}

void TabEditor::toggleFind() {
	if (find_container->isHidden()) {
		find_container->show();
		find_edit->setFocus();
	}
	else {
		find_container->hide();
		textEdit->setFocus();
	}
}

void TabEditor::findChanged(const QString &text) {
	textEdit->findFirst(text, false, false, false, true, true, 0, 0);
}

void TabEditor::findNext() {
	textEdit->findNext();
}

void TabEditor::findPrevious() {
	textEdit->findPrevious();
}

void TabEditor::restoreHistoryEntry() {
	int idx = combo_history->currentIndex();
	QVariantMap m = combo_history->itemData(idx).toMap();

	settings.beginGroup("Editor");
	QFont myfont("Courier New", 10);
	myfont.fromString(settings.value("Font", myfont.toString()).toString());
	textEdit->setFont(myfont);
	textLexer->setDefaultFont(myfont);
	settings.endGroup();

	textEdit->setText(m["Content"].toString());
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
	textEdit->setFont(myfont);
	textLexer->setDefaultFont(myfont);
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

QIcon TabEditor::getTabIcon() {
	//return QIcon(":/images/notebooks.png"); // TODO: use server's
	return tabIcon;
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
	//textEdit->setCurrentFont(myfont);
	textEdit->setFont(myfont);
	textLexer->setDefaultFont(myfont);
	settings.endGroup();

	textEdit->setText(data.toMap()["TemplateData"].toString());
	textEdit->setReadOnly(false);

	changed = false;
	btn_savetpl->setEnabled(changed);
	tabStatusChanged();
}

void TabEditor::doSave() {
	QMap<QString, QVariant> req = node->getNodeData().toMap();
	req["Content"] = textEdit->text();

	srv->sendRequest("Skins.saveTemplate", req, this, "saveTemplateDone", NULL);
	textEdit->setModified(false);
}

void TabEditor::saveTemplateDone(QVariant data, QObject *) {
	if (data.toMap()["Success"].toBool() != true) {
		textEdit->setModified(true); // could not save?
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

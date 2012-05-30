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

#ifndef TABEDITOR_HPP_
#define TABEDITOR_HPP_

#include <QWidget>
#include <QVariant>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>

#include <Qsci/qsciscintilla.h>

class QVBoxLayout;
class QHBoxLayout;
class QTextEdit;
class ServerInterface;
class TplModelNode;
class QComboBox;

class TabEditor: public QWidget {
	Q_OBJECT

public:
	TabEditor(QWidget *parent, ServerInterface *_srv, TplModelNode *_node, QSettings &_settings);
	virtual ~TabEditor();

	QIcon getTabIcon();
	QString getTabName();
	QString getKey();

	void updateSyntax();
	bool isModified();

protected:

	//QWidget *tab;
	QVBoxLayout *layout_main;
	QsciScintilla *textEdit;
	QsciLexer *textLexer;
	QString name;
	TplModelNode *node;
	QSettings &settings;
	QIcon tabIcon;

	// find
	QWidget *find_container;
	QHBoxLayout *layout_find;
	QPushButton *btn_find_close;
	QLabel *find_label;
	QLineEdit *find_edit;
	QPushButton *btn_find_next;
	QPushButton *btn_find_previous;
	QCheckBox *chk_find_regex;

	// tpl opt
	QHBoxLayout *layout_tplopt;
	QPushButton *btn_savetpl;
	QPushButton *btn_tplprop;
	QPushButton *btn_pageprop;
	QPushButton *btn_putinprod;

	QHBoxLayout *layout_history;
	QComboBox *combo_history;
	QPushButton *btn_histo_reload;
	QPushButton *btn_histo_restore;

	ServerInterface *srv;

	bool changed;

public slots:
	void tabTextChanged(bool _changed);
	void setTabContents(QVariant data, QObject *extra);
	void saveTemplateDone(QVariant data, QObject *extra);
	void action_TplProperties();
	void action_PageProperties();
	void action_PutInProd();
	void doSave();
	void event_reloadSettings();
	void reloadHistory();
	void reloadHistoryResult(QVariant data, QObject *extra);
	void restoreHistoryEntry();
	void findChanged(const QString &text);
	void toggleFind();
	void findNext();
	void findPrevious();

Q_SIGNALS:
	void tabStatusChanged();
};

#endif /* TABEDITOR_HPP_ */

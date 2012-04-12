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

#include "TplModelDelegate.hpp"
#include "TplModelNode.hpp"

#include <QLineEdit>
#include <QEvent>
#include <QComboBox>
#include <QApplication>

TplModelDelegate::TplModelDelegate(QObject *parent): QItemDelegate(parent) {
	// nothing
}

QWidget *TplModelDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &index) const {
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());

	if (node->getType() == TplModelNode::LANGUAGE) {
		TplModelNode *skin = node;
		while(skin->getType() != TplModelNode::SKIN) skin = skin->getParent();

		QList<QString> lngs = skin->getExtra().toMap()["Languages"].toStringList();

		QComboBox *editor = new QComboBox(parent);
		for(QList<QString>::iterator i = lngs.begin(); i!=lngs.end(); ++i) {
			editor->addItem(*i);
		}
		return editor;
	} else {
		QLineEdit *editor = new QLineEdit(parent);
		node->setEditor(editor); // the node will connect some signals from the editor
		return editor;
	}
}

void TplModelDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const {
	QString value = index.model()->data(index, Qt::EditRole).toString();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());

	if (node->getType() == TplModelNode::LANGUAGE) {
		QComboBox *combo = qobject_cast<QComboBox*>(editor);
		int idx = -1;
		int combosize = combo->count();
		for(int i = 0; i < combosize; i++) {
			if (combo->itemText(i) == value) idx = i;
		}
		combo->setCurrentIndex(idx);
	} else {
		QLineEdit *lineedit = qobject_cast<QLineEdit*>(editor);
		lineedit->setText(value);
	}
}

void TplModelDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const {
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());

	if (node->getType() == TplModelNode::LANGUAGE) {
		QComboBox *combo = qobject_cast<QComboBox*>(editor);
		model->setData(index, combo->itemText(combo->currentIndex()), Qt::EditRole);
	} else {
		QLineEdit *lineedit = qobject_cast<QLineEdit*>(editor);
		model->setData(index, lineedit->text(), Qt::EditRole);
	}
}

void TplModelDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const {
	editor->setGeometry(option.rect);
}

void TplModelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) {
		QItemDelegate::paint(painter, option, index);
		return;
	}

	if (node->hasProgress()) {
		QStyleOptionProgressBar progressBarOption;
		progressBarOption.rect = option.rect;
		progressBarOption.minimum = 0;
		progressBarOption.maximum = node->getProgressMax();
		progressBarOption.progress = node->getProgress();
		progressBarOption.text = node->getDisplayName();
		progressBarOption.textVisible = true;

		QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);

		return;
	}

	QItemDelegate::paint(painter, option, index);
}

QSize TplModelDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index ) const {
	return QItemDelegate::sizeHint(option, index);
}

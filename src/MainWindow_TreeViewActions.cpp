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

#include <QMessageBox>
#include <QFileDialog>
#include <QDateTime>

#include "MainWindow.hpp"
#include "TabEditor.hpp"
#include "TplModelNode.hpp"
#include "TplModelFilter.hpp"

void MainWindow::on_mainTreeView_doubleClicked(const QModelIndex &index) {
	// Can't rely on what this "index" is, but we can get infos thanks to Qt :)
	TplModelNode *node = qobject_cast<TplModelNode*>(index.model()->data(index, Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	// support for double clicking on template nodes
	if (node->getType() == TplModelNode::TEMPLATE) {
		if (node->hasChildren() && node->size() == 1) {
			node = node->getChild(0);
		}
	}

	if (node->getType() != TplModelNode::LANGUAGE && node->getType() != TplModelNode::FILE) return; // we can only open a "language" for editing

	QVariant info = node->getNodeData();

	QString tabName;

	if (node->getType() == TplModelNode::FILE) {
		// check extension
		QString ext = node->getDisplayName().split(".").last();
		if (ext != "css" && ext != "less" && ext != "js") return;
		// create tabName
		tabName = info.toMap()["Skin"].toString()+info.toMap()["Folder"].toString()+QString("/")+info.toMap()["File"].toString();
	} else {
		tabName = info.toMap()["Skin"].toString()+QString("/")+info.toMap()["Page"].toString()+QString("/")+info.toMap()["Template"].toString()+QString("/")+info.toMap()["Language"].toString();
	}

	if (tabs.contains(tabName)) {
		// Locate this tab and show it
		ui.tabWidget->setCurrentIndex(ui.tabWidget->indexOf(tabs.value(tabName)));
		return;
	}

	TabEditor *editor = new TabEditor(this, srv, node, settings);
	connect(this, SIGNAL(reloadSettings()), editor, SLOT(event_reloadSettings()));

	connect(editor, SIGNAL(tabStatusChanged()), this, SLOT(subTabStatusChanged()));

	tabs.insert(editor->getKey(), editor);
	int tabid = ui.tabWidget->addTab(
		editor,
		editor->getTabIcon(),
		editor->getTabName()
	);
	ui.tabWidget->setTabToolTip(tabid, editor->getTabName());
	ui.tabWidget->setCurrentIndex(tabid);
}

void MainWindow::on_mainTreeView_customContextMenuRequested(const QPoint &pos) {
	// Get current noe in treeview
	TplModelNode *node = qobject_cast<TplModelNode*>(ui.mainTreeView->currentIndex().data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;
	QMenu menu;

	if (node->getType() == TplModelNode::FILE) {
		menu.addAction(ui.action_RenameFile);
		menu.addAction(ui.action_DeleteFile);
		menu.exec(ui.mainTreeView->viewport()->mapToGlobal(pos));
		return;
	}

	if (node->getType() == TplModelNode::FOLDER) {
		menu.addAction(ui.action_CreateDirectory);
		menu.addAction(ui.action_RenameDirectory);
		menu.addAction(ui.action_DeleteDirectory);
		menu.exec(ui.mainTreeView->viewport()->mapToGlobal(pos));
		return;
	}

	if (node->getType() >= TplModelNode::SKIN) menu.addAction(ui.action_NewPage);
	if (node->getType() == TplModelNode::SKIN) {
		menu.addAction(ui.action_CreateDirectory);
		menu.addAction(ui.action_SaveServer);
		menu.addAction(ui.action_RestoreServer);
		menu.addAction(ui.action_MergeServer);
	}
	if (node->getType() == TplModelNode::PAGE) {
		menu.addAction(ui.action_RenamePage);
		menu.addAction(ui.action_DeletePage);
		menu.addAction(ui.action_PageProperties);
		menu.addAction(ui.action_CompilePage);
	}
	if (node->getType() >= TplModelNode::PAGE) menu.addAction(ui.action_NewTemplate);
	if (node->getType() == TplModelNode::TEMPLATE) {
		menu.addAction(ui.action_RenameTemplate);
		menu.addAction(ui.action_DeleteTemplate);
		menu.addAction(ui.action_TplProperties);
	}
	if (node->getType() >= TplModelNode::TEMPLATE) menu.addAction(ui.action_NewLanguage);
	if (node->getType() == TplModelNode::LANGUAGE) menu.addAction(ui.action_DeleteLanguage);

	menu.exec(ui.mainTreeView->viewport()->mapToGlobal(pos));
}

void MainWindow::on_action_NewPage_triggered() {
	TplModelNode *node = qobject_cast<TplModelNode*>(ui.mainTreeView->currentIndex().data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	while ((node != NULL) && (node->getType() != TplModelNode::SKIN)) node = node->getParent();
	if (node == NULL) return;

	node = node->createTemporaryChild(TplModelNode::PAGE); // create a temporary child, and returns it
	if (node == NULL) return;

	QModelIndex index = node->getIndex();//model_proxy->mapFromSource(node->getIndex());

	ui.mainTreeView->scrollTo(index);
	ui.mainTreeView->edit(index);
}

//************ PAGE ACTIONS

void MainWindow::on_action_RenamePage_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node != NULL) node = node->getParentOfType(TplModelNode::PAGE);
	if (node == NULL) return;

	ui.mainTreeView->scrollTo(node->getIndex());
	ui.mainTreeView->edit(node->getIndex());
}

void MainWindow::on_action_DeletePage_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node != NULL) node = node->getParentOfType(TplModelNode::PAGE);
	if (node == NULL) return;

	if (QMessageBox::warning(this, "Please confirm", "Do you really want to delete this page?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		node->serverDelete();
	}
}

void MainWindow::on_action_PageProperties_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node != NULL) node = node->getParentOfType(TplModelNode::PAGE);
	if (node == NULL) return;

	if (node->getType() != TplModelNode::PAGE) return;

	node->editProperties();
}

void MainWindow::on_action_CompilePage_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node != NULL) node = node->getParentOfType(TplModelNode::PAGE);
	if (node == NULL) return;

	node->putInProd();
}

void MainWindow::on_mainTreeView_selectionChanged(const QItemSelection &selected, const QItemSelection &) {
	QModelIndex index = selected.indexes().first();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	bool has_server = true;
	bool has_page = true;
	bool has_template = true;
	bool has_language = true;
	bool is_folder = true;
	bool is_file = true;

	qDebug("type=%d", node->getType());

	if (node->getParentOfType(TplModelNode::SKIN) == NULL) has_server = false;
	if (node->getParentOfType(TplModelNode::PAGE) == NULL) has_page = false;
	if (node->getParentOfType(TplModelNode::TEMPLATE) == NULL) has_template = false;
	if (node->getParentOfType(TplModelNode::LANGUAGE) == NULL) has_language = false;
	if (node->getType() != TplModelNode::FOLDER) is_folder = false;
	if (node->getType() != TplModelNode::FILE) is_file = false;

	ui.action_NewPage->setEnabled(has_server);
	ui.action_RenamePage->setEnabled(has_page);
	ui.action_DeletePage->setEnabled(has_page);
	ui.action_PageProperties->setEnabled(has_page);
	ui.action_CompilePage->setEnabled(has_page);

	ui.action_RenameFile->setEnabled(is_file);
	ui.action_DeleteFile->setEnabled(is_file);

	ui.action_CreateDirectory->setEnabled(has_server && !is_file);
	ui.action_RenameDirectory->setEnabled(is_folder);
	ui.action_DeleteDirectory->setEnabled(is_folder);

	ui.action_NewTemplate->setEnabled(has_page);
	ui.action_RenameTemplate->setEnabled(has_template);
	ui.action_DeleteTemplate->setEnabled(has_template);
	ui.action_TplProperties->setEnabled(has_template);

	ui.action_NewLanguage->setEnabled(has_template);
	ui.action_DeleteLanguage->setEnabled(has_language);
	ui.action_ChangeLanguage->setEnabled(has_language);

	if (!has_page) {
		ui.action_CompilePage->setText(tr("Compile page"));
		ui.action_CompilePage->setIcon(QIcon());
		return;
	} else {
		ui.action_CompilePage->setEnabled(true);
		if (node->getParentOfType(TplModelNode::PAGE)->getKey() == "__common") {
			ui.action_CompilePage->setIcon(QIcon(":/images/exclamation.png"));
			ui.action_CompilePage->setText(tr("Compile all pages"));
		} else {
			ui.action_CompilePage->setIcon(QIcon());
			ui.action_CompilePage->setText(tr("Compile page"));
		}
	}
}

//************ TEMPLATE ACTIONS

void MainWindow::on_action_NewTemplate_triggered() {
	TplModelNode *node = qobject_cast<TplModelNode*>(ui.mainTreeView->currentIndex().data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	while ((node != NULL) && (node->getType() != TplModelNode::PAGE)) node = node->getParent();
	if (node == NULL) return;

	node = node->createTemporaryChild(TplModelNode::TEMPLATE); // create a temporary child, and returns it
	if (node == NULL) return;

	QModelIndex index = node->getIndex();//model_proxy->mapFromSource(node->getIndex());

	ui.mainTreeView->scrollTo(index);
	ui.mainTreeView->edit(index);
}

void MainWindow::on_action_RenameTemplate_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::TEMPLATE) return;

	ui.mainTreeView->scrollTo(index);
	ui.mainTreeView->edit(index);
}

void MainWindow::on_action_DeleteTemplate_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::TEMPLATE) return;

	if (QMessageBox::warning(this, "Please confirm", "Do you really want to delete this template?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		node->serverDelete();
	}
}

void MainWindow::on_action_TplProperties_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::TEMPLATE) return;

	node->editProperties();
}

//************ LANGUAGE ACTIONS

void MainWindow::on_action_NewLanguage_triggered() {
	TplModelNode *node = qobject_cast<TplModelNode*>(ui.mainTreeView->currentIndex().data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	while ((node != NULL) && (node->getType() != TplModelNode::TEMPLATE)) node = node->getParent();
	if (node == NULL) return;

	node = node->createTemporaryChild(TplModelNode::LANGUAGE); // create a temporary child, and returns it
	if (node == NULL) return;

	QModelIndex index = node->getIndex();//model_proxy->mapFromSource(node->getIndex());

	ui.mainTreeView->scrollTo(index);
	ui.mainTreeView->edit(index);
}

void MainWindow::on_action_ChangeLanguage_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::LANGUAGE) return;

	ui.mainTreeView->scrollTo(index);
	ui.mainTreeView->edit(index);
}

void MainWindow::on_action_DeleteLanguage_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::LANGUAGE) return;

	if (QMessageBox::warning(this, "Please confirm", "Do you really want to delete this language?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		node->serverDelete();
	}
}


//************ DIRECTORY ACTIONS

void MainWindow::on_action_CreateDirectory_triggered() {
	TplModelNode *node = qobject_cast<TplModelNode*>(ui.mainTreeView->currentIndex().data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	while ((node != NULL) && (node->getType() != TplModelNode::FOLDER) && (node->getType() != TplModelNode::SKIN)) node = node->getParent();
	if (node == NULL) return;

	node = node->createTemporaryChild(TplModelNode::FOLDER); // create a temporary child, and returns it
	if (node == NULL) return;

	QModelIndex index = node->getIndex(); //model_proxy->mapFromSource(node->getIndex());

	ui.mainTreeView->scrollTo(index);
	ui.mainTreeView->edit(index);
}

void MainWindow::on_action_RenameDirectory_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::FOLDER) return;

	ui.mainTreeView->scrollTo(index);
	ui.mainTreeView->edit(index);
}

void MainWindow::on_action_DeleteDirectory_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::FOLDER) return;

	if (QMessageBox::warning(this, "Please confirm", "Do you really want to delete this directory?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		node->serverDelete();
	}
}

//************ FILES ACTIONS

void MainWindow::on_action_RenameFile_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::FILE) return;

	ui.mainTreeView->scrollTo(index);
	ui.mainTreeView->edit(index);
}

void MainWindow::on_action_DeleteFile_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::FILE) return;

	if (QMessageBox::warning(this, "Please confirm", "Do you really want to delete this file?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
		node->serverDelete();
	}
}

//**************** SKINS ACTIONS

void MainWindow::on_action_SaveServer_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::SKIN) return;

	// compute default filename
	QString default_name = node->getData(Qt::EditRole).toString() + "_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hhmmss")+".tplbk";

	QString filename = QFileDialog::getSaveFileName(this, tr("Save server as..."), default_name, tr("Template backup files (*.tplbk)"));
	if (filename.isEmpty())
		return;

	node->saveToFile(filename);
}

void MainWindow::on_action_RestoreServer_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::SKIN) return;

	QString filename = QFileDialog::getOpenFileName(this, tr("Load server from..."), QString(), tr("Template backup files (*.tplbk)"));
	if (filename.isEmpty())
		return;

	if (QMessageBox::question(this, tr("Server restauration"), tr("Are you sure you want to overwrite server \"%1\" with contents of file \"%2\"?").arg(node->getData(Qt::EditRole).toString()).arg(filename), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
		return;

	node->restoreFromFile(filename);
}

void MainWindow::on_action_MergeServer_triggered() {
	QModelIndex index = ui.mainTreeView->currentIndex();
	TplModelNode *node = qobject_cast<TplModelNode*>(index.data(Qt::UserRole).value<QObject*>());
	if (node == NULL) return;

	if (node->getType() != TplModelNode::SKIN) return;

	QString filename = QFileDialog::getOpenFileName(this, tr("Load merge server from..."), QString(), tr("Template backup files (*.tplbk)"));
	if (filename.isEmpty())
		return;

	if (QMessageBox::question(this, tr("Server restauration"), tr("Are you sure you want to overwrite components of server \"%1\" (merge mode) with contents of file \"%2\"?").arg(node->getData(Qt::EditRole).toString()).arg(filename), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
		return;

	node->restoreFromFile(filename, true);
}

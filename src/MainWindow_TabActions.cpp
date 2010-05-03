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

#include "MainWindow.hpp"
#include "TabEditor.hpp"
#include "TplModelNode.hpp"
#include "TplModelFilter.hpp"

void MainWindow::on_actionUpdate_syntax_coloration_triggered() {
	TabEditor *editor = qobject_cast<TabEditor*>(ui.tabWidget->currentWidget());
	if (editor != NULL) editor->updateSyntax();
}

void MainWindow::on_action_Save_triggered() {
	TabEditor *editor = qobject_cast<TabEditor*>(ui.tabWidget->currentWidget());
	if (editor != NULL) editor->doSave();
}

void MainWindow::closeTab(int id) {
	if (id == -1) return;

	QString text = ui.tabWidget->tabText(id);
	TabEditor *editor = qobject_cast<TabEditor*>(ui.tabWidget->widget(id));

	if (editor == NULL) {
		ui.tabWidget->removeTab(id);
		return;
	}

	if (editor->isModified()) {
		int res = QMessageBox::warning(this,
				"Document not saved",
				"This document hasn't been saved, do you want to save it now?",
				QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
				QMessageBox::Yes);
		if (res == QMessageBox::Cancel) {
			return;
		}
		if (res == QMessageBox::Yes) {
			editor->doSave();
		}
	}

	ui.tabWidget->removeTab(id);
	tabs.remove(editor->getKey());
	delete editor;
}

/**
 * This slot receives a call when one of our tabs want to change its status in the tabWidget (tab name, etc)
 */
void MainWindow::subTabStatusChanged() {
	TabEditor *editor = qobject_cast<TabEditor *>(sender());
	if (editor == NULL) return;

	int tabid = ui.tabWidget->indexOf(editor);
	if (tabid == -1) return; // boo
	ui.tabWidget->setTabText(tabid, editor->getTabName());
	ui.tabWidget->setTabIcon(tabid, editor->getTabIcon());
}

void MainWindow::on_action_Close_triggered() {
	closeTab(ui.tabWidget->currentIndex());
}

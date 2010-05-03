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

#include "PreferencesDialog.hpp"
#include <QFont>

PreferencesDialog::PreferencesDialog(QSettings &_settings): settings(_settings) {
	ui.setupUi(this);



	settings.beginGroup("Editor");
	QFont font;
	font.fromString(settings.value("Font", QFont("Courier New", 10).toString()).toString());
	ui.font->setCurrentFont(QFont(font.family()));
	ui.size->setValue(font.pointSize());
	settings.endGroup();

	updateFontStyle();

	connect(ui.font, SIGNAL(currentFontChanged(QFont)), this, SLOT(updateFontStyle()));
	connect(ui.size, SIGNAL(valueChanged(int)), this, SLOT(updateFontStyle()));

	show();
}

void PreferencesDialog::close() {
	hide();
	deleteLater();
}

void PreferencesDialog::updateFontStyle() {
	QFont font = ui.font->currentFont();
	font.setPointSize(ui.size->value());

	ui.label_sample->setFont(font);
}

void PreferencesDialog::accept() {
	QFont font = ui.font->currentFont();
	font.setPointSize(ui.size->value());

	settings.beginGroup("Editor");
	settings.setValue("Font", font.toString());
	settings.endGroup();

	// set font to all TabEditors
	reloadSettings();
	close();
}


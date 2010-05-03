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

#include "TplSyntax.hpp"

#include <QFile>
#include <QDomDocument>

#include <QTextStream>

TplSyntax::TplSyntax(QTextEdit *parent): QSyntaxHighlighter(parent) {
	// Load definition file from :/defaults/syntax.xml
	if (!QFile::exists("syntax.xml")) {
		QFile syntax(":/defaults/syntax.xml");
		syntax.copy("syntax.xml");
	}
	loadXmlSyntax("syntax.xml");
}

void TplSyntax::setTextFormat(const QString &fmt) {
	hl_format = QString(fmt);
	rehighlight();
}

void TplSyntax::loadXmlStyles(QDomElement &node) {
	QDomNodeList list = node.childNodes();

	// cleanup current styles coming from file...
	for(QMap<QString, struct syntax_style>::iterator i = syn_styles.begin(); i != syn_styles.end(); ++i) {
		while ((i.value().origin == syntax_style::FILE) && (i != syn_styles.end())) {
			i = syn_styles.erase(i);
		}
	}

	for(int i = 0; i < list.count(); i++) {
		QDomElement style = list.item(i).toElement();
		if (style.isNull()) continue; // skip invalid
		if (style.tagName() != "style") continue; // wtf?

		QString stylename = style.attribute("name");
		if (syn_styles.contains(stylename)) {
			// check origin, if already from file, skip it (a style shouldn't be defined more than once anyway
			if (syn_styles.value(stylename).origin == syntax_style::FILE) {
				qDebug("Style defined more than once in XML file: %s", qPrintable(stylename));
				continue;
			}
			// overload stuff from server!
			syn_styles.remove(stylename);
		}

		QTextCharFormat fmt;

		if (style.hasAttribute("color")) fmt.setForeground(QColor(style.attribute("color")));
		if (style.hasAttribute("bold")) fmt.setFontWeight(QFont::Bold);
		if (style.hasAttribute("italic")) fmt.setFontItalic(true);

		struct syntax_style fmt_style;
		fmt_style.origin = syntax_style::FILE;
		fmt_style.style = fmt;

		syn_styles.insert(stylename, fmt_style);
	}
}

void TplSyntax::loadXmlSimple(QDomElement &node) {
	if (!node.hasAttribute("name")) {
		qDebug("Skipping language without name from syntax file.");
		return;
	}

	QString name = node.attribute("name");

	if (extra_syntax.contains(name)) {
		qDebug("Duplicate language %s skipped", qPrintable(name));
		return;
	}

	QList<struct simple_syntax_rule> ins_rules;

	// Read rules
	QDomNodeList list = node.childNodes();
	for(int i = 0; i < list.count(); i++) {
		QDomElement subnode = list.item(i).toElement();
		if (subnode.tagName() == "rule") {
			QString match = subnode.attribute("match");
			QString style = subnode.attribute("style");

			if (!syn_styles.contains(style)) {
				qDebug("Match '%s' uses non-existing style '%s', skipped.", qPrintable(match), qPrintable(style));
				continue;
			}

			struct simple_syntax_rule tmprule;
			tmprule.regexp = match;
			tmprule.style = style;
			ins_rules.append(tmprule);
		} else {
			qDebug("Unknown simple node element: %s", qPrintable(subnode.tagName()));
		}
	}

	struct syntax_params syn;
	syn.origin = syntax_params::FILE;
	syn.syntax_type = syntax_params::SIMPLE;
	syn.simple_rules = ins_rules;
	extra_syntax.insert(QString(name), syn);
}

void TplSyntax::loadXmlComplex(QDomElement &node) {
	if (!node.hasAttribute("name")) {
		qDebug("Skipping language without name from syntax file.");
		return;
	}

	QString name = node.attribute("name");

	if (extra_syntax.contains(name)) {
		qDebug("Duplicate language %s skipped", qPrintable(name));
		return;
	}

	// ***** Read specal chars
	QMap<char, struct complex_syntax_sp_char> sp_char_list;
	QDomElement xml_sp_char_base = node.firstChildElement("specialChars");
	QDomNodeList xml_sp_char_list = xml_sp_char_base.childNodes();

	for(int i = 0; i < xml_sp_char_list.count(); i++) {
		QDomElement xml_sp_char = xml_sp_char_list.item(i).toElement();
		if (xml_sp_char.isNull()) continue; // skip invalid
		if (xml_sp_char.tagName() != "char") continue;
		if (!xml_sp_char.hasAttribute("value")) continue;
		QString style = xml_sp_char.attribute("style");
		if (!syn_styles.contains(style)) {
			qDebug("Complex char rule for language %s references non-existant style %s", qPrintable(name), qPrintable(style));
			continue;
		}

		struct complex_syntax_sp_char sp_char;

		sp_char.sp_char = xml_sp_char.attribute("value").at(0).toAscii();
		if (xml_sp_char.hasAttribute("endChar")) {
			sp_char.sp_char_end = xml_sp_char.attribute("endChar").at(0).toAscii();
		} else {
			sp_char.sp_char_end = sp_char.sp_char;
		}

		sp_char.style = style;
		sp_char.allow_multiline = xml_sp_char.attribute("allowMultiline", "false") == "true";

		QDomNodeList xml_sp_char_allow_list = xml_sp_char.childNodes();

		for(int j = 0; j < xml_sp_char_allow_list.size(); j++) {
			QDomElement allowedchar = xml_sp_char_allow_list.item(j).toElement();
			if (allowedchar.isNull()) continue;
			if (allowedchar.tagName() != "allowChar") continue;
			if (!allowedchar.hasAttribute("value")) continue;

			sp_char.allowed_chars.insert(allowedchar.attribute("value").at(0).toAscii(), true);
		}

		sp_char_list.insert(sp_char.sp_char, sp_char);
	}

	struct syntax_params syn;
	syn.origin = syntax_params::FILE;
	syn.syntax_type = syntax_params::COMPLEX;
	syn.complex_sp_chars = sp_char_list;
	extra_syntax.insert(QString(name), syn);
}

void TplSyntax::loadXmlSyntax(const QString &file) {
	// Load the XML file via DOM
	QDomDocument doc;
	QFile xmlfile(file);
	if (!xmlfile.open(QIODevice::ReadOnly)) {
		qDebug("Failed to open file %s (error %d)", qPrintable(file), xmlfile.error());
		return;
	}
	doc.setContent(xmlfile.readAll(), false);
	xmlfile.close();

	QDomElement root = doc.documentElement();
	if (root.tagName() != "syntaxDescriptor") {
		qDebug("ERROR: could not parse Xml syntax file %s: bad root tag name '%s'", qPrintable(file), qPrintable(root.tagName()));
		return;
	}

	// Remove all styles from file
	for(QMap<QString, struct syntax_params>::iterator i = extra_syntax.begin(); i != extra_syntax.end(); ++i) {
		while ((i.value().origin == syntax_params::FILE) && (i != extra_syntax.end())) {
			i = extra_syntax.erase(i);
		}
	}

	// Read language styles
	QDomElement styles = root.firstChildElement("styles");
	if (!styles.isNull()) loadXmlStyles(styles);

	QDomElement languages = root.firstChildElement("languages");
	if (!languages.isNull()) {
		QDomNodeList list = languages.childNodes();
		for(int i = 0; i < list.count(); i++) {
			QDomElement lng = list.item(i).toElement();
			if (lng.isNull()) continue;
			if (lng.tagName() != "language") continue; // wtf?

			QString type = lng.attribute("type", "simple");
			if (type == "simple") {
				loadXmlSimple(lng);
				continue;
			}
			if (type == "complex") {
				loadXmlComplex(lng);
				continue;
			}

			qDebug("Unknown language type in XML file: %s", qPrintable(type));
		}
	}

	rehighlight();
}

void TplSyntax::applySimpleHighlight(const QString &text, const QString &pattern, const QTextCharFormat &fmt) {
	QRegExp expr(pattern, Qt::CaseSensitive, QRegExp::RegExp2);
	int index = text.indexOf(expr);

	while(index >= 0) {
		int len = expr.matchedLength();
		setFormat(index, len, fmt);
		index = text.indexOf(expr, index+len);
	}
}

void TplSyntax::manageComplexSyntax(const QString &text, struct syntax_params &syn) {
	int stack = previousBlockState();
	if (stack == -1) stack = 0; // first block?
	char cur = stack & 0xff;

	int length = text.size();
	int startpos = 0;
	char stopchar = 0;

	if (syn.complex_sp_chars.contains(cur)) stopchar = syn.complex_sp_chars.value(cur).sp_char_end;

	for(int i = 0; i < length; i++) {
		char curchar = text.at(i).toAscii();

		if (curchar == stopchar) {
			// Stop current highlight
			if (syn_styles.contains(syn.complex_sp_chars.value(cur).style)) {
				setFormat(startpos, i-startpos+1, syn_styles.value(syn.complex_sp_chars.value(cur).style).style);
			}
			// unstack cur
			stack = stack >> 8;
			cur = stack & 0xff;
			startpos = i+1;
			stopchar = 0;
			if (syn.complex_sp_chars.contains(cur)) stopchar = syn.complex_sp_chars.value(cur).sp_char_end;
			continue;
		}

		if (syn.complex_sp_chars.contains(curchar)) {
			// check if this char is allowed
			if ((cur > 0) && (!syn.complex_sp_chars.value(cur).allowed_chars.contains(curchar))) {
				continue;
			}
			// Start something... so first, stop current stuff...
			if ((cur > 0) && (syn_styles.contains(syn.complex_sp_chars.value(cur).style))) {
				setFormat(startpos, i-startpos, syn_styles.value(syn.complex_sp_chars.value(cur).style).style);
			}
			// ...and stack it
			cur = curchar;
			stack = stack << 8;
			stack = (stack & 0xffffff00) | cur;
			stopchar = syn.complex_sp_chars.value(cur).sp_char_end;
			startpos = i;
		}
	}

	// apply style to EOL
	if ((cur > 0) && (syn_styles.contains(syn.complex_sp_chars.value(cur).style))) {
		setFormat(startpos, length-startpos, syn_styles.value(syn.complex_sp_chars.value(cur).style).style);
	}

	setCurrentBlockState(stack);
}

void TplSyntax::applySyntax(const QString &text, const QString &syntax) {
	if (!extra_syntax.contains(syntax)) return;
	int tmp; // used later

	struct syntax_params syn = extra_syntax.value(syntax);

	switch(syn.syntax_type) {
		case syntax_params::SIMPLE:
			// For each rule...
			tmp = syn.simple_rules.size();
			for(int j = 0; j < tmp; j++) {
				const struct simple_syntax_rule *rule = &syn.simple_rules.at(j);
				if (!syn_styles.contains(rule->style)) continue;
				QTextCharFormat &fmt = syn_styles[rule->style].style;
				applySimpleHighlight(text, rule->regexp, fmt);
			}
			break;
		case syntax_params::COMPLEX:
			manageComplexSyntax(text, syn);
	}
}

void TplSyntax::highlightBlock(const QString &text) {
	// apply filters

	if (!hl_format.isEmpty()) applySyntax(text, hl_format);

	applySyntax(text, "@always");
	applySyntax(text, "@auto"); // Server-provided syntax
}

void TplSyntax::setRemoteSyntaxRules(int id, QVariant data, QObject *) {
	// Read styles
	QMap<QString, QVariant> styles = data.toMap()["styles"].toMap();

	// Remove server-provided styles
	for(QMap<QString, struct syntax_style>::iterator i = syn_styles.begin(); i != syn_styles.end(); ++i) {
		while ((i.value().origin == syntax_style::SERVER) && (i != syn_styles.end())) {
			i = syn_styles.erase(i);
		}
	}

	for(QMap<QString, QVariant>::iterator i = styles.begin(); i != styles.end(); ++i) {
		if (syn_styles.contains(i.key())) continue; // locally overloaded style

		QMap<QString, QVariant> style = i.value().toMap();
		QTextCharFormat fmt;
		if (style["color"].isValid()) fmt.setForeground(QColor(style["color"].toString()));
		if (style["bold"].toBool()) fmt.setFontWeight(QFont::Bold);

		struct syntax_style fmt_style;
		fmt_style.origin = syntax_style::SERVER;
		fmt_style.style = fmt;

		syn_styles.insert(i.key(), fmt_style);
	}

	// rules
	for(QMap<QString, struct syntax_params>::iterator i = extra_syntax.begin(); i != extra_syntax.end(); ++i) {
		while ((i.value().origin == syntax_params::SERVER) && (i != extra_syntax.end())) {
			i = extra_syntax.erase(i);
		}
	}

	QList<QVariant> rules = data.toMap()["rules"].toList();
	int syn_count = rules.size();

	QList<struct simple_syntax_rule> ins_rules;

	for(int i=0; i<syn_count; i++) {
		// read style
		QList<QVariant> rule = rules.at(i).toList();

		if (!syn_styles.contains(rule.at(1).toString())) {
			qDebug("Unknown style for rule %s: %s", qPrintable(rule.at(0).toString()), qPrintable(rule.at(1).toString()));
		}

		struct simple_syntax_rule tmprule;
		tmprule.regexp = rule.at(0).toString();
		tmprule.style = rule.at(1).toString();
		ins_rules.append(tmprule);
	}

	struct syntax_params syn;
	syn.origin = syntax_params::SERVER;
	syn.syntax_type = syntax_params::SIMPLE;
	syn.simple_rules = ins_rules;
	extra_syntax.insert(QString("@auto"), syn);

	rehighlight();
}


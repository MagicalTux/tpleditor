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

#include <QSyntaxHighlighter>

class QDomElement;

class TplSyntax: public QSyntaxHighlighter {
	Q_OBJECT;

public:

	TplSyntax(QTextEdit *parent);
	void setTextFormat(const QString &fmt);
	void loadXmlSyntax(const QString &file);

protected:
	void highlightBlock(const QString &text);

	struct syntax_style;

	void applySyntax(const QString &text, const QString &name);
	void applySimpleHighlight(const QString &text, const QString &pattern, const QTextCharFormat &fmt);

	void loadXmlStyles(QDomElement &node);
	void loadXmlSimple(QDomElement &node);
	void loadXmlComplex(QDomElement &node);

	QString hl_format;

	struct simple_syntax_rule {
		QString style;
		QString regexp;
	};

	struct complex_syntax_sp_char {
		char sp_char;
		char sp_char_end;
		QString style;
		bool allow_multiline;
		QMap<char, bool> allowed_chars;
	};

	struct syntax_params {
		enum { SERVER, FILE } origin;
		enum { SIMPLE, COMPLEX } syntax_type;

		// Simple syntax
		QList<struct simple_syntax_rule> simple_rules;
		// Complex syntax
		QMap<char, struct complex_syntax_sp_char> complex_sp_chars;
	};

	struct syntax_style {
		enum { SERVER, FILE } origin;
		QTextCharFormat style;
	};

	void manageComplexSyntax(const QString &text, struct TplSyntax::syntax_params &syn);

	QMap<QString, struct syntax_style> syn_styles;
	QMap<QString, struct syntax_params> extra_syntax;

protected slots:
	void setRemoteSyntaxRules(int id, QVariant data, QObject *extra);
};


#include <QScriptEngine>
#include <QScriptValue>
#include <QtJson.hpp>

/**
 * JSON engine for Qt
 *
 * For now as I'm lazy, I'll just use QtScript to make stuff work, however
 * I'd like to avoid linking against QtScript. Will need a public domain
 * JSON lib to replace this ugly code (just keep the API, it should be
 * fine).
 */

QByteArray QtJson::encode(const QString &var) {
	// convert a QString to a QByteArray by escaping unicode chars
	QByteArray res;
	res.append('"');

	for(int i = 0; i < var.length(); i++) {
		ushort c = var.at(i).unicode();
		if ((c > 31) && (c < 127)) {
			if ((c == '\\') || (c == '"'))
				res.append('\\');
			res.append(c);
			continue;
		}
		if ((c >= 7) && (c <= 13)) {
			res.append('\\');
			switch(c) {
				case 7: res.append('a'); break;
				case 8: res.append('b'); break;
				case 9: res.append('t'); break;
				case 10: res.append('n'); break;
				case 11: res.append('v'); break;
				case 12: res.append('f'); break;
				case 13: res.append('r'); break;
			}
			continue;
		}
		QByteArray tmp;
		tmp.append((c >> 8) & 0xff);
		tmp.append(c & 0xff);
		res.append('\\');
		res.append('u');
		res.append(tmp.toHex());
	}
	res.append('"');
	return res;
}

QByteArray QtJson::encode(const QByteArray &var) {
	// convert a QByteArray (binary string) by escaping non-ascii chars
	QByteArray res;
	res.append('"');

	for(int i = 0; i < var.length(); i++) {
		char c = var.at(i);
		if ((c > 31) && (c < 127)) {
			if ((c == '\\') || (c == '"'))
				res.append('\\');
			res.append(c);
			continue;
		}
		if ((c >= 7) && (c <= 13)) {
			res.append('\\');
			switch(c) {
				case 7: res.append('a'); break;
				case 8: res.append('b'); break;
				case 9: res.append('t'); break;
				case 10: res.append('n'); break;
				case 11: res.append('v'); break;
				case 12: res.append('f'); break;
				case 13: res.append('r'); break;
			}
			continue;
		}
		// json parsers do not like binary
		return encode(var.toBase64());
		/*
		res.append('\\');
		res.append('x');
		res.append(QByteArray(1, c).toHex()); */
	}
	res.append('"');
	return res;
}

QByteArray QtJson::encode(const QList<QVariant> &var) {
	QByteArray res;

	res.append('[');

	for(int i = 0; i < var.length(); i++) {
		if (i > 0) res.append(',');
		res.append(encode(var.at(i)));
	}
	res.append(']');
	return res;
}

QByteArray QtJson::encode(const QMap<QString,QVariant> &var) {
	QByteArray res;
	bool first = true;

	res.append('{');

	QMapIterator<QString,QVariant> i(var);

	while(i.hasNext()) {
		i.next();
		if (first) {
			first = false;
		} else {
			res.append(',');
		}
		res.append(encode(i.key()));
		res.append(':');
		res.append(encode(i.value()));
	}
	res.append('}');
	return res;
}

QByteArray QtJson::encode(const QVariant &var) {
	// encode a QVariant!
	switch(var.type()) {
		case QVariant::Bool:
			return var.toBool()?"true":"false";
		case QVariant::ByteArray:
			return encode(var.toByteArray());
		case QVariant::List:
			return encode(var.toList());
		case QVariant::Map:
			return encode(var.toMap());
		case QVariant::StringList:
			return encode(var.toList());
		case QVariant::String:
		case QVariant::Url:
			return encode(var.toString());
		// types that can be passed as is
		case QVariant::UInt:
		case QVariant::ULongLong:
		case QVariant::Double:
		case QVariant::Int:
		case QVariant::LongLong:
			return var.toString().toAscii();
		default:
			return "null"; // mh?
	}
}

QVariant QtJson::decode(const QByteArray &var) {
	return QScriptEngine().evaluate("("+QString(var)+")").toVariant();
}


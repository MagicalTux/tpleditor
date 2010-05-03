#include "QtWddx.hpp"

#include <QDateTime>

// This code is released under public domain license because of high level of crappyness
// If "public domain" is not legal within your territory, you can treat this code as licensed
// under BSD license.
//
// The author (Mark Karpeles <mark@hell.ne.jp>) disclaim any responsability for the creation
// of this code, and whatever you happen to do (or not) is strictly your own responsability.
//
// If you are not happy with those terms, do not use this code. I can NOT guarantee that this
// code won't disable your e1000 ethernet adapter, or make your computer go *BOOM*. You've been
// warned.

QVariant QtWddx::unserialize(QDomElement element) {
	// check element
	if (element.isNull()) return QVariant();

//	qDebug("type=%s", element.tagName().toLatin1().data());

	if (element.tagName() == "struct") {
		// ok, this is a struct. We need to prepare some stuff. A struct is equivalent to QMap<QString, QVariant>
		QMap <QString, QVariant> data;
		QDomNodeList children = element.childNodes();

		for(int i=0; i<children.count(); i++) {
			QDomElement child = children.item(i).toElement();
			if (child.isNull()) continue; // skip invalid

			if (child.tagName() != "var") continue;
			QString varname = child.attribute("name");

			QVariant elem = unserialize(child.firstChild().toElement());
			if (!elem.isValid()) continue;
			data[varname] = elem;
		}

		// parsing completed, return data
		return data;
	}

	if (element.tagName() == "array") {
		// an array is in fact a QList<QVariant>
		QList<QVariant> data;
		QDomNodeList children = element.childNodes();

		// compare values
		if (children.count() != element.attribute("length").toInt()) return QVariant();

		// ok, let's parse!
		for(int i=0; i<children.count(); i++) {
			QDomElement child = children.item(i).toElement();
			if (child.isNull()) continue; // skip invalid

			QVariant elem = unserialize(child);
			if (!elem.isValid()) continue;
			data.append(elem);
		}

		// parsing completed, return data
		return data;
	}

	if (element.tagName() == "string") {
		// this one is easy :)
		return element.text();
	}

	if (element.tagName() == "number") {
		return element.text().toFloat();
	}

	if (element.tagName() == "boolean") {
		QString val = element.attribute("value");
		if ((val == "false") || (val == "0")) return false;
		return true;
	}

	// next ones are un-tested quick implementations of various stuff

	if (element.tagName() == "null") {
		return QVariant(QVariant::String); // This will create a "null variant" of type string, so you can do if (variant.isNull()) and see that it's null
		// According to WDDX doc, "Languages that do not have the concept of a null value should deserialize nulls as empty strings."
		// I guess this is OK this way.
	}

	if (element.tagName() == "dateTime") {
		return QDateTime::fromString(element.text(), Qt::ISODate);
	}

	if (element.tagName() == "binary") {
		// TODO: validate length if provided
		return QByteArray::fromBase64(element.text().toLatin1());
	}

	qDebug("unhandled type=%s", element.tagName().toLatin1().data());
//	qDebug("version=%s", root.attribute("version").toLatin1().data());

	return QVariant();
}

QVariant QtWddx::unserialize(const QByteArray xml) {
	QDomDocument dom;
	if (!dom.setContent(QByteArray("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n").append(xml), false)) return QVariant();

	QDomElement root = dom.documentElement();

	// Check document version, return an invalid variant if not valid
	if (root.tagName() != "wddxPacket") return QVariant();
	if (root.attribute("version") != "1.0") return QVariant();

	// get childs from root element (should only have two, "header" and "data")
	QDomNodeList children = root.childNodes();

	if (children.count() != 2) return QVariant();

	// get data from header
	QDomElement header = children.item(0).toElement();
	QDomElement data = children.item(1).toElement();

	// is one of the two element not an element?
	if ((header.isNull()) || (data.isNull())) return QVariant();

	// check tagnames
	if (header.tagName() != "header") return QVariant();
	if (data.tagName() != "data") return QVariant();

	return unserialize(data.firstChild().toElement());
}

void QtWddx::serialize(QDomDocument &xml, QDomElement &node, const QVariant &data) {
	// convert an element!

	if (data.type() == QVariant::Map) {
		QMap<QString, QVariant> children = data.value<QMap<QString, QVariant> >();

		// init xml
		QDomElement strct = xml.createElement("struct");
		node.appendChild(strct);

		for (QMap<QString, QVariant>::iterator i = children.begin(); i != children.end(); ++i) {
			// for each var...
			QDomElement var = xml.createElement("var");
			var.setAttribute("name", i.key());
			strct.appendChild(var);

			serialize(xml, var, i.value());
		}

		return;
	}

	if (data.type() == QVariant::List) {
		QList<QVariant> children = data.value<QList<QVariant> >();

		// init xml
		QDomElement array = xml.createElement("array");
		array.setAttribute("length", QString("%1").arg(children.count()));
		node.appendChild(array);

		for(QList<QVariant>::iterator i = children.begin(); i != children.end(); ++i) {
			serialize(xml, array, *i);
		}

		return;
	}

	if (data.type() == QVariant::String) {
		// ok, we got a string!
		if (data.isNull()) { // handle null values
			node.appendChild(xml.createElement("null"));
			return;
		}
		QDomElement str = xml.createElement("string");
		node.appendChild(str);

		QDomText str_data = xml.createTextNode(data.value<QString>());
		str.appendChild(str_data);

		return;
	}

	if (data.type() == QVariant::Bool) {
		QDomElement b = xml.createElement("boolean");
		node.appendChild(b);

		QDomText b_data = xml.createTextNode(data.toBool()?"true":"false");
		b.appendChild(b_data);

		return;
	}

	if ((data.type() == QVariant::Double) || (data.type() == QVariant::Int)) {
		QDomElement num = xml.createElement("number");
		node.appendChild(num);

		QDomText num_data = xml.createTextNode(data.value<QString>());
		num.appendChild(num_data);

		return;
	}

	if (data.type() == QVariant::DateTime) {
		QDomElement elem = xml.createElement("dateTime");
		node.appendChild(elem);

		elem.appendChild(xml.createTextNode(data.toDateTime().toString(Qt::ISODate)));

		return;
	}

	if (data.type() == QVariant::ByteArray) {
		QDomElement elem = xml.createElement("binary");
		QByteArray value = data.toByteArray();
		node.appendChild(elem);
		elem.setAttribute("length", QString("%1").arg(value.size()));
		elem.appendChild(xml.createTextNode(value.toBase64()));

		return;
	}

	// don't know what we have there, try to convert to string
	if (data.canConvert(QVariant::String)) {
		qDebug("got unknown stuff: %s", data.typeName());

		QDomElement str = xml.createElement("string");
		node.appendChild(str);

		QDomText str_data = xml.createTextNode(data.value<QString>());
		str.appendChild(str_data);

		return;
	}

	qDebug("unhandled type=%s", data.typeName());
}

QString QtWddx::serialize(const QVariant &datavar) {
	// Ok, we got some qvariant and we want to serialize it... initialize a domdocument

	QDomDocument xml;
	QDomElement root = xml.createElement("wddxPacket");
	root.setAttribute("version", "1.0");
	xml.appendChild(root);

	QDomElement header = xml.createElement("header");
	root.appendChild(header);

	QDomElement data = xml.createElement("data");
	root.appendChild(data);

	serialize(xml, data, datavar);

	return xml.toString(1); // no indent
}


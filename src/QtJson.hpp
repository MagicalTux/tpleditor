#include <QByteArray>
#include <QVariant>

class QtJson {
public:
	static QVariant decode(const QByteArray &var);

	static QByteArray encode(const QVariant &var);
	static QByteArray encode(const QString &var);
	static QByteArray encode(const QByteArray &var);
	static QByteArray encode(const QList<QVariant> &var);
	static QByteArray encode(const QMap<QString,QVariant> &var);
};


#ifndef UTIL_H
#define UTIL_H

#include <QColor>
#include <QString>
#include <QByteArray>
//==================================================================================

class Util
{
public:
	static QColor  color(int value, int alpha = 255);
	static bool    decodeColor(QColor &result, const QString &colorStr);
	static QString encodeColor(const QColor &color);
};
//==================================================================================

#endif

#include "util.h"
//==================================================================================

QString Util::encodeColor(const QColor &color)
{
	return color.name();
}
//==================================================================================

/*
 * Warning: Ensure length of buf greater than charCount !
 */
static void decodeHexInt(unsigned int &result, int charCount, const char *buf, unsigned int defaultChar)
{
	result = 0;
	defaultChar = defaultChar & 0x0f;
	unsigned int v;
	char c;

	for (int i = 0; i < charCount; i++) {
		c = buf[i];
		if      (c >= '0' && c <= '9') { v = c - '0';        }
		else if (c >= 'A' && c <= 'F') { v = c - 'A' + 0x0a; }
		else if (c >= 'a' && c <= 'f') { v = c - 'a' + 0x0a; }
		else                           { v = defaultChar;    }
		result |= (v << ((charCount - 1 - i) * 4));
	}
}

bool Util::decodeColor(QColor &result, const QString &colorStr)
{
	QByteArray ba = colorStr.toAscii();
	if (ba.isEmpty()) return false;

	const char *src = ba.constData();
	char buf[9];
	int len = ba.length();

	if (len > 0 && src[0] == '#') {
		len--;
		src = &src[1];	// skip '#', #aabbccdd
	}

	buf[6] = 'F';
	buf[7] = 'F';
	buf[8] = '\0';

	switch (len) {
		case 4:
			buf[6] = src[3];  buf[7] = src[3];	// alpha value
		case 3:
			buf[0] = src[0];  buf[1] = src[0];
			buf[2] = src[1];  buf[3] = src[1];
			buf[4] = src[2];  buf[5] = src[2];
			break;
		case 8:
		case 6:
			memcpy(buf, src, len);
			break;
		default:
			memcpy(buf, "000000FF", 8);
	}

	//printf("color-buf: %s\n", buf);
	unsigned int rgb, alpha;

	decodeHexInt(rgb, 6, buf, 0x00);
	decodeHexInt(alpha, 2, &buf[6], 0x0f);

	//printf("color-val: %x\n", val);
	//sscanf (buf, "%x", &val);
	result = Util::color(rgb, alpha);
	return true;
}

QColor Util::color(int value, int alpha)
{
	int r = (value & 0x00ff0000) >> 16;
	int g = (value & 0x0000ff00) >> 8;
	int b = (value & 0x000000ff) >> 0;

	return QColor(r, g, b, alpha);
}
//==================================================================================

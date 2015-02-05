#include <stdio.h>
#include <string.h>

#include <QtCore/QTimer>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtGui/QPainter>
#include <QDebug>

#include "networkapplet.h"
#include "panelwindow.h"
#include "panel.h"
//==================================================================================

NetWorkApplet::NetWorkApplet(PanelWindow *panelWindow)
	: Applet(panelWindow)
	, m_count(2)
{
	m_timer = new QTimer();
	m_timer->setSingleShot(false);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(updateContent()));
}

NetWorkApplet::~NetWorkApplet()
{
}
//==================================================================================

bool NetWorkApplet::start()
{
	scheduleUpdate();
	setInteractive(true);

	return true;
}

bool NetWorkApplet::stop()
{
	return true;
}

void NetWorkApplet::scheduleUpdate()
{
	m_timer->setInterval(500);
	m_timer->start();
}

void NetWorkApplet::updateContent()
{
	if (!readDevices()) return;
	QRectF r(m_outRect.x(), m_outRect.y(), m_outRect.width(), m_outRect.height());
	update(r);
}

void NetWorkApplet::layoutChanged()
{
}

QSize NetWorkApplet::desiredSize()
{
	QSize size = calculateMeterSize(m_outRect, m_count * 2, cfg().networkUnit);

	return size;
}
//==================================================================================

#define KB_000        0
#define KB_001     1024
#define KB_003     3072
#define KB_010    10240
#define KB_030    30720
#define KB_100   102400
#define KB_300   307200
#define MB_001  1024000
#define MB_003  3072000
#define MB_010 10240000

void NetWorkApplet::paintUi(QPainter *painter, int X, int Y, int W, int H,
							int val, bool up)
{
	const QColor *rgb_100m = &cfg().meterColorCyan;
	const QColor *rgb_010m = &cfg().meterColorPurple;
	const QColor *rgb_001m = &cfg().meterColorBlue;
	const QColor *rgb_100k = &cfg().meterColorGreen;
	const QColor *rgb_010k = &cfg().meterColorYellow;
	const QColor *rgb_001k = &cfg().meterColorRed;
	const QColor *rgb_none = &cfg().meterColorGray;
	const QColor *rgb;

	int L = H - 2;	// height of total value
	int v;			// height of value

	if      (val > MB_010) { rgb = rgb_100m; v = L; }
	else if (val > MB_001) { rgb = rgb_010m; v = L * val / MB_010; }
	else if (val > KB_100) { rgb = rgb_001m; v = L * val / MB_001; }
	else if (val > KB_010) { rgb = rgb_100k; v = L * val / KB_100; }
	else if (val > KB_001) { rgb = rgb_010k; v = L * val / KB_010; }
	else if (val > KB_000) { rgb = rgb_001k; v = L * val / KB_001; if (!v) v++; }
	else                   { rgb = rgb_none; v = 0; }

	painter->setBrush(QBrush(*rgb));
	painter->drawRect(X, Y, W, H);

	if (v >= L) return;

	int h = L - v;
	int y = up ? (Y + 1) : (Y + 1 + L - h);

	painter->setBrush(QBrush(cfg().meterColorBlack));
	painter->drawRect(X + 1, y, W - 2, h);
}

void NetWorkApplet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);

	painter->setPen(Qt::NoPen);
	painter->setBrush(QBrush(cfg().meterColorBlack));
	painter->drawRect(m_outRect);

	int unitThick = cfg().networkUnit;
	int spacing = cfg().meterSpacing;

	int x = m_outRect.left() + m_padding.left;
	int y = m_outRect.top() + m_padding.top;
	int w = unitThick;
	int h = m_outRect.height() - yPaddings();

	QHash<QString, NetSpeed*>::const_iterator i = m_items.constBegin();
	while (i != m_items.constEnd()) {
	//	QString name = i.key();
		NetSpeed *net = i.value();
		paintUi(painter, x, y, w, h, net->sendOut, true);
		x += unitThick + spacing;
		paintUi(painter, x, y, w, h, net->receive, false);
		x += unitThick + spacing;
		++i;
	}
}
//==================================================================================

static void parseName(const char *pos, const char *str, char *buf, int len)
{
	memset(buf, 0, len);
	len--;
	int i = 0;
	while (pos <= str && i < len) {
		if (*pos != ' ') { buf[i] = *pos; i++; }
		pos++;
	}
}

bool NetWorkApplet::readDevices()
{
	QString path("/proc/net/dev");
	QFile file(path);
	if (!file.exists()) return false;
	if (!file.open(QIODevice::ReadOnly)) return false;

	QByteArray data = file.readAll();
	file.close();

	unsigned long sendOut, receive;
	const char *ori = data.constData();
	const char *str = ori;
	const char *pos = str;
	char buf[32];
	bool ok;
	int readed, count = 0;
	int y = 20;

	if (m_info.strings.size() < 1)
		initHelpInfo();

	while (*str != '\0') {
		ok = false;
		while (*str != '\0') {
			if (*str == ' ') { pos = str; }
			if (*str == ':') {
				parseName(pos, str, buf, 32);
				str++;
				if (memcmp(buf, "irda0:", 6) == 0 ||
					memcmp(buf, "lo:",    3) == 0) continue;
				ok = true;
				break;
			}
			str++;
		}

		if (*str == '\0') break;
		if (!ok) break;

		readed = sscanf(str, "%lu %*d %*d %*d %*d %*d %*d %*d %lu",
						&receive, &sendOut);
		if (readed != 2) continue;

		count++;
		QString name = buf;

		if (m_items.contains(name)) {
			QHash<QString, NetSpeed*>::iterator i = m_items.find(name);
			NetSpeed *netSpeed = i.value();
			netSpeed->setValues(sendOut, receive);
			if (m_hovering) setHelpInfo(netSpeed);
		} else {
			NetSpeed *netSpeed = new NetSpeed(sendOut, receive);
			m_items.insert(name, netSpeed);
			initHelpInfo(name, netSpeed, y);
		}
		y += 20;
	}

	// TODO: remove unpluged devices
	// ...

	if (m_count != count) {
		m_count = count;
		m_panelWindow->updateLayout();
	}

	if (m_hovering)
		showHelp(m_info);

	return true;
}
//==================================================================================
#define BUF_LEN  128
void formatBytes(quint64 bytes, char *buffer, int size);

void NetWorkApplet::setHelpInfo(NetSpeed *net)
{
	char buffer[BUF_LEN];

	formatBytes(net->sendOut, buffer, BUF_LEN);
	QString send = QString::fromUtf8(buffer);
	net->send.setString(send);

	formatBytes(net->receive, buffer, BUF_LEN);
	QString recv = QString::fromUtf8(buffer);
	net->recv.setString(recv);
}

void NetWorkApplet::initHelpInfo(const QString &name, NetSpeed *net, int y)
{
	char buffer[BUF_LEN];

	net->name.setString(0, y, 40, Qt::AlignRight, name);
	m_info.addString(&net->name);

	formatBytes(net->sendOut, buffer, BUF_LEN);
	QString send = QString::fromUtf8(buffer);
	net->send.setString(50, y, 80, Qt::AlignHCenter, send);
	m_info.addString(&net->send);

	formatBytes(net->receive, buffer, BUF_LEN);
	QString recv = QString::fromUtf8(buffer);
	net->recv.setString(130, y, 80, Qt::AlignHCenter, recv);
	m_info.addString(&net->recv);
}

void NetWorkApplet::initHelpInfo()
{
	QString txSend = tr("Send");
	QString txRecv = tr("Recv");

	m_hsSend.setString( 60, 0, 80, Qt::AlignHCenter, txSend);
	m_hsRecv.setString(140, 0, 80, Qt::AlignHCenter, txRecv);

	m_info.clear();
	m_info.addString(&m_hsSend);
	m_info.addString(&m_hsRecv);
}

void formatBytes(quint64 bytes, char *buffer, int size)
{
	if (bytes > 0.0) { // bytes range
		if (bytes > 1024.0) { // KB range
			if (bytes > 1048576.0) { // MB range
				if (bytes > 1073741824.0) { // GB range
					if (bytes > 1099511627776.0) { // TB range
						if (bytes > 1125899906842624.0) { // PB range
							if (bytes > 1152921504606846976.0) { // EB range
								if (bytes > 1180591620717411303424.0) { // ZB range
									if (bytes > 12089258196146291747066176.0) { // YB range
										snprintf(buffer, size, "%.2f YB", (float)bytes/12089258196146291747066176.0);
									} else  // ZB range
										snprintf(buffer, size, "%.2f ZB", (float)bytes/1180591620717411303424.0);
								} else  // EB range
									snprintf(buffer, size, "%.2f EB", (float)bytes/1152921504606846976.0);
							} else // PB range
								snprintf(buffer, size, "%.2f PB", (float)bytes/1125899906842624.0);
						} else // TB range
							snprintf(buffer, size, "%.2f TB", (float)bytes/1099511627776.0);
					} else // GB range
						snprintf(buffer, size, "%.2f GB", (float)bytes/1073741824.0);
				} else //MB range
					snprintf(buffer, size, "%.2f MB", (float)bytes/1048576.0);
			} else //KB range
				snprintf(buffer, size, "%.2f KB", (float)bytes/1024.0);
		} else // byte range
			snprintf(buffer, size, "%llu bytes", bytes);
	} else // none
		snprintf(buffer, size, "0");
}

#ifndef ACPIUTILS_H
#define ACPIUTILS_H

#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <QByteArray>
//==================================================================================

namespace AcpiDeviceType {
	enum { Battery = 0, AcAdapter, ThermalZone, CoolingDevice };
}

namespace AcpiState {
	enum { Unsupported = 0, Supported };
}

namespace AcpiDeal {
	enum { Break, Finished, Continue };
}

namespace AcpiChargeState {
	enum { Unsupported, Charged, Charging, Discharging };
}

typedef int (*AcpiFindDevicesCb) (const QFileInfo &fileInfo, void *udat);

//==================================================================================

struct AcpiUtil
{
	static bool find(QStringList &devices, QString &path, int acpiDeviceType);
	static bool read(QByteArray &result, const QString &path, const char *name);
	static int parseInt(const QByteArray &data);
};

#endif

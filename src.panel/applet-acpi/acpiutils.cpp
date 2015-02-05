#include <stdlib.h>
#include <string.h>

#include <QStringList>
#include <QDir>
#include <QDebug>

#include "acpiutils.h"
//==================================================================================

#define ACPI_PATH_PROC   "/proc/acpi/"
#define ACPI_PATH_SYS    "/sys/class/"

struct AcpiDevice
{
	int         type;
	const char *proc;
	const char *sys;
	const char *nameFilter;
};

AcpiDevice acpiDevices[4] =
{
  { AcpiDeviceType::Battery,       "battery",      "power_supply", "BAT*"            },
  { AcpiDeviceType::AcAdapter,     "ac_adapter",   "power_supply", "AC*"             },
  { AcpiDeviceType::ThermalZone,   "thermal_zone", "thermal",      "thermal_zone*"   },
  { AcpiDeviceType::CoolingDevice, "fan",          "thermal",      "cooling_device*" }
};
//==================================================================================

static bool getDeviceDir(QDir &dir, const char *acpiPath, const char *acpiDir)
{
	char buf[1024];
	snprintf (buf, 1024, "%s%s", acpiPath, acpiDir);
	dir.setPath(QString::fromUtf8(buf));
	return dir.exists();
}

bool AcpiUtil::find(QStringList &devices, QString &path, int acpiDeviceType)
{
	QDir dir;

	if (!getDeviceDir(dir, ACPI_PATH_PROC, acpiDevices[acpiDeviceType].proc) &&
		!getDeviceDir(dir, ACPI_PATH_SYS,  acpiDevices[acpiDeviceType].sys))
	{
		qDebug() << "AcpiUtil::find(): getDeviceDir() failed.";
		return false;
	}

	QString filter(acpiDevices[acpiDeviceType].nameFilter);
	QStringList nameFilters(filter);
	devices = dir.entryList(nameFilters);
	path = dir.absolutePath() + QDir::separator();

	return true;
}
//==================================================================================

bool AcpiUtil::read(QByteArray &result, const QString &path, const char *name)
{
	QFile file(path + name);
	if (!file.exists()) return false;
	if (!file.open(QIODevice::ReadOnly)) return false;

	result = file.readAll();
	file.close();

	return true;
}
//==================================================================================

int AcpiUtil::parseInt(const QByteArray &data)
{
	const char *str = data.constData();
	int n = -1;
	sscanf(str, "%d", &n);
	return n;
}

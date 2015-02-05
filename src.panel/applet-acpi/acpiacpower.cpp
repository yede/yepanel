#include <QDebug>
#include "acpiacpower.h"
#include "acpiutils.h"
//==================================================================================

AcpiAcpower::AcpiAcpower(const QString &name, const QString &path)
	: m_name(name)
	, m_path(path + name + "/")
{
}
//==================================================================================

void AcpiAcpower::read()
{
	online = -1;
	type_ac = -1;

	QByteArray data;

	if (AcpiUtil::read(data, m_path, "online")) {
		online = AcpiUtil::parseInt(data) ? true : false;
	}
	else if (AcpiUtil::read(data, m_path, "state")) {
		online = (data.toLower() == "online");
	}
	else if (AcpiUtil::read(data, m_path, "status")) {
		online = (data.toLower() == "online");
	}

	if (AcpiUtil::read(data, m_path, "type")) {
		type_ac = (data.toLower() == "mains");
	}
}

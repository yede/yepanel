#ifndef ACPIACPOWER_H
#define ACPIACPOWER_H

#include <QString>
#include "helpwidget.h"
//==================================================================================

struct AcpiAcpower
{
	AcpiAcpower(const QString &name, const QString &path);

	void read();

	int  acpi_state;
	int  online;
	int  type_ac;
	char acpi_path[128];
	char acpi_name[8];

private:
	QString m_name;
	QString m_path;
};

#endif

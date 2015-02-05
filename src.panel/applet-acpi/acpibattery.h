#ifndef ACPIBATTERY_H
#define ACPIBATTERY_H

#include <QString>
//==================================================================================

struct AcpiBattery
{
	AcpiBattery(const QString &name, const QString &path);

	bool isBattery;
	int voltage;

	int design_capacity;
//	int design_capacity_unit;
	int last_capacity;
//	int last_capacity_unit;
	int remaining_capacity;
//	int remaining_energy;
	int present_rate;
	int charge_state;

	int hours, minutes, seconds;
	int percentage;
	const char *state_text;
	const char *poststr;

	int  acpi_state;
	char capacity_unit[4];
	char acpi_path[128];
	char acpi_name[8];

	int n_paint;
	int v_paint;

	void init();
	void read();

	static const char *getChargeStateText(int chargeState);

private:
	bool readType();
	bool readVoltage();
	bool readDesignCapacity();
	bool readLastCapacity();
	bool readRemainingCapacity();
	bool readPresentRate();
	bool readCharging();

	bool calcTimeValues();
	bool calcPercentage();

private:
	QString m_name;
	QString m_path;
};

#endif

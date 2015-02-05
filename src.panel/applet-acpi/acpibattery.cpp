#include "acpibattery.h"
#include "acpiutils.h"
//==================================================================================

#define SET_UNIT_mAh() \
	capacity_unit[0] = 'm'; capacity_unit[1] = 'A'; capacity_unit[2] = 'h'; \
	capacity_unit[3] = '\0'

#define SET_UNIT_mWh() \
	capacity_unit[0] = 'm'; capacity_unit[1] = 'W'; capacity_unit[2] = 'h'; \
	capacity_unit[3] = '\0'

#define ACPI_MIN_PRESENT_RATE 1
#define ACPI_MIN_CAPACITY     1
#define ACPI_MIN_TEMP         0.01
//==================================================================================

AcpiBattery::AcpiBattery(const QString &name, const QString &path)
	: m_name(name)
	, m_path(path + name + "/")
{
	init();
}
//==================================================================================

bool AcpiBattery::readType()
{
	QByteArray data;
	if (!AcpiUtil::read(data, m_path, "type")) return false;

	isBattery = (data.toLower() == "battery");
	return true;
}
//==================================================================================

bool AcpiBattery::readVoltage()
{
	QByteArray data;
	if (!AcpiUtil::read(data, m_path, "voltage_now")) return false;

	voltage = AcpiUtil::parseInt(data) / 1000;
	if (voltage == 0) voltage = -1;

	return true;
}
//==================================================================================

bool AcpiBattery::readDesignCapacity()
{
	QByteArray data;

	if (AcpiUtil::read(data, m_path, "energy_full_design")) {
		if (voltage > 0) {
			design_capacity = AcpiUtil::parseInt(data) / voltage;
		} else {
			design_capacity = AcpiUtil::parseInt(data) / 1000;
			SET_UNIT_mWh();
		}
		return true;
	}

	if (AcpiUtil::read(data, m_path, "charge_full_design")) {
		design_capacity = AcpiUtil::parseInt(data) / 1000;
		return true;
	}

	return false;
}
//==================================================================================

bool AcpiBattery::readLastCapacity()
{
	QByteArray data;

	if (AcpiUtil::read(data, m_path, "energy_full")) {
		if (voltage > 0) {
			last_capacity = AcpiUtil::parseInt(data) / voltage;
		} else {
			last_capacity = AcpiUtil::parseInt(data) / 1000;
			SET_UNIT_mWh();
		}
		return true;
	}

	if (AcpiUtil::read(data, m_path, "charge_full")) {
		last_capacity = AcpiUtil::parseInt(data) / 1000;
		return true;
	}

	if (AcpiUtil::read(data, m_path, "last full capacity")) {
		last_capacity = AcpiUtil::parseInt(data);
		return true;
	}

	return false;
}
//==================================================================================

bool AcpiBattery::readRemainingCapacity()
{
	QByteArray data;

	if (AcpiUtil::read(data, m_path, "energy_now")) {
		if (voltage > 0) {
			remaining_capacity = AcpiUtil::parseInt(data) / voltage;
		} else {
			remaining_capacity = AcpiUtil::parseInt(data) / 1000;
		//	SET_UNIT_mWh();
		}
		return true;
	}

	if (AcpiUtil::read(data, m_path, "charge_now")) {
		remaining_capacity = AcpiUtil::parseInt(data) / 1000;
		return true;
	}

	if (AcpiUtil::read(data, m_path, "remaining capacity")) {
		remaining_capacity = AcpiUtil::parseInt(data);
		return true;
	}

	return false;
}
//==================================================================================

bool AcpiBattery::readPresentRate()
{
	QByteArray data;

	if (AcpiUtil::read(data, m_path, "power_now")) {
		if (voltage != -1) {
			present_rate = AcpiUtil::parseInt(data) / voltage;
		} else {
			present_rate = AcpiUtil::parseInt(data) / 1000;
		}
		poststr = "by: power_now";
		return true;
	}

	if (AcpiUtil::read(data, m_path, "current_now")) {
		present_rate = AcpiUtil::parseInt(data) / 1000;
		poststr = "by: current_now";
		return true;
	}

	if (AcpiUtil::read(data, m_path, "present rate")) {
		present_rate = AcpiUtil::parseInt(data);
		poststr = "by: present rate";
		return true;
	}

	return false;
}
//==================================================================================

bool AcpiBattery::readCharging()
{
	QByteArray data;

	if (AcpiUtil::read(data, m_path, "status") ||
		AcpiUtil::read(data, m_path, "State")  ||
		AcpiUtil::read(data, m_path, "charging state"))
	{
		data = data.toLower();

		if (data == "full") {
			state_text = "full";
			charge_state = AcpiChargeState::Charged;
			if (present_rate > ACPI_MIN_PRESENT_RATE) {
				seconds = 3600 * remaining_capacity / present_rate;
				poststr = " remaining";
			} else {
				poststr = "stay at zero rate.";
				seconds = -1;
			}
		}
		else if (data == "charging") {
			state_text = "charging";
			charge_state = AcpiChargeState::Charging;
			if (present_rate > ACPI_MIN_PRESENT_RATE) {
				seconds = 3600 * (last_capacity - remaining_capacity) / present_rate;
				poststr = " until charged";
			} else {
				poststr = "charging at zero rate - will never fully charge.";
				seconds = -1;
			}
		}
		else if (data == "discharging") {
			state_text = "discharging";
			charge_state = AcpiChargeState::Discharging;
			if (present_rate > ACPI_MIN_PRESENT_RATE) {
				seconds = 3600 * remaining_capacity / present_rate;
				poststr = " remaining";
			} else {
				poststr = "discharging at zero rate - will never fully discharge.";
				seconds = -1;
			}
		}
		else {
			state_text = "Unknown";
			charge_state = AcpiChargeState::Unsupported;
		}

		return true;
	}

	return false;

}
//==================================================================================
bool AcpiBattery::calcTimeValues()
{
	if (seconds > 0) {
		hours = seconds / 3600;
		seconds -= 3600 * hours;
		minutes = seconds / 60;
		seconds -= 60 * minutes;
	}

	return true;
}

bool AcpiBattery::calcPercentage()
{
	if (last_capacity < ACPI_MIN_CAPACITY)
		percentage = 0;
	else
		percentage = remaining_capacity * 100 / last_capacity;

	if (percentage > 100) percentage = 100;

	return true;
}
//==================================================================================

void AcpiBattery::init()
{
	isBattery = false;
	voltage = -1;

	design_capacity = -1;
//	design_capacity_unit = -1;
	last_capacity = -1;
//	last_capacity_unit = -1;
	remaining_capacity = -1;
//	remaining_energy = -1;
	present_rate = -1;
	charge_state = AcpiChargeState::Unsupported;

	hours = -1;
	minutes = -1;
	seconds = -1;
	percentage = -1;
	state_text = "";
	poststr = "rate information unavailable";
}

void AcpiBattery::read()
{
	init();
	SET_UNIT_mAh();

	readType();
	readVoltage();

	readDesignCapacity();
	readLastCapacity();
	readRemainingCapacity();

	readPresentRate();
	readCharging();

	calcTimeValues();
	calcPercentage();
}
//==================================================================================

const char *AcpiBattery::getChargeStateText(int chargeState)
{
	switch (chargeState) {
		case AcpiChargeState::Charged: return "charged";
		case AcpiChargeState::Charging: return "charging";
		case AcpiChargeState::Discharging: return "discharging";
	//  case ACPI_CHARGE_NOSUPP: return "charge-info unavailable";
	}
	return "charge-info unavailable";
}

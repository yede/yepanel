#ifndef _ACPINFO_H
#define _ACPINFO_H

//==================================================================================

enum { ACPI_STATE_UNKNOWN, ACPI_STATE_WORKING, ACPI_STATE_NOSUPP };
enum { ACPI_UNKNOWN = -1, ACPI_FALSE = 0, ACPI_TRUE = 1 };
enum { ACPI_FINISHED, ACPI_CONTINUE };
enum { ACPI_SUCCESS, ACPI_FAILED, ACPI_OPEN_FAILED, ACPI_READ_FAILED };
enum { ACPI_CHARGE_NOSUPP, ACPI_CHARGED, ACPI_CHARGING, ACPI_DISCHARGING };

struct Battery
{
	int type_battery;
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

	int x, y;		    // for cairo_set_source_surface
	int n_paint;

};

struct Adapter
{
    int  acpi_state;
    int  online;
    int  type_ac;
    char acpi_path[128];
    char acpi_name[8];

};
//==================================================================================

int  acpi_battery_init(Battery *batteries, int max_count);
void acpi_battery_read(Battery *bi);

const char *acpi_battery_charge_state_text(int charge_state);
//==================================================================================

int  acpi_ac_adapter_init(Adapter *ac);
void acpi_ac_adapter_read(Adapter *ac);

//==================================================================================
#endif

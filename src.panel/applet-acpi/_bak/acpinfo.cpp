
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <errno.h>

#include "acpinfo.h"
//==================================================================================

#define ACPI_MIN_PRESENT_RATE 1
#define ACPI_MIN_CAPACITY     1
#define ACPI_MIN_TEMP         0.01

#define ACPI_PATH_PROC   "/proc/acpi"
#define ACPI_PATH_SYS    "/sys/class"

namespace AcpiType {
	enum { Battery = 0, AcAdapter, ThermalZone, CoolingDevice };
}

struct AcpiDevice
{
	int         type;
	const char *proc;
	const char *sys;
	const char *sys_dev;

};

AcpiDevice acpiDevices[4] =
{
	{ AcpiType::Battery,       "battery",      "power_supply", "BAT" },
	{ AcpiType::AcAdapter,     "ac_adapter",   "power_supply", "AC" },
	{ AcpiType::ThermalZone,   "thermal_zone", "thermal",      "thermal_zone" },
	{ AcpiType::CoolingDevice, "fan",          "thermal",      "cooling_device" }
};
//==================================================================================

static int find_devices(const char *acpi_path, int device_nr,
						int (*callback) (const char *path, const char *type,
										 const char *name, void *udat),
						void *callback_data)
{
	int proc_interface = (memcmp(acpi_path, "/proc/acpi", 10) == 0);
	const char *device_type = proc_interface ? acpiDevices[device_nr].proc
											 : acpiDevices[device_nr].sys;
	int found = ACPI_FALSE;

	if (chdir(acpi_path) < 0) {
		fprintf(stderr, "No ACPI support in kernel, or incorrect acpi_path (\"%s\").\n", acpi_path);
		return ACPI_FAILED;
	}

	if (chdir(device_type) == 0) {
		struct dirent *de;
		DIR *d = opendir(".");
		if (!d) return ACPI_FAILED;

		const char *prefix = acpiDevices[device_nr].sys_dev;
		while ((de = readdir(d))) {
			if (!strcmp(de->d_name, ".") || !strcmp(de->d_name, "..")) continue;

			if (memcmp(de->d_name, prefix, strlen(prefix)) == 0) {
				found = ACPI_TRUE;
				int flag = callback(acpi_path, device_type, de->d_name,
									callback_data);
				if (flag == ACPI_FINISHED) break;
			}
		}
		closedir(d);
	}

	if (found != ACPI_TRUE) {
		fprintf(stderr, "No support for device path: %s/%s\n", acpi_path, device_type);
		return ACPI_FAILED;
	}

	return ACPI_SUCCESS;
}
//==================================================================================

static int acpi_open_read (char *buf, int buf_len, const char *path, const char *info_file)
{
	static char dev[128];
	snprintf (dev, 128, "%s/%s", path, info_file);

	int fd = open(dev, O_RDONLY);
	if (fd == -1) {
		//snprintf (buf, buf_len, "opening %s", dev);
		//perror (buf);
		return ACPI_OPEN_FAILED;
	}

	memset(buf, 0, buf_len);
	ssize_t sz = read(fd, buf, buf_len);
	close(fd);

	if (sz == -1) {
		//snprintf (buf, buf_len, "reading %s", dev);
		//perror(buf);
		return ACPI_READ_FAILED;
	}

	return ACPI_SUCCESS;
}
//==================================================================================

static int get_unit_value(char *buf)
{
	int n = ACPI_UNKNOWN;
	sscanf(buf, "%d", &n);
	return n;
}

//==================================================================================
// battery
//==================================================================================

#define SET_UNIT_mAh(bi) \
	bi->capacity_unit[0] = 'm'; \
	bi->capacity_unit[1] = 'A'; \
	bi->capacity_unit[2] = 'h'; \
	bi->capacity_unit[3] = '\0'

#define SET_UNIT_mWh(bi) \
	bi->capacity_unit[0] = 'm'; \
	bi->capacity_unit[1] = 'W'; \
	bi->capacity_unit[2] = 'h'; \
	bi->capacity_unit[3] = '\0'
//==================================================================================

static void do_batt_init(Battery *bi)
{
	bi->acpi_state = ACPI_STATE_WORKING;
	SET_UNIT_mAh(bi);
}
//==================================================================================

struct TmpBatt
{
	Battery *batteries;
	int max_count;
	int index;
};

static int on_find_battery(const char *path, const char *type,
						   const char *name, void *udat)
{
	TmpBatt *tmp = (TmpBatt *) udat;
	int i = tmp->index;
	if (i >= tmp->max_count) return ACPI_FINISHED;

	Battery *bi = &tmp->batteries[i];
	snprintf (bi->acpi_path, 128, "%s/%s/%s", path, type, name);
	snprintf (bi->acpi_name, 8, "%s", name);
	do_batt_init(bi);
	//    fprintf(stderr, "FOUND battery: index=%d, name=%s, path=%s\n", i, bi->acpi_name, bi->acpi_path);

	tmp->index ++;
	return (tmp->index < tmp->max_count) ? ACPI_CONTINUE : ACPI_FINISHED;
}

int acpi_battery_init (Battery *batteries, int max_count)
{
	int i;
	for (i = 0; i < max_count; i++) {
		batteries[i].acpi_state = ACPI_STATE_UNKNOWN;
	}

	TmpBatt tmp;
	tmp.batteries = batteries;
	tmp.max_count = max_count;
	tmp.index = 0;

	int flag = find_devices(ACPI_PATH_SYS, AcpiType::Battery,
							on_find_battery, &tmp);
	return flag;
}
//==================================================================================

static int acpi_batt_read_type (Battery *bi, char *buf, int buf_len)
{
	if (acpi_open_read(buf, buf_len, bi->acpi_path, "type") == ACPI_SUCCESS) {
		bi->type_battery = (strncasecmp(buf, "battery", 7) == 0);
		return ACPI_SUCCESS;
	}

	return ACPI_READ_FAILED;
}
//==================================================================================

static int acpi_batt_read_voltage (Battery *bi, char *buf, int buf_len)
{
	if (acpi_open_read(buf, buf_len, bi->acpi_path, "voltage_now") == ACPI_SUCCESS) {
		bi->voltage = get_unit_value(buf) / 1000;
		if (bi->voltage == 0) bi->voltage = ACPI_UNKNOWN;
		return ACPI_SUCCESS;
	}

	return ACPI_READ_FAILED;
}
//==================================================================================

static int acpi_batt_read_design_capacity (Battery *bi, char *buf, int buf_len)
{
	if (acpi_open_read(buf, buf_len, bi->acpi_path, "energy_full_design") == ACPI_SUCCESS) {
		if (bi->voltage != ACPI_UNKNOWN) {
			bi->design_capacity = get_unit_value(buf) / bi->voltage;
		} else {
			bi->design_capacity = get_unit_value(buf) / 1000;
			SET_UNIT_mWh(bi);
		}
		return ACPI_SUCCESS;
	}

	if (acpi_open_read(buf, buf_len, bi->acpi_path, "charge_full_design") == ACPI_SUCCESS) {
		bi->design_capacity = get_unit_value(buf) / 1000;
		return ACPI_SUCCESS;
	}

	return ACPI_READ_FAILED;
}
//==================================================================================

static int acpi_batt_read_last_capacity (Battery *bi, char *buf, int buf_len)
{
	if (acpi_open_read(buf, buf_len, bi->acpi_path, "energy_full") == ACPI_SUCCESS) {
		if (bi->voltage != ACPI_UNKNOWN) {
			bi->last_capacity = get_unit_value(buf) / bi->voltage;
		} else {
			bi->last_capacity = get_unit_value(buf) / 1000;
			SET_UNIT_mWh(bi);
		}
		return ACPI_SUCCESS;
	}

	if (acpi_open_read(buf, buf_len, bi->acpi_path, "charge_full") == ACPI_SUCCESS) {
		bi->last_capacity = get_unit_value(buf) / 1000;
		return ACPI_SUCCESS;
	}

	if (acpi_open_read(buf, buf_len, bi->acpi_path, "last full capacity") == ACPI_SUCCESS) {
		bi->last_capacity = get_unit_value(buf);
		return ACPI_SUCCESS;
	}

	return ACPI_READ_FAILED;
}
//==================================================================================

static int acpi_batt_read_remaining_capacity (Battery *bi, char *buf, int buf_len)
{
	if (acpi_open_read(buf, buf_len, bi->acpi_path, "energy_now") == ACPI_SUCCESS) {
		if (bi->voltage != ACPI_UNKNOWN) {
			bi->remaining_capacity = get_unit_value(buf) / bi->voltage;
		} else {
			bi->remaining_capacity = get_unit_value(buf) / 1000;
		//	SET_UNIT_mWh(bi);
		}
		return ACPI_SUCCESS;
	}

	if (acpi_open_read(buf, buf_len, bi->acpi_path, "charge_now") == ACPI_SUCCESS) {
		bi->remaining_capacity = get_unit_value(buf) / 1000;
		return ACPI_SUCCESS;
	}

	if (acpi_open_read(buf, buf_len, bi->acpi_path, "remaining capacity") == ACPI_SUCCESS) {
		bi->remaining_capacity = get_unit_value(buf);
		return ACPI_SUCCESS;
	}

	return ACPI_READ_FAILED;
}
//==================================================================================

static int acpi_batt_read_present_rate (Battery *bi, char *buf, int buf_len)
{
	if (acpi_open_read(buf, buf_len, bi->acpi_path, "power_now") == ACPI_SUCCESS) {
		if (bi->voltage != ACPI_UNKNOWN) {
			bi->present_rate = get_unit_value(buf) / bi->voltage;
		} else {
			bi->present_rate = get_unit_value(buf) / 1000;
		}
		bi->poststr = "by: power_now";
		return ACPI_SUCCESS;
	}

	if (acpi_open_read(buf, buf_len, bi->acpi_path, "current_now") == ACPI_SUCCESS) {
		bi->present_rate = get_unit_value(buf) / 1000;
		bi->poststr = "by: current_now";
		return ACPI_SUCCESS;
	}

	if (acpi_open_read(buf, buf_len, bi->acpi_path, "present rate") == ACPI_SUCCESS) {
		bi->present_rate = get_unit_value(buf);
		bi->poststr = "by: present rate";
		return ACPI_SUCCESS;
	}

	return ACPI_READ_FAILED;
}
//==================================================================================

static int acpi_batt_read_charging (Battery *bi, char *buf, int buf_len)
{
	if (acpi_open_read(buf, buf_len, bi->acpi_path, "status") == ACPI_SUCCESS ||
			acpi_open_read(buf, buf_len, bi->acpi_path, "State") == ACPI_SUCCESS ||
			acpi_open_read(buf, buf_len, bi->acpi_path, "charging state") == ACPI_SUCCESS)
	{
		if (!strncasecmp(buf, "full", 4)) {
			bi->state_text = "full";
			bi->charge_state = ACPI_CHARGED;
			if (bi->present_rate > ACPI_MIN_PRESENT_RATE) {
				bi->seconds = 3600 * bi->remaining_capacity / bi->present_rate;
				bi->poststr = " remaining";
			} else {
				bi->poststr = "stay at zero rate.";
				bi->seconds = ACPI_UNKNOWN;
			}
		} else if (!strncasecmp(buf, "charging", 8)) {
			bi->state_text = "charging";
			bi->charge_state = ACPI_CHARGING;
			if (bi->present_rate > ACPI_MIN_PRESENT_RATE) {
				bi->seconds = 3600 * (bi->last_capacity - bi->remaining_capacity) / bi->present_rate;
				bi->poststr = " until charged";
			} else {
				bi->poststr = "charging at zero rate - will never fully charge.";
				bi->seconds = ACPI_UNKNOWN;
			}
		} else if (!strncasecmp(buf, "discharging", 11)) {
			bi->state_text = "discharging";
			bi->charge_state = ACPI_DISCHARGING;
			if (bi->present_rate > ACPI_MIN_PRESENT_RATE) {
				bi->seconds = 3600 * bi->remaining_capacity / bi->present_rate;
				bi->poststr = " remaining";
			} else {
				bi->poststr = "discharging at zero rate - will never fully discharge.";
				bi->seconds = ACPI_UNKNOWN;
			}
		} else {
			bi->state_text = "Unknown";
			bi->charge_state = ACPI_CHARGE_NOSUPP;
		}
		return ACPI_SUCCESS;
	}

	return ACPI_READ_FAILED;

}
//==================================================================================

static int acpi_batt_calc_time (Battery *bi)
{
	if (bi->seconds > 0) {
		bi->hours = bi->seconds / 3600;
		bi->seconds -= 3600 * bi->hours;
		bi->minutes = bi->seconds / 60;
		bi->seconds -= 60 * bi->minutes;
	}

	return ACPI_SUCCESS;
}

static int acpi_batt_calc_percentage (Battery *bi)
{
	if (bi->last_capacity < ACPI_MIN_CAPACITY)
		bi->percentage = 0;
	else
		bi->percentage = bi->remaining_capacity * 100 / bi->last_capacity;

	if (bi->percentage > 100) bi->percentage = 100;

	return ACPI_SUCCESS;
}
//==================================================================================

void acpi_battery_read(Battery *bi)
{
	static char buf[2048];
	int buf_len = 2048;

	bi->type_battery = ACPI_TRUE;
	bi->voltage = ACPI_UNKNOWN;

	bi->design_capacity = ACPI_UNKNOWN;
//	bi->design_capacity_unit = ACPI_UNKNOWN;
	bi->last_capacity = ACPI_UNKNOWN;
//	bi->last_capacity_unit = ACPI_UNKNOWN;
	bi->remaining_capacity = ACPI_UNKNOWN;
//	bi->remaining_energy = ACPI_UNKNOWN;
	bi->present_rate = ACPI_UNKNOWN;
	bi->charge_state = ACPI_CHARGE_NOSUPP;

	bi->hours = ACPI_UNKNOWN;
	bi->minutes = ACPI_UNKNOWN;
	bi->seconds = ACPI_UNKNOWN;
	bi->percentage = ACPI_UNKNOWN;
	bi->state_text = "";
	bi->poststr = "rate information unavailable";

	SET_UNIT_mAh(bi);
	acpi_batt_read_type(bi, buf, buf_len);
	acpi_batt_read_voltage(bi, buf, buf_len);

	acpi_batt_read_design_capacity(bi, buf, buf_len);
	acpi_batt_read_last_capacity(bi, buf, buf_len);
	acpi_batt_read_remaining_capacity(bi, buf, buf_len);

	acpi_batt_read_present_rate(bi, buf, buf_len);
	acpi_batt_read_charging(bi, buf, buf_len);

	acpi_batt_calc_time(bi);
	acpi_batt_calc_percentage(bi);
}
//==================================================================================

const char *acpi_battery_charge_state_text(int charge_state)
{
	switch (charge_state) {
		case ACPI_CHARGED: return "charged";
		case ACPI_CHARGING: return "charging";
		case ACPI_DISCHARGING: return "discharging";
	//  case ACPI_CHARGE_NOSUPP: return "charge-info unavailable";
	}
	return "charge-info unavailable";
}

//==================================================================================
// ac_adapter
//==================================================================================

static int on_find_ac_adapter(const char *path, const char *type,
							  const char *name, void *udat)
{
	Adapter *ac = (Adapter *) udat;
	snprintf (ac->acpi_path, 128, "%s/%s/%s", path, type, name);
	snprintf (ac->acpi_name, 8, "%s", name);
	//    fprintf(stderr, "FOUND ac_adapter: name=%s, path=%s\n", ac->acpi_name, ac->acpi_path);
	ac->acpi_state = ACPI_STATE_WORKING;
	return ACPI_FINISHED;
}

int acpi_ac_adapter_init (Adapter *ac)
{
	ac->acpi_state = ACPI_STATE_UNKNOWN;

	return find_devices(ACPI_PATH_SYS, AcpiType::AcAdapter,
						on_find_ac_adapter, ac);
}
//==================================================================================

void acpi_ac_adapter_read(Adapter *ac)
{
	static char buf[2048];
	int buf_len = 2048;

	ac->online = ACPI_UNKNOWN;
	ac->type_ac = ACPI_UNKNOWN;

	if (acpi_open_read(buf, buf_len, ac->acpi_path, "online") == ACPI_SUCCESS) {
		ac->online = get_unit_value(buf) ? ACPI_TRUE : ACPI_FALSE;
	}
	else if (acpi_open_read(buf, buf_len, ac->acpi_path, "state") == ACPI_SUCCESS) {
		ac->online = (memcmp(buf, "online", 6) == 0) ? ACPI_TRUE : ACPI_FALSE;
	}
	else if (acpi_open_read(buf, buf_len, ac->acpi_path, "status") == ACPI_SUCCESS) {
		ac->online = (memcmp(buf, "online", 6) == 0) ? ACPI_TRUE : ACPI_FALSE;
	}

	if (acpi_open_read(buf, buf_len, ac->acpi_path, "type") == ACPI_SUCCESS) {
		ac->type_ac = !strncasecmp(buf, "Mains", 5) ? ACPI_TRUE : ACPI_FALSE;
	}
}

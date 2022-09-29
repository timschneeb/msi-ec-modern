// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * msi-ec.c - MSI Embedded Controller for laptops support.
 *
 * Adapted to support MSI Modern 15 (A11M)
 *
 * This driver exports a few files in /sys/devices/platform/msi-laptop:
 *   webcam            Integrated webcam activation
 *   fn_key            Function key location
 *   win_key           Windows key location
 *   battery_mode      Battery health options
 *   cooler_boost      Cooler boost function
 *   shift_mode        CPU & GPU performance modes
 *   fan_mode          FAN performance modes
 *   fw_version        Firmware version
 *   fw_release_date   Firmware release date
 *   cpu/..            CPU related options
 *   gpu/..            GPU related options
 *
 * This driver also registers available led class devices for
 * mute, micmute and keyboard_backlight leds
 *
 * This driver might not work on other laptops produced by MSI. Also, and until
 * future enhancements, no DMI data are used to identify your compatibility
 *
 */

#include "constants.h"

#include <acpi/battery.h>
#include <linux/acpi.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define streq(x, y) (strcmp(x, y) == 0 || strcmp(x, y "\n") == 0)

static int ec_read_seq(u8 addr, u8 *buf, u8 len)
{
	int result;
	u8 i;
	for (i = 0; i < len; i++) {
		result = ec_read(addr + i, buf + i);
		if (result < 0)
			return result;
	}
	return 0;
}

static int ec_write_bit(u8 addr, u8 index, bool set)
{
	u8 data;
	int result;

	result = ec_read(addr, &data);
	if (result < 0)
		return result;
	if(set)
		data |= (1UL << index);
	else
		data &= ~(1UL << index);

	return ec_write(addr, data);
}

static bool is_bit_set(u8 index, u8 byte)
{
	return (byte >> index) & 1UL;
}

// ============================================================ //
// Sysfs platform device attributes (root)
// ============================================================ //

static ssize_t webcam_show(struct device *device, struct device_attribute *attr,
			   char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_WEBCAM_ADDRESS, &rdata);
	if (result < 0)
		return result;

	if(is_bit_set(MSI_EC_WEBCAM_BIT, rdata))
		return sprintf(buf, "%s\n", "on");
	else
		return sprintf(buf, "%s\n", "off");
}

static ssize_t webcam_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int result = -EINVAL;

	if (streq(buf, "on"))
		result = ec_write_bit(MSI_EC_WEBCAM_ADDRESS,
				      MSI_EC_WEBCAM_BIT,
				      TRUE);

	if (streq(buf, "off"))
		result = ec_write_bit(MSI_EC_WEBCAM_ADDRESS,
				      MSI_EC_WEBCAM_BIT,
				      FALSE);

	if (result < 0)
		return result;

	return count;
}

static ssize_t fn_key_show(struct device *device, struct device_attribute *attr,
			   char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_FN_WIN_ADDRESS, &rdata);
	if (result < 0)
		return result;

	if(is_bit_set(MSI_EC_FN_WIN_BIT, rdata) == MSI_EC_FN_KEY_LEFT) {
		return sprintf(buf, "%s\n", "left");
	}
	else {
		return sprintf(buf, "%s\n", "right");
	}
}

static ssize_t fn_key_store(struct device *dev, struct device_attribute *attr,
			    const char *buf, size_t count)
{
	int result = -EINVAL;

	if (streq(buf, "left"))
		result = ec_write_bit(MSI_EC_FN_WIN_ADDRESS,
				      MSI_EC_FN_WIN_BIT,
				      MSI_EC_FN_KEY_LEFT);

	if (streq(buf, "right"))
		result = ec_write_bit(MSI_EC_FN_WIN_ADDRESS,
				      MSI_EC_FN_WIN_BIT,
				      MSI_EC_FN_KEY_RIGHT);

	if (result < 0)
		return result;

	return count;
}

static ssize_t win_key_show(struct device *device,
			    struct device_attribute *attr, char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_FN_WIN_ADDRESS, &rdata);
	if (result < 0)
		return result;

	if(is_bit_set(MSI_EC_FN_WIN_BIT, rdata) == MSI_EC_WIN_KEY_LEFT) {
		return sprintf(buf, "%s\n", "left");
	}
	else {
		return sprintf(buf, "%s\n", "right");
	}
}

static ssize_t win_key_store(struct device *dev, struct device_attribute *attr,
			     const char *buf, size_t count)
{
	int result = -EINVAL;

	if (streq(buf, "left"))
		result = ec_write_bit(MSI_EC_FN_WIN_ADDRESS,
				      MSI_EC_FN_WIN_BIT,
				      MSI_EC_WIN_KEY_LEFT);

	if (streq(buf, "right"))
		result = ec_write_bit(MSI_EC_FN_WIN_ADDRESS,
				      MSI_EC_FN_WIN_BIT,
				      MSI_EC_WIN_KEY_RIGHT);

	if (result < 0)
		return result;

	return count;
}

static ssize_t battery_charge_mode_show(struct device *device,
				 	struct device_attribute *attr, char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_BATTERY_MODE_ADDRESS, &rdata);
	if (result < 0)
		return result;

	switch (rdata) {
	case MSI_EC_BATTERY_MODE_MAX_CHARGE:
		return sprintf(buf, "%s\n", "max");
	case MSI_EC_BATTERY_MODE_MEDIUM_CHARGE:
		return sprintf(buf, "%s\n", "medium");
	case MSI_EC_BATTERY_MODE_MIN_CHARGE:
		return sprintf(buf, "%s\n", "min");
	default:
		return sprintf(buf, "%s (%i)\n", "unknown", rdata);
	}
}

static ssize_t battery_charge_mode_store(struct device *dev,
				  	 struct device_attribute *attr,
				  	 const char *buf, size_t count)
{
	int result = -EINVAL;

	if (streq(buf, "max"))
		result = ec_write(MSI_EC_BATTERY_MODE_ADDRESS,
				  MSI_EC_BATTERY_MODE_MAX_CHARGE);

	if (streq(buf, "medium"))
		result = ec_write(MSI_EC_BATTERY_MODE_ADDRESS,
				  MSI_EC_BATTERY_MODE_MEDIUM_CHARGE);

	if (streq(buf, "min"))
		result = ec_write(MSI_EC_BATTERY_MODE_ADDRESS,
				  MSI_EC_BATTERY_MODE_MIN_CHARGE);

	if (result < 0)
		return result;

	return count;
}

static ssize_t cooler_boost_show(struct device *device,
				 struct device_attribute *attr, char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_COOLER_BOOST_ADDRESS, &rdata);
	if (result < 0)
		return result;

	if(is_bit_set(MSI_EC_COOLER_BOOST_BIT, rdata))
		return sprintf(buf, "%s\n", "on");
	else
		return sprintf(buf, "%s\n", "off");
}

static ssize_t cooler_boost_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf, size_t count)
{
	int result = -EINVAL;

	if (streq(buf, "on"))
		result = ec_write_bit(MSI_EC_COOLER_BOOST_ADDRESS,
				      MSI_EC_COOLER_BOOST_BIT,
				      TRUE);

	if (streq(buf, "off"))
		result = ec_write_bit(MSI_EC_COOLER_BOOST_ADDRESS,
				      MSI_EC_COOLER_BOOST_BIT,
				      FALSE);

	if (result < 0)
		return result;

	return count;
}

static ssize_t shift_mode_show(struct device *device,
			       struct device_attribute *attr, char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_SHIFT_MODE_ADDRESS, &rdata);
	if (result < 0)
		return result;

	switch (rdata) {
	case MSI_EC_SHIFT_MODE_OVERCLOCK:
		return sprintf(buf, "%s\n", "overclock");
	case MSI_EC_SHIFT_MODE_BALANCED:
		return sprintf(buf, "%s\n", "balanced");
	case MSI_EC_SHIFT_MODE_ECO:
		return sprintf(buf, "%s\n", "eco");
	case MSI_EC_SHIFT_MODE_OFF:
		return sprintf(buf, "%s\n", "off");
	default:
		return sprintf(buf, "%s (%i)\n", "unknown", rdata);
	}
}

static ssize_t shift_mode_store(struct device *dev,
				struct device_attribute *attr, const char *buf,
				size_t count)
{
	int result = -EINVAL;

	if (streq(buf, "overclock"))
		result = ec_write(MSI_EC_SHIFT_MODE_ADDRESS,
				  MSI_EC_SHIFT_MODE_OVERCLOCK);

	if (streq(buf, "balanced"))
		result = ec_write(MSI_EC_SHIFT_MODE_ADDRESS,
				  MSI_EC_SHIFT_MODE_BALANCED);

	if (streq(buf, "eco"))
		result = ec_write(MSI_EC_SHIFT_MODE_ADDRESS,
				  MSI_EC_SHIFT_MODE_ECO);

	if (streq(buf, "off"))
		result = ec_write(MSI_EC_SHIFT_MODE_ADDRESS,
				  MSI_EC_SHIFT_MODE_OFF);

	if (result < 0)
		return result;

	return count;
}

static ssize_t fan_mode_show(struct device *device,
			     struct device_attribute *attr, char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_FAN_MODE_ADDRESS, &rdata);
	if (result < 0)
		return result;

	if (is_bit_set(MSI_EC_FAN_MODE_SILENT_BIT, rdata))  {
		return sprintf(buf, "%s\n", "silent");
	}
	else if (is_bit_set(MSI_EC_FAN_MODE_ADVANCED_BIT, rdata))  {
		return sprintf(buf, "%s\n", "advanced");
	}
	else if (is_bit_set(MSI_EC_FAN_MODE_BASIC_BIT, rdata)) {
		return sprintf(buf, "%s\n", "basic");
	}
	else {
		return sprintf(buf, "%s\n", "auto");
	}
}

static ssize_t fan_mode_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	int result = -EINVAL;
	bool is_auto = streq(buf, "auto");
	bool is_silent = streq(buf, "silent");
	bool is_basic = streq(buf, "basic");
	bool is_adv = streq(buf, "advanced");

	if (!is_auto && !is_basic && !is_adv && !is_silent)
		return result;

	result = ec_write_bit(MSI_EC_FAN_MODE_ADDRESS,
			      MSI_EC_FAN_MODE_BASIC_BIT,
			      is_basic);

	if (result < 0)
		return result;

	result = ec_write_bit(MSI_EC_FAN_MODE_ADDRESS,
			      MSI_EC_FAN_MODE_ADVANCED_BIT,
			      is_adv);

	if (result < 0)
		return result;

	result = ec_write_bit(MSI_EC_FAN_MODE_ADDRESS,
			      MSI_EC_FAN_MODE_SILENT_BIT,
			      is_silent);

	if (result < 0)
		return result;

	return count;
}

static ssize_t preset_show(struct device *device,
			     struct device_attribute *attr, char *buf)
{
	int c;
	int v;
	int result;
	u8 rdata;
	bool match;

	for (v = 0; v < ARRAY_SIZE(MSI_EC_PRESET_VALUE_TABLE); v++) {
		match = TRUE;
		for (c = 0; c < ARRAY_SIZE(MSI_EC_PRESET_MEMORY_TABLE); c++) {
			u8 addr = MSI_EC_PRESET_MEMORY_TABLE[c];
			u8 value = MSI_EC_PRESET_VALUE_TABLE[v][c];

			result = ec_read(addr, &rdata);

			if (result < 0) {
				pr_err("msi-ec: preset_store: failed to read from address %#02x "
				       "while checking preset %i (error code %i)",
				       addr, v, result);
				match = FALSE;
				break;
			}

			// Ignore keyboard brightness; not actually relevant
			if (c == MSI_EC_PRESET_COLUMN_KBD_BL)
				continue;
			else if (c == MSI_EC_PRESET_COLUMN_SILENT_FLAG) {
				if(value == is_bit_set(MSI_EC_FAN_MODE_SILENT_BIT, rdata))
					continue;

				match = FALSE;
				break;
			}
			else if (value != rdata) {
				match = FALSE;
				break;
			}
		}

		if (match) {
			switch (v) {
			case MSI_EC_PRESET_SUPER_BATTERY:
				return sprintf(buf, "%s\n", "super_battery");
			case MSI_EC_PRESET_SILENT:
				return sprintf(buf, "%s\n", "silent");
			case MSI_EC_PRESET_BALANCED:
				return sprintf(buf, "%s\n", "balanced");
			case MSI_EC_PRESET_HIGH_PERFORMANCE:
				return sprintf(buf, "%s\n", "high_performance");
			}
		}
	}

	return sprintf(buf, "%s\n", "custom");
}

static ssize_t preset_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t count)
{
	int result = -EINVAL;
	int index;
	int c;

	if (streq(buf, "super_battery"))
		index = MSI_EC_PRESET_SUPER_BATTERY;
	else if (streq(buf, "silent"))
		index = MSI_EC_PRESET_SILENT;
	else if (streq(buf, "balanced"))
		index = MSI_EC_PRESET_BALANCED;
	else if (streq(buf, "high_performance"))
		index = MSI_EC_PRESET_HIGH_PERFORMANCE;
	else
		return result;

	for (c = 0; c < ARRAY_SIZE(MSI_EC_PRESET_MEMORY_TABLE); c++) {
		u8 addr = MSI_EC_PRESET_MEMORY_TABLE[c];
		u8 value = MSI_EC_PRESET_VALUE_TABLE[index][c];

		if(c == MSI_EC_PRESET_COLUMN_SILENT_FLAG) {
			result = ec_write_bit(addr,
					      MSI_EC_FAN_MODE_SILENT_BIT,
					      value);
		}
		else {
			result = ec_write(addr, value);
		}

		if(result < 0)
			pr_err("msi-ec: preset_store: failed to write to address %#02x "
				       "while setting preset %i (error code %i)",
				       addr, index, result);
	}

	/* ---- Validate fan modes ---- */
	if(index != MSI_EC_PRESET_HIGH_PERFORMANCE) {
		// Disable basic/adv fan mode flags when not using high performance preset
		 ec_write_bit(MSI_EC_FAN_MODE_ADDRESS,
			      MSI_EC_FAN_MODE_ADVANCED_BIT,
			      FALSE);

		 ec_write_bit(MSI_EC_FAN_MODE_ADDRESS,
			      MSI_EC_FAN_MODE_BASIC_BIT,
			      FALSE);
	}

	return count;
}

static ssize_t fw_version_show(struct device *device,
			       struct device_attribute *attr, char *buf)
{
	u8 rdata[MSI_EC_FW_VERSION_LENGTH + 1];
	int result;

	memset(rdata, 0, MSI_EC_FW_VERSION_LENGTH + 1);
	result = ec_read_seq(MSI_EC_FW_VERSION_ADDRESS, rdata,
			     MSI_EC_FW_VERSION_LENGTH);
	if (result < 0)
		return result;

	return sprintf(buf, "%s\n", rdata);
}

static ssize_t fw_release_date_show(struct device *device,
				    struct device_attribute *attr, char *buf)
{
	u8 rdate[MSI_EC_FW_DATE_LENGTH + 1];
	u8 rtime[MSI_EC_FW_TIME_LENGTH + 1];
	int result;
	int year, month, day, hour, minute, second;

	memset(rdate, 0, MSI_EC_FW_DATE_LENGTH + 1);
	result = ec_read_seq(MSI_EC_FW_DATE_ADDRESS, rdate,
			     MSI_EC_FW_DATE_LENGTH);
	if (result < 0)
		return result;
	sscanf(rdate, "%02d%02d%04d", &month, &day, &year);

	memset(rtime, 0, MSI_EC_FW_TIME_LENGTH + 1);
	result = ec_read_seq(MSI_EC_FW_TIME_ADDRESS, rtime,
			     MSI_EC_FW_TIME_LENGTH);
	if (result < 0)
		return result;
	sscanf(rtime, "%02d:%02d:%02d", &hour, &minute, &second);

	return sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d\n", year, month, day,
		       hour, minute, second);
}

static ssize_t ac_connected_show(struct device *device,
			     	 struct device_attribute *attr, char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_POWER_ADDRESS, &rdata);
	if (result < 0)
		return result;

	return sprintf(buf, "%i\n", is_bit_set(MSI_EC_POWER_AC_CONNECTED_BIT, rdata));
}

static ssize_t lid_open_show(struct device *device,
			     struct device_attribute *attr, char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_POWER_ADDRESS, &rdata);
	if (result < 0)
		return result;

	return sprintf(buf, "%i\n", is_bit_set(MSI_EC_POWER_LID_OPEN_BIT, rdata));
}

static DEVICE_ATTR_RW(webcam);
static DEVICE_ATTR_RW(fn_key);
static DEVICE_ATTR_RW(win_key);
static DEVICE_ATTR_RW(battery_charge_mode);
static DEVICE_ATTR_RW(cooler_boost);
static DEVICE_ATTR_RW(shift_mode);
static DEVICE_ATTR_RW(fan_mode);
static DEVICE_ATTR_RW(preset);
static DEVICE_ATTR_RO(fw_version);
static DEVICE_ATTR_RO(fw_release_date);
static DEVICE_ATTR_RO(ac_connected);
static DEVICE_ATTR_RO(lid_open);

static struct attribute *msi_root_attrs[] = {
	&dev_attr_webcam.attr,		&dev_attr_fn_key.attr,
	&dev_attr_win_key.attr,		&dev_attr_battery_charge_mode.attr,
	&dev_attr_cooler_boost.attr,	&dev_attr_shift_mode.attr,
	&dev_attr_fan_mode.attr,		&dev_attr_fw_version.attr,
	&dev_attr_ac_connected.attr,	&dev_attr_lid_open.attr,
	&dev_attr_fw_release_date.attr,	&dev_attr_preset.attr,
	NULL
};

static const struct attribute_group msi_root_group = {
	.attrs = msi_root_attrs,
};

// ============================================================ //
// Sysfs platform device attributes (cpu)
// ============================================================ //

static ssize_t cpu_realtime_temperature_show(struct device *device,
					     struct device_attribute *attr,
					     char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_CPU_REALTIME_TEMPERATURE_ADDRESS, &rdata);
	if (result < 0)
		return result;

	return sprintf(buf, "%i\n", rdata);
}

static ssize_t cpu_realtime_fan_speed_show(struct device *device,
					   struct device_attribute *attr,
					   char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_CPU_REALTIME_FAN_SPEED_ADDRESS, &rdata);
	if (result < 0)
		return result;

	if (rdata < MSI_EC_CPU_REALTIME_FAN_SPEED_BASE_MIN ||
	    rdata > MSI_EC_CPU_REALTIME_FAN_SPEED_BASE_MAX)
		return -EINVAL;

	return sprintf(buf, "%i\n",
		       100 * (rdata - MSI_EC_CPU_REALTIME_FAN_SPEED_BASE_MIN) /
			       (MSI_EC_CPU_REALTIME_FAN_SPEED_BASE_MAX -
				MSI_EC_CPU_REALTIME_FAN_SPEED_BASE_MIN));
}



static struct device_attribute dev_attr_cpu_realtime_temperature = {
	.attr = {
		.name = "realtime_temperature",
		.mode = 0444,
	},
	.show = cpu_realtime_temperature_show,
};

static struct device_attribute dev_attr_cpu_realtime_fan_speed = {
	.attr = {
		.name = "realtime_fan_speed",
		.mode = 0444,
	},
	.show = cpu_realtime_fan_speed_show,
};

static struct attribute *msi_cpu_attrs[] = {
	&dev_attr_cpu_realtime_temperature.attr,
	&dev_attr_cpu_realtime_fan_speed.attr,
	NULL,
};

static const struct attribute_group msi_cpu_group = {
	.name = "cpu",
	.attrs = msi_cpu_attrs,
};

// ============================================================ //
// Sysfs platform device attributes (gpu)
// ============================================================ //

static ssize_t gpu_realtime_temperature_show(struct device *device,
					     struct device_attribute *attr,
					     char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_GPU_REALTIME_TEMPERATURE_ADDRESS, &rdata);
	if (result < 0)
		return result;

	return sprintf(buf, "%i\n", rdata);
}

static ssize_t gpu_realtime_fan_speed_show(struct device *device,
					   struct device_attribute *attr,
					   char *buf)
{
	u8 rdata;
	int result;

	result = ec_read(MSI_EC_GPU_REALTIME_FAN_SPEED_ADDRESS, &rdata);
	if (result < 0)
		return result;

	return sprintf(buf, "%i\n", rdata);
}

static struct device_attribute dev_attr_gpu_realtime_temperature = {
	.attr = {
		.name = "realtime_temperature",
		.mode = 0444,
	},
	.show = gpu_realtime_temperature_show,
};

static struct device_attribute dev_attr_gpu_realtime_fan_speed = {
	.attr = {
		.name = "realtime_fan_speed",
		.mode = 0444,
	},
	.show = gpu_realtime_fan_speed_show,
};

static struct attribute *msi_gpu_attrs[] = {
	&dev_attr_gpu_realtime_temperature.attr,
	&dev_attr_gpu_realtime_fan_speed.attr,
	NULL,
};

static const struct attribute_group msi_gpu_group = {
	.name = "gpu",
	.attrs = msi_gpu_attrs,
};

static const struct attribute_group *msi_platform_groups[] = {
	&msi_root_group,
	&msi_cpu_group,
	&msi_gpu_group,
	NULL,
};

static int msi_platform_probe(struct platform_device *pdev)
{
	int result;
	result = sysfs_create_groups(&pdev->dev.kobj, msi_platform_groups);
	if (result < 0)
		return result;
	return 0;
}

static int msi_platform_remove(struct platform_device *pdev)
{
	sysfs_remove_groups(&pdev->dev.kobj, msi_platform_groups);
	return 0;
}

static struct platform_device *msi_platform_device;

static struct platform_driver msi_platform_driver = {
	.driver = {
		.name = MSI_DRIVER_NAME,
	},
	.probe = msi_platform_probe,
	.remove = msi_platform_remove,
};

// ============================================================ //
// Sysfs leds subsystem
// ============================================================ //

static int micmute_led_sysfs_set(struct led_classdev *led_cdev,
				 enum led_brightness brightness)
{
	int result = ec_write_bit(MSI_EC_KBD_LED_MICMUTE_ADDRESS,
				  MSI_EC_KBD_LED_MICMUTE_BIT,
				  brightness);
	if (result < 0)
		return result;
	return 0;
}

static int mute_led_sysfs_set(struct led_classdev *led_cdev,
			      enum led_brightness brightness)
{
	int result = ec_write_bit(MSI_EC_KBD_LED_MUTE_ADDRESS,
				  MSI_EC_KBD_LED_MUTE_BIT,
				  brightness);
	if (result < 0)
		return result;
	return 0;
}

static enum led_brightness kbd_bl_sysfs_get(struct led_classdev *led_cdev)
{
	u8 rdata;
	int result = ec_read(MSI_EC_KBD_BL_ADDRESS, &rdata);
	if (result < 0)
		return 0;
	return rdata & MSI_EC_KBD_BL_STATE_MASK;
}

static int kbd_bl_sysfs_set(struct led_classdev *led_cdev,
			    enum led_brightness brightness)
{
	u8 wdata;
	if (brightness > 3)
		return -1;
	wdata = MSI_EC_KBD_BL_STATE[brightness];
	return ec_write(MSI_EC_KBD_BL_ADDRESS, wdata);
}

static struct led_classdev micmute_led_cdev = {
	.name = "platform::micmute",
	.max_brightness = 1,
	.brightness_set_blocking = &micmute_led_sysfs_set,
	.default_trigger = "audio-micmute",
};

static struct led_classdev mute_led_cdev = {
	.name = "platform::mute",
	.max_brightness = 1,
	.brightness_set_blocking = &mute_led_sysfs_set,
	.default_trigger = "audio-mute",
};

static struct led_classdev msiacpi_led_kbdlight = {
	.name = "msiacpi::kbd_backlight",
	.max_brightness = 3,
	.flags = LED_BRIGHT_HW_CHANGED & LED_RETAIN_AT_SHUTDOWN,
	.brightness_set_blocking = &kbd_bl_sysfs_set,
	.brightness_get = &kbd_bl_sysfs_get,
};

// ============================================================ //
// Module load/unload
// ============================================================ //

static int __init msi_ec_init(void)
{
	int result;

	if (acpi_disabled) {
		pr_err("Unable to init because ACPI needs to be enabled first!\n");
		return -ENODEV;
	}

	result = platform_driver_register(&msi_platform_driver);
	if (result < 0) {
		return result;
	}

	msi_platform_device = platform_device_alloc(MSI_DRIVER_NAME, -1);
	if (msi_platform_device == NULL) {
		platform_driver_unregister(&msi_platform_driver);
		return -ENOMEM;
	}

	result = platform_device_add(msi_platform_device);
	if (result < 0) {
		platform_device_del(msi_platform_device);
		platform_driver_unregister(&msi_platform_driver);
		return result;
	}

	led_classdev_register(&msi_platform_device->dev, &micmute_led_cdev);
	led_classdev_register(&msi_platform_device->dev, &mute_led_cdev);
	led_classdev_register(&msi_platform_device->dev, &msiacpi_led_kbdlight);

	// Enable backlight by default, the kernel doesn't properly retain its state despite flag for some reason
	ec_write(MSI_EC_KBD_BL_ADDRESS, MSI_EC_KBD_BL_STATE[2]);

	pr_info("msi-ec: module_init\n");
	return 0;
}

static void __exit msi_ec_exit(void)
{
	led_classdev_unregister(&mute_led_cdev);
	led_classdev_unregister(&micmute_led_cdev);
	led_classdev_unregister(&msiacpi_led_kbdlight);

	platform_driver_unregister(&msi_platform_driver);
	platform_device_del(msi_platform_device);

	pr_info("msi-ec: module_exit\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jose Angel Pastrana <japp0005@red.ujaen.es>");
MODULE_AUTHOR("Aakash Singh <mail@singhaakash.dev>");
MODULE_AUTHOR("Tim Schneeberger <tim.schneeberger@outlook.de>");
MODULE_DESCRIPTION("MSI Embedded Controller");
MODULE_VERSION("0.09");

module_init(msi_ec_init);
module_exit(msi_ec_exit);

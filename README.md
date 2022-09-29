# Embedded Controller for MSI Modern laptops

Fork of msi-ec (https://github.com/BeardOverflow/msi-ec) with support for MSI Modern 15 A11M (Business series)

I have also created a [pattern script for ImHex](msi_modern_15_a11m_ec.hexpat). It contains annotations for all useful memory addresses discovered on the Modern 15.

![hex pattern screenshot](https://user-images.githubusercontent.com/38386967/193131500-8b0dd17d-0c7b-4eef-9316-289640bba345.png)



## Disclaimer

This driver might not work on other laptops produced by MSI. Use it at your own risk, I am not responsible for any damage suffered.

## Installation

1. Install the following packages:
- For Debian: `build-essential linux-headers-amd64`
- For Ubuntu: `build-essential linux-headers-generic`
2. Clone this repository and cd'ed
3. Run `make`
4. Run `make install`
5. (Optional) To uninstall, run `make uninstall`

## Usage

This driver exports a few files in its own platform device, msi-ec, and is available to userspace under:

- `/sys/devices/platform/msi-ec/preset`
  - Description: This entry allows setting a preset (also known as user scenario in MSI Center Pro).
  - Access: Read, Write
  - Valid values:
    - super_battery: Battery saving mode
    - silent: Prefer silent fans
    - balanced: Balanced power profile
    - high_performance: Best performance

- `/sys/devices/platform/msi-ec/webcam`
  - Description: This entry allows enabling the integrated webcam.
  - Access: Read, Write
  - Valid values:
    - on: integrated webcam is enabled
    - off: integrated webcam is disabled

- `/sys/devices/platform/msi-ec/fn_key`
  - Description: This entry allows switching the position between the function key and the windows key.
  - Access: Read, Write
  - Valid values:
    - left: function key goes to the left, windows key goes to the right
    - right: function key goes to the right, windows key goes to the left

- `/sys/devices/platform/msi-ec/win_key`
  - Description: This entry allows changing the position for the function key.
  - Access: Read, Write
  - Valid values:
    - left: windows key goes to the left, function key goes to the right
    - right: windows key goes to the right, function key goes to the left

- `/sys/devices/platform/msi-ec/battery_charge_mode`
  - Description: This entry allows changing the battery mode for health purposes.
  - Access: Read, Write
  - Valid values:
    - max: best for mobility. Charge the battery to 100% all the time
    - medium: balanced. Charge the battery when under 70%, stop at 80%
    - min: best for battery. Charge the battery when under 50%, stop at 60%

- `/sys/devices/platform/msi-ec/cooler_boost`
  - Description: This entry allows enabling the cooler boost function. It provides powerful cooling capability by boosting the airflow.
  - Access: Read, Write
  - Valid values:
    - on: cooler boost function is enabled
    - off: cooler boost function is disabled

- `/sys/devices/platform/msi-ec/shift_mode`
  - Description: This entry allows switching the shift mode.
  - Access: Read, Write
  - Valid values:
    - overclock: maximum clock frequency
    - balanced: dynamic clock frequency for the CPU & GPU, aka power balanced mode
    - eco: low clock frequency for the CPU & GPU, aka power saving mode
    - off: operating system decides

- `/sys/devices/platform/msi-ec/fan_mode`
  - Description: This entry allows switching the fan mode. It provides a set of profiles for adjusting the fan speed under specific criteria.
  - Access: Read, Write
  - Valid values:
    - auto: fan speed adjusts automatically
    - silent: fan speed remains as low as possible
    - advanced: fixed 6-levels fan speed for CPU/GPU (percent)

- `/sys/devices/platform/msi-ec/fw_version`
  - Description: This entry reports the firmware version of the motherboard.
  - Access: Read
  - Valid values: Represented as string

- `/sys/devices/platform/msi-ec/fw_release_date`
  - Description: This entry reports the firmware release date of the motherboard.
  - Access: Read
  - Valid values: Represented as string

- `/sys/devices/platform/msi-ec/ac_connected`
  - Description: This entry reports whether the power adapter is connected.
  - Access: Read
  - Valid values: 0 - 1
    - 0: Connected
    - 1: Not connected
    
- `/sys/devices/platform/msi-ec/lid_open`
  - Description: This entry reports whether the lid is opened.
  - Access: Read
  - Valid values: 0 - 1
    - 0: Closed
    - 1: Open

- `/sys/devices/platform/msi-ec/cpu/realtime_temperature`
  - Description: This entry reports the current cpu temperature.
  - Access: Read
  - Valid values: 0 - 100 (celsius scale)

- `/sys/devices/platform/msi-ec/cpu/realtime_fan_speed`
  - Description: This entry reports the current cpu fan speed.
  - Access: Read
  - Valid values: 0 - 150 (percent)

- `/sys/devices/platform/msi-ec/gpu/realtime_temperature`
  - Description: This entry reports the current gpu temperature.
  - Access: Read
  - Valid values: 0 - 100 (celsius scale)

- `/sys/devices/platform/msi-ec/gpu/realtime_fan_speed`
  - Description: This entry reports the current gpu fan speed.
  - Access: Read
  - Valid values: 0 - 150 (percent)

Led subsystem allows us to control the leds on the laptop including the keyboard backlight

- `/sys/class/leds/platform::<led_name>/brightness`
  - Description: sets the current state of the led.
  - Access: Read, Write
  - Valid values: 0 - 1
    - 0: Led off
    - 1: Led on

- `/sys/class/leds/msiacpi::kbd_backlight/brightness`
  - Description: sets the current state of keyboard backlight.
  - Access: Read, Write
  - Valid values: 0 - 3
    - 0: Off
    - 1: On
    - 2: Half
    - 1: Full


## List of tested laptops:

- MSI Modern 15 A11M (1552EMS1.118)

## Credits
 * [msi-ec](https://github.com/BeardOverflow/msi-ec)

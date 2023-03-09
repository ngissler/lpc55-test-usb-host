# Test USB host on the lpcxpresso55S69

Test MSD class using NXP usb host implementation on the lpcxpresso55S69 board using Zephyr RTOS.

I have created 2 implementations:

- RTOS-aware (from lpcxpresso55s69_host_msd_command_freertos)
*NOT WORKING*
- no RTOS-aware (from lpcxpresso55s69_host_msd_command_bm)
*WORKING*

Change the USE_USB_WITH_ZPHYR_RTOS in the CMakeLists.txt to compile the RTOS-aware implementations.

## Set-up and build

SDK: zephyr-sdk-0.15.2
Zephyr version: 3.3.0

	mkdir lpc55-test-usb-host-workspace
	cd lpc55-test-usb-host-workspace
	west init -m https://git.lacie.com/daspro/unit-tests/lpc55/zephyr/lpc55-test-usb-host
	west update
	west build lpc55-test-usb-host




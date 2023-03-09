zephyr_include_directories(.)

zephyr_library_sources(
  usb_host_devices.c
  usb_host_framework.c
  usb_host_hci.c
  usb_host_ip3516hs.c
  usb_host_ohci.c
  )

add_subdirectory(class)
message("Add NXP MCUX USB HOST")

SET(nxp_mcux_usb_host_INCLUDES "")
SET(nxp_mcux_usb_host_SRC "")

# usb header files from NXP HAL needed # Ugly because it knows where it is in the tree
get_filename_component(PARENT_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
get_filename_component(PARENT_DIR ${PARENT_DIR} DIRECTORY)
set(USB_INCLUDE_PATH ${PARENT_DIR}/modules/hal/nxp/mcux/middleware/mcux-sdk-middleware-usb/include)
message("USB_INCLUDE_PATH is ${USB_INCLUDE_PATH}")

SET(nxp_mcux_usb_host_INCLUDES 
${nxp_mcux_usb_host_INCLUDES}
${USB_INCLUDE_PATH}
${CMAKE_CURRENT_LIST_DIR}/usb_host_app/include
${CMAKE_CURRENT_LIST_DIR}/fsl_components/lists
${CMAKE_CURRENT_LIST_DIR}/fsl_components/osa
${CMAKE_CURRENT_LIST_DIR}/fsl_components/osa/unit_tests
${CMAKE_CURRENT_LIST_DIR}/usb/include
${CMAKE_CURRENT_LIST_DIR}/usb/host
${CMAKE_CURRENT_LIST_DIR}/usb/host/class
)

include_directories(${nxp_mcux_usb_host_INCLUDES})

IF (USE_USB_WITH_ZPHYR_RTOS)
SET(nxp_mcux_usb_host_SRC 
${CMAKE_CURRENT_LIST_DIR}/fsl_components/osa/fsl_os_abstraction_zephyr.c
)
ELSE()
SET(nxp_mcux_usb_host_SRC 
${CMAKE_CURRENT_LIST_DIR}/fsl_components/osa/fsl_os_abstraction_bm.c
)
ENDIF (USE_USB_WITH_ZPHYR_RTOS)

SET(nxp_mcux_usb_host_SRC 
${nxp_mcux_usb_host_SRC}
${CMAKE_CURRENT_LIST_DIR}/fsl_components/lists/fsl_component_generic_list.c
${CMAKE_CURRENT_LIST_DIR}/fsl_components/osa/unit_tests/test_osa.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/usb_host_devices.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/usb_host_ehci.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/usb_host_framework.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/usb_host_hci.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/usb_host_ip3516hs.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/usb_host_khci.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/usb_host_ohci.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/class/usb_host_hub.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/class/usb_host_hub_app.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/class/usb_host_msd.c
${CMAKE_CURRENT_LIST_DIR}/usb/host/class/usb_host_msd_ufi.c
${CMAKE_CURRENT_LIST_DIR}/usb_host_app/src/usb_host_app.c
)

#include "usb_host_config.h"
#include "usb_host.h"
#include "fsl_device_registers.h"
#include "usb_host_msd.h"
#include "host_msd_command.h"
#include "fsl_common.h"
#include "fsl_iocon.h"
#include "fsl_power.h"
#include "usb_phy.h"

#include "usb_host_app.h"

#if ((!USB_HOST_CONFIG_KHCI) && (!USB_HOST_CONFIG_EHCI) && (!USB_HOST_CONFIG_OHCI) && (!USB_HOST_CONFIG_IP3516HS))
#error Please enable USB_HOST_CONFIG_KHCI, USB_HOST_CONFIG_EHCI, USB_HOST_CONFIG_OHCI, or USB_HOST_CONFIG_IP3516HS in file usb_host_config.
#endif

#define IOCON_PIO_DIGITAL_EN 0x0100u  /*!<@brief Enables digital function */
#define IOCON_PIO_FUNC1 0x01u         /*!<@brief Selects pin function 1 */
#define IOCON_PIO_FUNC4 0x04u         /*!<@brief Selects pin function 4 */
#define IOCON_PIO_FUNC7 0x07u         /*!<@brief Selects pin function 7 */
#define IOCON_PIO_INV_DI 0x00u        /*!<@brief Input function is not inverted */
#define IOCON_PIO_MODE_INACT 0x00u    /*!<@brief No addition pin function */
#define IOCON_PIO_MODE_PULLUP 0x20u   /*!<@brief Selects pull-up function */
#define IOCON_PIO_OPENDRAIN_DI 0x00u  /*!<@brief Open drain is disabled */
#define IOCON_PIO_SLEW_STANDARD 0x00u /*!<@brief Standard mode, output slew rate control is enabled */

void usb_host_app_init_pins_usb0(void)
{
    /* Enables the clock for the I/O controller.: Enable Clock. */
    CLOCK_EnableClock(kCLOCK_Iocon);
    const uint32_t port1_pin12_config = (/* Pin is configured as USB0_PORTPWRN */
                                         IOCON_PIO_FUNC4 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT1 PIN12 (coords: 67) is configured as USB0_PORTPWRN */
    IOCON_PinMuxSet(IOCON, 1U, 12U, port1_pin12_config);
#if 1    
    const uint32_t port0_pin22_config = (/* Pin is configured as USB0_VBUS */
                                         IOCON_PIO_FUNC7 |
                                         /* No addition pin function */
                                         IOCON_PIO_MODE_INACT |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN22 (coords: 78) is configured as USB0_VBUS */
    IOCON_PinMuxSet(IOCON, 0U, 22U, port0_pin22_config);
#endif
#if 0
    const uint32_t port0_pin28_config = (/* Pin is configured as USB0_OVERCURRENTN */
                                         IOCON_PIO_FUNC7 |
                                         /* Selects pull-up function */
                                         IOCON_PIO_MODE_PULLUP |
                                         /* Standard mode, output slew rate control is enabled */
                                         IOCON_PIO_SLEW_STANDARD |
                                         /* Input function is not inverted */
                                         IOCON_PIO_INV_DI |
                                         /* Enables digital function */
                                         IOCON_PIO_DIGITAL_EN |
                                         /* Open drain is disabled */
                                         IOCON_PIO_OPENDRAIN_DI);
    /* PORT0 PIN28 (coords: 66) is configured as USB0_OVERCURRENTN */
    IOCON_PinMuxSet(IOCON, 0U, 28U, port0_pin28_config);
#endif
}

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*!
 * @brief host callback function.
 *
 * device attach/detach callback function.
 *
 * @param deviceHandle        device handle.
 * @param configurationHandle attached device's configuration descriptor information.
 * @param eventCode           callback event code, please reference to enumeration host_event_t.
 *
 * @retval kStatus_USB_Success              The host is initialized successfully.
 * @retval kStatus_USB_NotSupported         The application don't support the configuration.
 */
static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode);

/*!
 * @brief application initialization.
 */
static void USB_HostApplicationInit(void);

static void USB_HostTask(void *p1, void *p2, void *p3);

static void USB_HostApplicationTask(void *p1, void *p2, void *p3);

extern void USB_HostClockInit(void);
extern void USB_HostIsrEnable(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
extern usb_host_handle g_HostHandle;
/* Allocate the memory for the heap. */
#if defined(configAPPLICATION_ALLOCATED_HEAP) && (configAPPLICATION_ALLOCATED_HEAP)
USB_DMA_NONINIT_DATA_ALIGN(USB_DATA_ALIGN_SIZE) uint8_t ucHeap[configTOTAL_HEAP_SIZE];
#endif
/*! @brief USB host msd command instance global variable */
extern usb_host_msd_command_instance_t g_MsdCommandInstance;
usb_host_handle g_HostHandle;

/*******************************************************************************
 * Code
 ******************************************************************************/
void USB0_IRQHandler(void)
{
    USB_HostOhciIsrFunction(g_HostHandle);
}

void USB_HostClockInit(void)
{
    CLOCK_EnableUsbfs0HostClock(kCLOCK_UsbfsSrcPll1, 48000000U);
    for (int i = 0; i < (FSL_FEATURE_USBFSH_USB_RAM >> 2); i++)
    {
        ((uint32_t *)FSL_FEATURE_USBFSH_USB_RAM_BASE_ADDRESS)[i] = 0U;
    }
}

void USB_HostIsrEnable(void)
{
    IRQ_CONNECT(USB0_IRQn, 0, USB0_IRQHandler, 0, 0);
    //IRQ_DIRECT_CONNECT(USB0_IRQn, 0, USB0_IRQHandler, 0);
    irq_enable(USB0_IRQn); 
}



/*!
 * @brief USB isr function.
 */

static usb_status_t USB_HostEvent(usb_device_handle deviceHandle,
                                  usb_host_configuration_handle configurationHandle,
                                  uint32_t eventCode)
{
    usb_status_t status = kStatus_USB_Success;
    switch (eventCode & 0x0000FFFFU)
    {
        case kUSB_HostEventAttach:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventNotSupported:
            usb_echo("device not supported.\r\n");
            break;

        case kUSB_HostEventEnumerationDone:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventDetach:
            status = USB_HostMsdEvent(deviceHandle, configurationHandle, eventCode);
            break;

        case kUSB_HostEventEnumerationFail:
            usb_echo("enumeration failed\r\n");
            break;

        default:
            break;
    }
    return status;
}

static void USB_HostApplicationInit(void)
{
    usb_status_t status = kStatus_USB_Success;

    USB_HostClockInit();

    status = USB_HostInit(CONTROLLER_ID, &g_HostHandle, USB_HostEvent);
    if (status != kStatus_USB_Success)
    {
        usb_echo("host init error\r\n");
        return;
    }
    USB_HostIsrEnable();

    usb_echo("host init done\r\n");
}

void USB_HostTaskFn(void *p1, void *p2, void *p3)
{    
    USB_HostOhciTaskFunction(p1);

}


static void USB_HostTask(void *p1, void *p2, void *p3)
{
    while (1)
    {
        USB_HostTaskFn(p1, NULL, NULL);
    }
}


static void USB_HostApplicationTask(void *p1, void *p2, void *p3)
{
    while (1)
    {
        USB_HostMsdTask(p1);
    }
}

static void USB_HostAllTask(void *p1, void *p2, void *p3)
{
    while (1)
    {
        USB_HostTaskFn(g_HostHandle, NULL, NULL);
        USB_HostMsdTask(&g_MsdCommandInstance);
    }
}

//==========================
#if defined(FSL_RTOS_ZEPHYR)
// USB_HOST task has a higher priority than USB_HOST_APP
// In zephyr; higher priority = lower number
#define THREAD_USB_HOST_SIZE (4096)
#define THREAD_USB_HOST_PRIORITY 0
K_THREAD_STACK_DEFINE(thread_usb_host_stack_area, THREAD_USB_HOST_SIZE);
struct k_thread thread_usb_host;

#define THREAD_USB_HOST_APP_SIZE (4096)
#define THREAD_USB_HOST_APP_PRIORITY 4
K_THREAD_STACK_DEFINE(thread_usb_host_app_stack_area, THREAD_USB_HOST_APP_SIZE);
struct k_thread thread_usb_host_app;
#else
// USB_HOST task has a higher priority than USB_HOST_APP
// In zephyr; higher priority = lower number
#define THREAD_USB_HOST_SIZE (4096)
#define THREAD_USB_HOST_PRIORITY 3
K_THREAD_STACK_DEFINE(thread_usb_host_stack_area, THREAD_USB_HOST_SIZE);
struct k_thread thread_usb_host;
#endif

void usb_host_app_init(void)
{
    /* set BOD VBAT level to 1.65V */
    POWER_SetBodVbatLevel(kPOWER_BodVbatLevel1650mv, kPOWER_BodHystLevel50mv, false);

    usb_host_app_init_pins_usb0();

    NVIC_ClearPendingIRQ(USB0_IRQn);
    NVIC_ClearPendingIRQ(USB0_NEEDCLK_IRQn);
    NVIC_ClearPendingIRQ(USB1_IRQn);
    NVIC_ClearPendingIRQ(USB1_NEEDCLK_IRQn);

    POWER_DisablePD(kPDRUNCFG_PD_USB0_PHY); /*< Turn on USB0 Phy */
    POWER_DisablePD(kPDRUNCFG_PD_USB1_PHY); /*< Turn on USB1 Phy */

    /* reset the IP to make sure it's in reset state. */
    RESET_PeripheralReset(kUSB0D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HSL_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB0HMR_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1H_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1D_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1_RST_SHIFT_RSTn);
    RESET_PeripheralReset(kUSB1RAM_RST_SHIFT_RSTn);

    USB_HostApplicationInit();

#if defined(FSL_RTOS_ZEPHYR)
    k_tid_t usb_host_tid = k_thread_create(&thread_usb_host, thread_usb_host_stack_area,
                                     K_THREAD_STACK_SIZEOF(thread_usb_host_stack_area),
                                     USB_HostTask,
                                     g_HostHandle, NULL, NULL,
                                     THREAD_USB_HOST_PRIORITY, 0, K_NO_WAIT);    

    k_tid_t usb_host_app_tid = k_thread_create(&thread_usb_host_app, thread_usb_host_app_stack_area,
                                     K_THREAD_STACK_SIZEOF(thread_usb_host_app_stack_area),
                                     USB_HostApplicationTask,
                                     &g_MsdCommandInstance, NULL, NULL,
                                     THREAD_USB_HOST_APP_PRIORITY, 0, K_NO_WAIT);  


#else
        k_tid_t usb_host_tid = k_thread_create(&thread_usb_host, thread_usb_host_stack_area,
                                     K_THREAD_STACK_SIZEOF(thread_usb_host_stack_area),
                                     USB_HostAllTask,
                                     NULL, NULL, NULL,
                                     THREAD_USB_HOST_PRIORITY, 0, K_NO_WAIT);     
#endif  
}


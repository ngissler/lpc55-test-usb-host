#include <zephyr/device.h>
#include <errno.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <assert.h>
#include <zephyr/drivers/gpio.h>

#include <stdlib.h>
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(test_osa);

#include "fsl_os_abstraction.h"
#include "fsl_power.h"

//*************************************************
// Test OSA EVENT
//*************************************************
typedef struct _usb_host_ip3516hs_state_struct
{
    osa_event_handle_t ip3516HsEvent;
    uint32_t taskEventHandleBuffer[(OSA_EVENT_HANDLE_SIZE + 3) / 4];
} usb_host_ip3516hs_state_struct_t;
#define USB_HOST_IP3516HS_EVENT_PORT_CHANGE (0x04U)
#define USB_HOST_IP3516HS_EVENT_ISO_TOKEN_DONE (0x08U)

#define THREAD_TEST_EVENT_SIZE (4096)
#define THREAD_TEST_EVENT_PRIORITY 3
K_THREAD_STACK_DEFINE(thread_test_event_stack_area, THREAD_TEST_EVENT_SIZE);
struct k_thread thread_test_event;

 usb_host_ip3516hs_state_struct_t *usbHostState = NULL;

void consumer_thread_test_event(void *, void *, void *)
{
    printk("thread_test_event running\n");
    osa_status_t osaStatus;
    
    if (!usbHostState)
        k_panic();

    printk("waiting event\n");
    while (1) {
        osaStatus = OSA_EventWait(usbHostState->ip3516HsEvent, USB_HOST_IP3516HS_EVENT_PORT_CHANGE, false, 0xffffffff, NULL);
        if (osaStatus != KOSA_StatusSuccess) {
            printk("timeout\n");
        } else {
            printk("Got Event !!!\n");
        }
    }
}

int test_event(void)
{
    int ret = 0;
    printk("-->test_event\n");

    usbHostState = (usb_host_ip3516hs_state_struct_t *)OSA_MemoryAllocate(
        sizeof(usb_host_ip3516hs_state_struct_t));
    if (usbHostState == NULL)
        k_panic();

    usbHostState->ip3516HsEvent = (osa_event_handle_t)&usbHostState->taskEventHandleBuffer[0];
    if (KOSA_StatusSuccess != OSA_EventCreate(usbHostState->ip3516HsEvent, 1U))
        k_panic();

    printk("usbHostState->ip3516HsEvent is %p\n", usbHostState->ip3516HsEvent);

    printk("Launching consumer thread thread_test_event\n");
    k_tid_t my_tid = k_thread_create(&thread_test_event, thread_test_event_stack_area,
                                     K_THREAD_STACK_SIZEOF(thread_test_event_stack_area),
                                     consumer_thread_test_event,
                                     NULL, NULL, NULL,
                                     THREAD_TEST_EVENT_PRIORITY, 0, K_NO_WAIT);


    k_sleep(K_MSEC(1000));
    printk("posting event USB_HOST_IP3516HS_EVENT_PORT_CHANGE\n");
    OSA_EventSet(usbHostState->ip3516HsEvent, USB_HOST_IP3516HS_EVENT_PORT_CHANGE);
    k_sleep(K_MSEC(1000));
    printk("posting event USB_HOST_IP3516HS_EVENT_ISO_TOKEN_DONE\n");
    OSA_EventSet(usbHostState->ip3516HsEvent, USB_HOST_IP3516HS_EVENT_ISO_TOKEN_DONE);
    k_sleep(K_MSEC(1000));
    printk("posting event USB_HOST_IP3516HS_EVENT_PORT_CHANGE\n");
    OSA_EventSet(usbHostState->ip3516HsEvent, USB_HOST_IP3516HS_EVENT_PORT_CHANGE);

    if (usbHostState)
        free(usbHostState);
    printk("<--test_event %s\n", (!ret) ? "SUCESS":"FAILED");   
    return ret; 
}

//*************************************************
// Test OSA MUTEX
//*************************************************
typedef struct _usb_host_ehci_instance
{
    osa_mutex_handle_t ehciMutex;              /*!< EHCI mutex*/
    uint32_t mutexBuffer[(OSA_MUTEX_HANDLE_SIZE + 3) / 4];           /*!< The mutex buffer. */
} usb_host_ehci_instance_t;

int test_OSA_mutex(void)
{
    printk("-->test_OSA_mutex\n");
    osa_status_t osaStatus;
    usb_host_ehci_instance_t *ehciInstance; 
    ehciInstance      = (usb_host_ehci_instance_t *)OSA_MemoryAllocate(
        sizeof(usb_host_ehci_instance_t)); /* malloc host ehci instance */
    if (ehciInstance == NULL)
        k_panic();

    ehciInstance->ehciMutex = (osa_mutex_handle_t)(&ehciInstance->mutexBuffer[0]);
    osaStatus               = OSA_MutexCreate(ehciInstance->ehciMutex);
    if (osaStatus != KOSA_StatusSuccess)
        k_panic();    
    osaStatus               = OSA_MutexLock(ehciInstance->ehciMutex, osaWaitForever_c);
    if (osaStatus != KOSA_StatusSuccess)
        k_panic();  
    osaStatus               = OSA_MutexUnlock(ehciInstance->ehciMutex);
    if (osaStatus != KOSA_StatusSuccess)
        k_panic();   
    osaStatus               = OSA_MutexDestroy(ehciInstance->ehciMutex);
    if (osaStatus != KOSA_StatusSuccess)
        k_panic();  
    printk("<--test_OSA_mutex SUCCESS\n");    
    return 0;              
}

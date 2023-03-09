#include <zephyr/device.h>
#include <errno.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <assert.h>
#include <zephyr/drivers/gpio.h>

#include <stdlib.h>
#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main);

// Needed so usb_host functions got what is in usb_host_config.h. Ugly
#include "usb_host_config.h"
#include "usb_host.h"

#include "host_msd_command.h"

#include "usb_host_app.h"

#include "test_osa.h"

int main(void)
{
    printk("Starting main\n");
    usb_echo("usb_echo is working\n");

    usb_host_app_init();
    //test_event();
    //test_OSA_mutex();
    while (1) {
        k_sleep(K_MSEC(1000));
    }

    k_panic();
    return 0;
}

#define MAX_STRING_LENGTH 96 // dont put too much, it stresses the stack

int DbgConsole_Printf(const char *fmt_s, ...)
{
    int ret;
    char str[MAX_STRING_LENGTH];
    va_list vl;    
    va_start(vl, fmt_s);
    ret = vsnprintk(str, MAX_STRING_LENGTH - 1, fmt_s, vl);
    printk("%s", str);
    va_end(vl);
    return ret;
}


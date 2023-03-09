/*! *********************************************************************************
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017, 2019 NXP
 * All rights reserved.
 *
 *
 * This is the source file for the OS Abstraction layer for zephyr.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 ********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "fsl_common.h"
#include "fsl_os_abstraction.h"
#include "fsl_os_abstraction_zephyr.h"
#include <string.h>
#include "fsl_component_generic_list.h"

/*! *********************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
********************************************************************************** */

/* Weak function. */
#if defined(__GNUC__)
#define __WEAK_FUNC __attribute__((weak))
#elif defined(__ICCARM__)
#define __WEAK_FUNC __weak
#elif defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define __WEAK_FUNC __attribute__((weak))
#endif

#define millisecToTicks(millisec) (((millisec)*sys_clock_hw_cycles_per_sec() + 999U) / 1000U)


/*! @brief Converts milliseconds to ticks*/
#define MSEC_TO_TICK(msec) \
    (((uint32_t)(msec) + 500uL / (uint32_t)sys_clock_hw_cycles_per_sec()) * (uint32_t)sys_clock_hw_cycles_per_sec() / 1000uL)
#define TICKS_TO_MSEC(tick) ((uint32_t)((uint64_t)(tick)*1000uL / (uint64_t)sys_clock_hw_cycles_per_sec()))
/************************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
typedef struct osa_zephyr_task
{
    list_element_t link;
//    TaskHandle_t taskHandle;
} osa_zephyr_task_t;

typedef struct _osa_event_struct
{
//    EventGroupHandle_t handle; /* The event handle */
    uint8_t autoClear;         /*!< Auto clear or manual clear   */
} osa_event_struct_t;

/*! @brief State structure for bm osa manager. */
typedef struct _osa_state
{
#if (defined(FSL_OSA_TASK_ENABLE) && (FSL_OSA_TASK_ENABLE > 0U))
    list_label_t taskList;
#if (defined(FSL_OSA_MAIN_FUNC_ENABLE) && (FSL_OSA_MAIN_FUNC_ENABLE > 0U))
    OSA_TASK_HANDLE_DEFINE(mainTaskHandle);
#endif
#endif
    uint32_t basePriority;
    int32_t basePriorityNesting;
    uint32_t interruptDisableCount;
} osa_state_t;

/*! *********************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
********************************************************************************** */
const uint8_t gUseRtos_c = USE_RTOS; /* USE_RTOS = 0 for BareMetal and 1 for OS */

//static osa_state_t s_osaState = {0};
/*! *********************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
********************************************************************************** */
/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_MemoryAllocate
 * Description   : Reserves the requested amount of memory in bytes.
 *
 *END**************************************************************************/
void *OSA_MemoryAllocate(uint32_t length)
{
    void *p = (void *)k_malloc(length);
    if (!p)
        k_panic();

    if (NULL != p)
    {
        (void)memset(p, 0, length);
    }

    return p;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_MemoryFree
 * Description   : Frees the memory previously reserved.
 *
 *END**************************************************************************/
void OSA_MemoryFree(void *p)
{
    k_free(p);
}


void OSA_EnterCritical(uint32_t *sr)
{
    //*sr = irq_lock();
    *sr = DisableGlobalIRQ();   
}

void OSA_ExitCritical(uint32_t sr)
{
    //irq_unlock(sr);
    EnableGlobalIRQ(sr);
}

#warning "Avoid OSA task implementation since we do not use them"


/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_TimeGetMsec
 * Description   : This function gets current time in milliseconds.
 *
 *END**************************************************************************/
uint32_t OSA_TimeGetMsec(void)
{
    return k_uptime_get_32();
}

// The mutex is a pointer to the actual mutex

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_MutexCreate
 * Description   : This function is used to create a mutex.
 * Return        : Mutex handle of the new mutex, or NULL if failed.
 *
 *END**************************************************************************/
// mutexHandle pointer to struct k_mutex
osa_status_t OSA_MutexCreate(osa_mutex_handle_t mutexHandle)
{
    int ret; 
    if (sizeof(osa_mutex_handle_t) != sizeof(struct k_mutex *)) 
        k_panic();

    uint32_t *ptr = (uint32_t *) mutexHandle;    
    if (!ptr) 
        k_panic();

    struct k_mutex *tex = (struct k_mutex *) malloc(sizeof(struct k_mutex));
    if (!tex)
        k_panic();     

    ret = k_mutex_init(tex);
    if (ret)
        k_panic();

    *ptr = (uint32_t) tex;    // this is beyond ugly

    return KOSA_StatusSuccess;
}
#define EVENT_TO_MUTEX() uint32_t *ptr = (uint32_t *) mutexHandle; \
                         assert(ptr); \
                         struct k_mutex *tex = (struct k_mutex *) *ptr;

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_MutexLock
 * Description   : This function checks the mutex's status, if it is unlocked,
 * lock it and returns KOSA_StatusSuccess, otherwise, wait for the mutex.
 * This function returns KOSA_StatusSuccess if the mutex is obtained, returns
 * KOSA_StatusError if any errors occur during waiting. If the mutex has been
 * locked, pass 0 as timeout will return KOSA_StatusTimeout immediately.
 *
 *END**************************************************************************/
osa_status_t OSA_MutexLock(osa_mutex_handle_t mutexHandle, uint32_t millisec)
{
    int ret;
    k_timeout_t time_out;

    switch (millisec) {
    case osaWaitForever_c:
        time_out = K_FOREVER;
        break;
    case 0:
        time_out = K_NO_WAIT;
        break;
    default:
        time_out = K_MSEC(millisec);
        break;
    }

    EVENT_TO_MUTEX();

    ret = k_mutex_lock(tex, time_out);
    if (ret) // Error
    {
        return KOSA_StatusTimeout; /* timeout */
    }
    return KOSA_StatusSuccess; /* semaphore taken */
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_MutexUnlock
 * Description   : This function is used to unlock a mutex.
 *
 *END**************************************************************************/
osa_status_t OSA_MutexUnlock(osa_mutex_handle_t mutexHandle)
{
    int ret;

    EVENT_TO_MUTEX();
    ret = k_mutex_unlock(tex);

    if(!ret)
        return KOSA_StatusSuccess;
    return KOSA_StatusError;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_MutexDestroy
 * Description   : This function is used to destroy a mutex.
 * Return        : KOSA_StatusSuccess if the lock object is destroyed successfully, otherwise return KOSA_StatusError.
 *
 *END**************************************************************************/
osa_status_t OSA_MutexDestroy(osa_mutex_handle_t mutexHandle)
{
    EVENT_TO_MUTEX();
    free(tex);
    tex = NULL;
    *ptr = 0;

    return KOSA_StatusSuccess;
}


/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_EventCreate
 * Description   : This function is used to create a event object.
 * Return        : Event handle of the new event, or NULL if failed.
 *
 *END**************************************************************************/
#warning autoclear is not handle by Zephyr !!!! autoclear is true by default
osa_status_t OSA_EventCreate(osa_event_handle_t eventHandle, uint8_t autoClear)
{
    if (!autoClear)
        k_panic();
    
    if (sizeof(osa_event_handle_t) != sizeof(struct k_event *)) 
        k_panic();

    uint32_t *ptr = (uint32_t *) eventHandle;    
    if (!ptr) 
        k_panic();

    struct k_event *evt = (struct k_event *) malloc(sizeof(struct k_event));
    if (!evt)
        k_panic(); 

    k_event_init(evt);

    *ptr = (uint32_t) evt;    // this is beyond ugly
    return KOSA_StatusSuccess;
}

#define EVENT_TO_EV()  uint32_t *ptr = (uint32_t *) eventHandle;     \
                       assert(ptr);  \
                        struct k_event *ev = ( struct k_event *) *ptr;

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_EventDestroy
 * Description   : This function is used to destroy a event object. Return
 * KOSA_StatusSuccess if the event object is destroyed successfully, otherwise
 * return KOSA_StatusError.
 *
 *END**************************************************************************/
osa_status_t OSA_EventDestroy(osa_event_handle_t eventHandle)
{
    EVENT_TO_EV();
    free(ev);
    ev = NULL;
    *ptr = 0;
    return KOSA_StatusSuccess;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_EventSet
 * Description   : Set one or more event flags of an event object.
 * Return        : KOSA_StatusSuccess if set successfully, KOSA_StatusError if failed.
 *
 *END**************************************************************************/
osa_status_t OSA_EventSet(osa_event_handle_t eventHandle, osa_event_flags_t flagsToSet)
{
    EVENT_TO_EV();
    k_event_set(ev, flagsToSet);
    return KOSA_StatusSuccess;
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_EventClear
 * Description   : Clear one or more event flags of an event object.
 * Return        :KOSA_StatusSuccess if clear successfully, KOSA_StatusError if failed.
 *
 *END**************************************************************************/
osa_status_t OSA_EventClear(osa_event_handle_t eventHandle, osa_event_flags_t flagsToClear)
{
    EVENT_TO_EV();    
    k_event_clear(ev, flagsToClear);
    return KOSA_StatusSuccess;    
}

/*FUNCTION**********************************************************************
 *
 * Function Name : OSA_EventWait
 * Description   : This function checks the event's status, if it meets the wait
 * condition, return KOSA_StatusSuccess, otherwise, timeout will be used for
 * wait. The parameter timeout indicates how long should wait in milliseconds.
 * Pass osaWaitForever_c to wait indefinitely, pass 0 will return the value
 * KOSA_StatusTimeout immediately if wait condition is not met. The event flags
 * will be cleared if the event is auto clear mode. Flags that wakeup waiting
 * task could be obtained from the parameter setFlags.
 * This function returns KOSA_StatusSuccess if wait condition is met, returns
 * KOSA_StatusTimeout if wait condition is not met within the specified
 * 'timeout', returns KOSA_StatusError if any errors occur during waiting.
 *
 *END**************************************************************************/
osa_status_t OSA_EventWait(osa_event_handle_t eventHandle,
                           osa_event_flags_t flagsToWait,
                           uint8_t waitAll,
                           uint32_t millisec,
                           osa_event_flags_t *pSetFlags)
{
    uint32_t ret;
    uint32_t *ptr = (uint32_t *) eventHandle;
    assert(ptr);
    uint32_t sr;

    OSA_EnterCritical(&sr);
    struct k_event *ev = ( struct k_event *) *ptr;

    k_timeout_t time_out;

    switch (millisec) {
    case osaWaitForever_c:
        time_out = K_FOREVER;
        break;
    case 0:
        time_out = K_NO_WAIT;
        break;
    default:
        time_out = K_MSEC(millisec);
        break;
    }

    if ((bool)waitAll)
        ret = k_event_wait_all(ev, flagsToWait, true, time_out);
    else
        ret = k_event_wait(ev, flagsToWait, true, time_out);    

    if (pSetFlags)
        *pSetFlags = ret;

    OSA_ExitCritical(sr);
    if (!ret)
        return KOSA_StatusTimeout;
    //k_event_clear(ev, ret);
    return KOSA_StatusSuccess; 
}
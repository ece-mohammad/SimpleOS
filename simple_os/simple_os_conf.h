/*******************************************************************************
 * @file    simple_os_conf.h
 * @brief   Simple OS configuration file
 * @details Simple OS configuration file
 * @date    5 Oct. 2022
 * @author  Mohammad Mohsen
 ******************************************************************************/

#ifndef SIMPLE_OS_CONF_H_
#define SIMPLE_OS_CONF_H_

#include "board_config.h"

/* ------------------------------------------------------------------------- */

/**
 * @addtogroup  simple_os_configurations Simple OS configurations
 * @{
 * */

/**
 * Simple OS tick rate in Hz (how many times the OS_vidUpdate() is called per second)
 * */
#ifdef CONF_OS_TICK_RATE_HZ
#define OS_TICK_RATE_HZ             CONF_OS_TICK_RATE_HZ
#else
#define OS_TICK_RATE_HZ             1000u
#endif /*  CONF_OS_TICK_RATE_MS  */

/**
 * Simple OS maximum number of tasks. It also defines the maximum number of task priorities,
 * ranging from [0 : OS_TASK_COUNT - 1]
 * */
#ifdef CONF_OS_TASK_COUNT
#define OS_TASK_COUNT               CONF_OS_TASK_COUNT
#else
#define OS_TASK_COUNT             4u
#endif /*  CONF_OS_TICK_RATE_MS  */

/**
 * @brief Enable using 16-bit ticks (instead of 32-bit ticks)
 * @details Using 32-bit ticks is the default, but using 16-bit ticks will reduce task context size by 4 bytes/task.
 *
 * | **tick size** | **task context size** | **padding** | **total** |
 * |:-------------:|:---------------------:|:-----------:|:---------:|
 * |       16      |      3.5 * 4 = 14     |      2      |     16    |
 * |       32      |      4.5 * 4 = 18     |      2      |     20    |
 *
 * */
#ifdef CONF_OS_USE_16BIT_TICK
#define OS_USE_16BIT_TICK      CONF_OS_USE_16BIT_TICK
#else
#define OS_USE_16BIT_TICK
#endif  /*  CONF_OS_USE_16BIT_TICK  */

/**@}*/

#endif /* SIMPLE_OS_CONF_H_ */

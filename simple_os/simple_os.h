/*******************************************************************************
 * @file    simple_os.h
 * @brief   Simple co-operative operating system
 * @details A simple co-operative operating system.
 *          Simple OS only contains a round robin, priority based task scheduler,
 *          and provides a simple minimal API of 5 functions. Simple OS
 *          doesn't implement tick generation, but it provides a function to
 *          update task states. This allows Simple OS implementation to be
 *          simple and can be easily integrated into different environments.
 * @date    5 Oct. 2022
 * @author  Mohammad Mohsen
 ******************************************************************************/

#ifndef __SIMPLE_OS_H__
#define __SIMPLE_OS_H__

#include "simple_os_conf.h"

/* ------------------------------------------------------------------------- */

/**
 * @addtogroup  simple_os Simple OS
 * @brief   Simple Co-operative operating system
 * @{
 * */

/**
 * @brief Simple OStick typedef.
 * If #OS_USE_16BIT_TICK is enabled, OS_Tick_t is uint16_t and limited to values [0: 65535],
 * but will save 4 bytes/task in the RAM.
 * */
#ifdef OS_USE_16BIT_TICK
typedef uint16_t OS_Tick_t;
#else
typedef uint32_t OS_Tick_t;
#endif /*  OS_USE_16BIT_TICK  */


/**
 * @brief A macro to convert from milliseconds to OS ticks
 * */
#define OS_MS_TO_TICKS(ms)      ((OS_Tick_t)((OS_TICK_RATE_HZ * (ms)) / 1000u))


/**
 * @brief Simple OS error codes
 * */
typedef enum os_error_t {
    OS_ERROR_NONE,              /**<  No error, function execution was successful  */
    OS_ERROR_NULLPTR,           /**<  Null pointer error, function execution failed due to an unexpected NULL pointer  */
    OS_ERROR_INVALID_PARAM,     /**<  Invalid param, function execution failed due to an invalid parameter value  */
}OS_Error_t;

/**
 * @brief Simple OS task handle
 * */
typedef void * OS_TaskHandle_t;

/**
 * @brief Simple OS task's handler, a pointer to a function that is executed when the task is ready
 *
 * @param [in] pvArgs : pointer to function parameter, passed to function at execution
 *
 * @return void
 * */
typedef void (* OS_vidTaskHandler_t)(void * const pvArgs);


/* ------------------------------------------------------------------------- */

/**
 * @brief Initialize simple OS
 *
 * @param void
 *
 * @pre OS is configured in simple_os_config.h
 *
 * @post Tasks can be added using OS_enAddTask() and
 *
 * @return void
 * */
void OS_vidInitialize(void);

/**
 * @brief Add task to OS's task list
 *
 * @pre OS is initialized using OS_vidInitialize()
 *
 * @post Task is added to the task list
 *
 * @param [in]  pvHandler   : pointer to task's function `[void, (void *)]`. Will be called when the function is ready to be executed.
 * @param [in]  pvArgs      : pointer to task's function argument, passed to the task's function at execution
 * @param [in]  u32Priority : task's priority, must be in range `[0: OS_TASK_COUNT - 1]`.
 *                            Lower value = higher priority, `0` is the highest priority,
 *                            while `OS_TASK _COUNT - 1` is the lowest priority.
 *                            If multiple tasks are ready to be executed at the same tick,
 *                            the task with the highest priority is executed first.
 * @param [in]  u32Period   : task's period in OS ticks, number of ticks between each call to the task's function.
 *                            \f(u32Period > 0 \f)> for periodic tasks. For one-shot tasks, \f( u32Period == 0 \f).
 * @param [in]  u32Delay    : task's first call delay, how many ticks to wait before calling the task's function for the first time.
 * @param [out] pTaskHandle : pointer to a task handle variable, used to save the task's handle.
 *                            Task handle is used with functions like OS_enDeleteTask(), OS_enSuspendTask(), OS_enResumeTask()
 *
 * @return #OS_Error_t
 *              OS_ERROR_NONE           : Task was added successfully to the task list
 *              OS_ERROR_NULLPTR        : Null error, task was not added to the task list because an unexpected NULL pointer
 *              OS_ERROR_INVALID_PARAM  : Invalid parameter error, task was not added to the task list because on or more parameters had a wrong value
 *
 * */
OS_Error_t OS_enAddTask(OS_vidTaskHandler_t pvHandler, void * const pvArgs, uint32_t u32Priority, OS_Tick_t u32Period, OS_Tick_t u32Delay, OS_TaskHandle_t * pTasKHandle);

/**
 * @brief Delete a task from the scheduler
 *
 * @param [in] xTaskHandle : Task handle (got from OS_enAddTask()) to delete
 *
 * @pre Task is added to scheduler using OS_enAddTask()
 *
 * @post Task is removed from task list
 *
 * @return #OS_Error_t
 *              OS_ERROR_NONE           : Task was added successfully to the scheduler
 *              OS_ERROR_INVALID_PARAM  : Invalid parameter error, task was not suspended because it was not found
 *
 * */
OS_Error_t OS_enDeleteTask(OS_TaskHandle_t xTasKHandle);

/**
 * @brief Update OS's task list. Must be called at #OS_TICK_RATE_HZ frequency
 *
 * @param void
 *
 * @pre OS is initialized using OS_vidInitialize()
 *
 * @post Added tasks (if any) are updated
 *
 * @return void
 *
 **/
void OS_vidUpdateTasks(void);

/**
 * @brief Dispatch OS's tasks that are ready to run
 *
 * @param void
 *
 * @pre OS is initialized using OS_vidInitialize()
 *
 * @pre OS_vidUpdate() is called at regular rate ( #OS_TICK_RATE_HZ )
 *
 * @post Ready tasks are executed
 *
 * @return void
 *
 **/
void OS_vidDispatchTasks(void);

uint32_t OS_enMillisecondsToTicks(uint32_t u32Millisecnds);

OS_Error_t OS_enSetTickRate(uint32_t u32TickRate);


/**@}*/

#endif /*  __SIMPLE_OS_H__  */

/*******************************************************************************
 * @file    simple_os.c
 * @brief   Simple OS Co-operative operating system
 * @details Simple Co-operative operating system
 * @date    5 Oct. 2022
 * @author  Mohammad Mohsen
 ******************************************************************************/

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <main.h>

#include "utils/utils.h"

#include "simple_os.h"

/* ------------------------------------------------------------------------- */

/**
 * @brief Simple OS flags, represent scheduker's state
 * */
typedef enum os_flag_t {
    OS_FLAG_NONE  = 0x00,           /**<  No flags  */
    OS_FLAG_DISPATCH_RDY = 0x02,    /**<  Scheduler is ready to execute some tasks.
                                          Task list was updated, and there may be some tasks ready to be executed  */
}OS_Flag_t;

/**
 * @brief Task flags, used to set the number of jobs a task has, or if the task is a single shot task.
 * Number of jobs is how many times the task will be executed.
 * */
typedef enum os_task_flags_t {
    OS_TASK_FLAG_NONE       = 0x00,     /**<  No flags  */
    OS_TASK_FLAG_MAX_JOBS   = 0x80 - 1, /**<  Maximum number of jobs a single task can has  */
    OS_TASK_FLAG_ONESHOT    = 0x80,     /**<  Task is a single shot task (execute only once then delete)  */
} OS_Task_Flag_t;

/**
 * @brief Task's structure, describe's OS task parameters
 * */
typedef struct os_task_struct_t {
        OS_vidTaskHandler_t handler;    /**<  Task's function, that is called when the function is ready to be executed  */
        void *    args;                 /**<  Pointer to an argument that is passed to the function handler  */
        OS_Tick_t delay;                /**<  Task's delay, ticks before the task is executed for the first time   */
        OS_Tick_t period;               /**<  Task's period, OS ticks between each time the task is executed  */
        uint8_t   priority;             /**<  Task priority, ranges from [0: #OS_TASK_COUNT - 1]. The lower the value,
                                              the higher the task priority. In the same tick, if multiple tasks are
                                              to be executed, tasks with higher priority are executed first before tasks
                                              with lower priority. There can be only one task with a certain priority level.  */
        OS_Task_Flag_t flags;           /**<  Task flags, setting it to #OS_TASK_FLAG_ONESHOT will get the task executed one time,
                                              hen deleted  */
} OS_Task_Def_t;

/* ------------------------------------------------------------------------- */

/**
 * List of tasks added to the scheduler
 * */
OS_Task_Def_t OS_asTaskList [OS_TASK_COUNT];

/**
 * Scheduler flags
 * */
OS_Flag_t OS_enFlags;

/* ------------------------------------------------------------------------- */

void OS_vidInitialize(void)
{
    uint32_t Local_u32TaskIdx;

    /*  reset task list  */
    for(Local_u32TaskIdx = 0; Local_u32TaskIdx < OS_TASK_COUNT; Local_u32TaskIdx++)
    {
        memset(&OS_asTaskList[Local_u32TaskIdx], 0x00, sizeof(OS_Task_Def_t));
    }
}

/* ------------------------------------------------------------------------- */

OS_Error_t OS_enAddTask(OS_vidTaskHandler_t pvHandler, void * const pvArgs, uint32_t u32Priority, OS_Tick_t u32Period, OS_Tick_t u32Delay, OS_TaskHandle_t * pTasKHandle)
{

#ifdef DEBUG

    if(IS_NULLPTR(pvHandler) | IS_NULLPTR(pTasKHandle))
    {
        return OS_ERROR_NULLPTR;
    }

    if(u32Priority >= OS_TASK_COUNT)
    {
        return OS_ERROR_INVALID_PARAM;
    }

#endif /*  DEBUG  */

    /*  add task to task list  */
    OS_asTaskList[u32Priority].handler  = pvHandler;
    OS_asTaskList[u32Priority].period   = u32Period;
    OS_asTaskList[u32Priority].args     = pvArgs;
    OS_asTaskList[u32Priority].flags    = OS_TASK_FLAG_NONE;

    if(IS_ZERO(u32Delay))
    {
        OS_asTaskList[u32Priority].delay    = u32Delay;
    }
    else
    {
        OS_asTaskList[u32Priority].delay    = u32Delay - 1;
    }

    if(IS_ZERO(u32Period))
    {
        OS_asTaskList[u32Priority].flags |= OS_TASK_FLAG_ONESHOT;
    }

    /*  pTaskHnadle = address offset of the task in the task list  */
    (*pTasKHandle) = (OS_TaskHandle_t *)((uint8_t *)&OS_asTaskList[u32Priority] - (uint8_t *)OS_asTaskList);

    return OS_ERROR_NONE;
}

/* ------------------------------------------------------------------------- */

OS_Error_t OS_enDeleteTask(OS_TaskHandle_t xTasKHandle)
{
    uint32_t Local_u32TaskIdx;

    /*  get task address  */
    Local_u32TaskIdx = (uint32_t)((uint8_t *)OS_asTaskList + (uint32_t)xTasKHandle);

    if(Local_u32TaskIdx >= OS_TASK_COUNT)
    {
        return OS_ERROR_INVALID_PARAM;
    }

    /*  reset task variables  */
    memset((OS_Task_Def_t*)xTasKHandle, 0x00, sizeof(OS_Task_Def_t));

    return OS_ERROR_NONE;
}

/* ------------------------------------------------------------------------- */

void OS_vidUpdateTasks(void)
{
    uint32_t Local_u32Priority;

    /*  clear update ready flag  */
    OS_enFlags |= OS_FLAG_DISPATCH_RDY;

    /*  update task list  */
    for(Local_u32Priority = 0; Local_u32Priority < OS_TASK_COUNT; Local_u32Priority++)
    {
        /*  check if no task was added  */
        if(IS_NULLPTR(OS_asTaskList[Local_u32Priority].handler))
        {
            continue;
        }

        /*
         * if task_delay == 0:
         *   - re-asign task delay (task_delay = task_period - 1)
         *   - set ask as ready to execute
         * else:
         *   - decrement task_delay
         **/
        if(OS_asTaskList[Local_u32Priority].delay)
        {
            OS_asTaskList[Local_u32Priority].delay--;
        }
        else
        {
            OS_asTaskList[Local_u32Priority].delay = OS_asTaskList[Local_u32Priority].period - 1;
            OS_asTaskList[Local_u32Priority].flags++;
        }
    }
}

/* ------------------------------------------------------------------------- */

void OS_vidDispatchTasks(void)
{
    uint32_t Local_u32Priority;

    /*  check scheduler dispatch ready flag  */
    if(!(OS_enFlags & OS_FLAG_DISPATCH_RDY))
    {
        return;
    }

    /*  clear scheduler's dispatch ready flag  */
    OS_enFlags &= ~OS_FLAG_DISPATCH_RDY;

    /*  execute tasks  */
    for(Local_u32Priority = 0; Local_u32Priority < OS_TASK_COUNT; Local_u32Priority++)
    {
        /*  check if no task was added  */
        if(IS_NULLPTR(OS_asTaskList[Local_u32Priority].handler))
        {
            continue;
        }

        /*  check if no task has any pending jobs  */
        if(OS_asTaskList[Local_u32Priority].flags & (OS_TASK_FLAG_MAX_JOBS))
        {
            /*  execute task (call ask handle & pass args)  */
            OS_asTaskList[Local_u32Priority].handler(OS_asTaskList[Local_u32Priority].args);

            /*  check if task is a one-time  */
            if((OS_asTaskList[Local_u32Priority].flags & OS_TASK_FLAG_ONESHOT))
            {
                memset(&OS_asTaskList[Local_u32Priority], 0x00, sizeof(OS_Task_Def_t));
            }
            else
            {
                OS_asTaskList[Local_u32Priority].flags = OS_TASK_FLAG_NONE;
            }
        }
    }
}

/* ------------------------------------------------------------------------- */

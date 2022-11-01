# Simple OS

Simple OS is a minimal co-operative operating system. The implementation is kept as minimal and simple as possible so, it only provides a scheduler with a single scheduling algorithm, priority based task scheduling. Simple OS doesn't implement tick generation, but provides a function to be called periodically to update the scheduler. This allows simple os to have a have small memory footprint, and be easily integrated into different environments. 
Simple OS is an example for co-operative schedulers, as discussed in [Paterns for Time Triggered Embedded Systems](https://www.safetty.net/publications/pttes) book by Michael J. Pont. 


### Scheduler

Simple OS scheduler divides its work between two functions: `OS_vidUpdateTasks()` and `OS_vidDispatchTasks()`. 

- `OS_vidUpdateTasks()`: updates the task list at every OS tick, then signals `OS_vidDispatchTasks()` to dispatch tasks by setting `OS_FLAG_DISPATCH_RDY` flag.

- `OS_vidDispatchTasks()`: when `OS_FLAG_DISPATCH_RDY` is set, checks tasks ready to be executed and execute them, highest priority task first.

```text
	
	T1: Task 1 with delay 1 OS ticks, and period 2 OS ticks, high priority
	T2: Task 2 with delay 0 OS ticks, and period 1 OS ticks, lower priority


	|		  
	| <--- T1 delay --->  <------------ T1 period ------------> |
	| <-- T2 period --> |
	+-----+				+-----+				+-----+				+-----+
	| Upd |   			| Upd |				| Upd |				| Upd |
    | ate |				| ate |				| ate |				| ate |
    | Tas +----+    	| Tas +----+----+	| Tas +----+	  	| Tas +----+----+
	| ks  | T2 |   		| ks  | T1 | T2 |	| ks  | T2 |    	| ks  | T1 | T2 |
	*-----+----+--------*-----+----+----+---*-----+----+--------*-----+----+----+-----> os ticks
    0              		1              		2         			3             

```


### Configuring Simple OS

Simple OS is configured using pre-processor definition. The definitions can be provided during build or added to `simple_os_conf.h` file.

- `OS_TICK_RATE_HZ`: Configure Simple OS tick rate, how many times `OS_vidUpdateTasks()` is called by application code per second. Used by `OS_MS_TO_TICK()` macro to convert from milli-seconds time to OS ticks.
 
- `OS_TASK_COUNT`: Maximum number of tasks that will be added. 

- `OS_USE_16BIT_TICK`: Use 16-bit ticks instead of 32-bit ticks. This will reduce tick count range to \[0: 65535\], but will save 4 bytes per task in the RAM.


### APIs


```C
OS_MS_TO_TICKS(ms)
```

Converts from milli-seconds to OS ticks. The macro depends on another macro `OS_TICK_RATE_HZ` defined in `simple_os_conf.h`.

**params**:

- *ms*: time in milliseconds

**return**:

- number of OS ticks equivalent to given time, rounded down.


```C
void OS_vidInitialize(void);
```

Initializes Simple OS and clears the task list.

**params**:

- void

**return**:

- void


```C
OS_Error_t OS_enAddTask(
	OS_vidTaskHandler_t 	pvHandler, 
	void * const 			pvArgs, 
	uint32_t 				u32Priority, 
	uint32_t 				u32Period, 
	uint32_t 				u32Delay, 
	OS_TaskHandle_t * 		pTasKHandle
);
```

Add a task to task list.

**params**:

- *pvHandler*: pointer to task's function `[void, (void *)]`. Will be called when the function is ready to be executed.

- *pvArgs*: pointer to task's function argument, passed to the task's handler at execution

- *u32Priority*: task's priority, must be in range `[0: OS_TASK_COUNT - 1]`. Lower value = higher priority, `0` is the highest priority, while `OS_TASK _COUNT - 1` is the lowest priority. If multiple tasks are ready to be executed at the same tick, the task with the highest priority is executed first.

- *u32Period*: task's period in OS ticks, number of ticks between each call to the task's function.
                            `u32Period > 0` for periodic tasks. For one-shot tasks, `u32Period == 0`

- *u32Delay*: task's first call delay, how many ticks to wait before calling the task's function for the first time.

- *pTaskHandle*: pointer to a task handle variable, used to save the task's handle.
                            Task handle is used with functions like `OS_enDeleteTask()`, `OS_enSuspendTask()`, `OS_enResumeTask()`

**return**:
 
 - `OS_Error_t`


```C
OS_Error_t OS_enDeleteTask(OS_TaskHandle_t xTasKHandle);
```

Delete a task from task list


**params**:

- *xTaskHandle*	\[in\] Task handle (got from `OS_enAddTask()`) to delete


**return**:
 - `OS_Error_t`



```C
void OS_vidUpdateTasks(void);
```

Update task list, must be called periodically

**params**:

- void

**return**:

- void


```C
void OS_vidDispatchTasks(void);
```

executes tasks that are ready to be executed

**params**:

- void

**return**:

- void

### Usage

```C

#include "main.h"
#include "gpio.h"

#include "simple_os.h"

typedef struct task_args_t {
        GPIO_TypeDef *  gpio_port;
        uint32_t        gpio_pin;
}Task_Args_t;

typedef struct task_desc_t {
        Task_Args_t     task_args;
        uint32_t        delay_ms;
        OS_TaskHandle_t task_handle;
}Task_Desc_t;

static Task_Desc_t task_desc [OS_TASK_COUNT] = {
        {.task_args = {.gpio_port=GPIOA, .gpio_pin=LL_GPIO_PIN_0}, .delay_ms = 500   },
        {.task_args = {.gpio_port=GPIOA, .gpio_pin=LL_GPIO_PIN_1}, .delay_ms = 1000  },
        {.task_args = {.gpio_port=GPIOA, .gpio_pin=LL_GPIO_PIN_2}, .delay_ms = 2000  },
        {.task_args = {.gpio_port=GPIOA, .gpio_pin=LL_GPIO_PIN_3}, .delay_ms = 4000  },
        {.task_args = {.gpio_port=GPIOA, .gpio_pin=LL_GPIO_PIN_4}, .delay_ms = 8000  },
        {.task_args = {.gpio_port=GPIOA, .gpio_pin=LL_GPIO_PIN_5}, .delay_ms = 16000 },
        {.task_args = {.gpio_port=GPIOA, .gpio_pin=LL_GPIO_PIN_6}, .delay_ms = 32000 },
        {.task_args = {.gpio_port=GPIOA, .gpio_pin=LL_GPIO_PIN_7}, .delay_ms = 64000 },
};

void SystemClock_Config(void);
void task_fn(void * const args);

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

    LL_GPIO_AF_Remap_SWJ_NOJTAG();

    SystemClock_Config();

    MX_GPIO_Init();

    SysTick->LOAD  = (uint32_t)((SystemCoreClock / 1000) - 1UL);        /* set reload register */
    SysTick->VAL   = 0UL;                                               /* Load the SysTick Counter Value */
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
            SysTick_CTRL_TICKINT_Msk |                         /* Enable the Systick interrupt */
            SysTick_CTRL_ENABLE_Msk;                           /* Enable the Systick Timer */

    OS_vidInitialize();

    for(uint32_t i = 0; i < OS_TASK_COUNT; i++)
    {
        LL_GPIO_SetPinMode(
                task_desc[i].task_args.gpio_port,
                task_desc[i].task_args.gpio_pin,
                LL_GPIO_MODE_OUTPUT
        );

        LL_GPIO_SetPinOutputType(
                task_desc[i].task_args.gpio_port,
                task_desc[i].task_args.gpio_pin,
                LL_GPIO_OUTPUT_OPENDRAIN
        );

        LL_GPIO_ResetOutputPin(
                task_desc[i].task_args.gpio_port,
                task_desc[i].task_args.gpio_pin
        );

        OS_enAddTask(
                task_fn,
                &task_desc[i].task_args,
                i,
                OS_MS_TO_TICKS(task_desc[i].delay_ms),
                0,
                &task_desc[i].task_handle
        );

    }


    while (1)
    {
        OS_vidDispatchTasks();
    }
}

void SystemClock_Config(void)
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);
    while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_0)
    {
    }
    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_HSI_Enable();

    /* Wait till HSI is ready */
    while(LL_RCC_HSI_IsReady() != 1)
    {

    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

    /* Wait till System clock is ready */
    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
    {
    }

    LL_Init1msTick(8000000);
    LL_SetSystemCoreClock(8000000);
}

void task_fn(void * const args)
{
    Task_Args_t * Local_psArgs = (Task_Args_t *)args;

    LL_GPIO_TogglePin(Local_psArgs->gpio_port, Local_psArgs->gpio_pin);
}

void SysTick_Handler(void)
{
    OS_vidUpdateTasks();
}


```

### Build

Simple OS can be built with make, using different build configurations: debug, release, release with minimum size.

> Make sure toolchain's GCC is in path or add `GCC_PATH=path/to/gcc` when invoking make: `make all GCC_PATH=../.`

```shell
make build=Debug 
make build=Release
make build=RelMinsize 
```

To build simple OS doxygen documentations, this will generate HTML files under `Docs/doxygen`

```shell
make docs
```

Clean build directories

```shell
make clean
make clean_all
```



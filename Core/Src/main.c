/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "simple_os.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void task_fn(void * const args);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_AFIO);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    /* System interrupt init*/
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    /* SysTick_IRQn interrupt configuration */
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),15, 0));

    /** NOJTAG: JTAG-DP Disabled and SW-DP Enabled
     */
    LL_GPIO_AF_Remap_SWJ_NOJTAG();

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    /* USER CODE BEGIN 2 */

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


    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
        OS_vidDispatchTasks();
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
    }
    /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
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

/* USER CODE BEGIN 4 */

void task_fn(void * const args)
{
    Task_Args_t * Local_psArgs = (Task_Args_t *)args;

    LL_GPIO_TogglePin(Local_psArgs->gpio_port, Local_psArgs->gpio_pin);
}

void SysTick_Handler(void)
{
    OS_vidUpdateTasks();
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

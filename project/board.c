/*
 * Copyright (c) 2006-2019, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017-07-24     Tanek        the first version
 * 2018-11-12     Ernest Chen  modify copyright
 */
 
#include <stdint.h>
#include <rthw.h>
#include <rtthread.h>
#include "board.h"
#include "usart.h"
#include "led.h"
#include "delay.h"

/*
#define _SCB_BASE       (0xE000E010UL)
#define _SYSTICK_CTRL   (*(rt_uint32_t *)(_SCB_BASE + 0x0))
#define _SYSTICK_LOAD   (*(rt_uint32_t *)(_SCB_BASE + 0x4))
#define _SYSTICK_VAL    (*(rt_uint32_t *)(_SCB_BASE + 0x8))
#define _SYSTICK_CALIB  (*(rt_uint32_t *)(_SCB_BASE + 0xC))
#define _SYSTICK_PRI    (*(rt_uint8_t  *)(0xE000ED23UL))
static uint32_t _SysTick_Config(rt_uint32_t ticks)
{
    if ((ticks - 1) > 0xFFFFFF)
    {
        return 1;
    }
    
    _SYSTICK_LOAD = ticks - 1; 
    _SYSTICK_PRI = 0xFF;
    _SYSTICK_VAL  = 0;
    _SYSTICK_CTRL = 0x07;  //bit 2 = 1 using hclk bit2 = 0 using ahb/8
    
    return 0;
}
*/
// Updates the variable SystemCoreClock and must be called 
// whenever the core clock is changed during program execution.
extern void SystemCoreClockUpdate(void);

// Holds the system core clock, which is the system clock 
// frequency supplied to the SysTick timer and the processor 
// core clock.
extern uint32_t SystemCoreClock;



#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 1024
static uint32_t rt_heap[RT_HEAP_SIZE];     // heap default size: 4K(1024 * 4)
RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

void rt_hw_console_output(const char *str)
{
    /* empty console output */
    rt_enter_critical();
    while(*str != '\0')
    {
        if(*str == '\n')
        {
            HAL_UART_Transmit(&UART1_Handler, (const uint8_t*)"\r\n", 2, 200);
        }
        HAL_UART_Transmit(&UART1_Handler, (const uint8_t*)(str++), 1, 200);
    }
    rt_exit_critical();
}

int test_board_section_init()
{
    rt_kprintf("tony test this is a board init function\n");
    return 0;
}
INIT_BOARD_EXPORT(test_board_section_init);
/**
 * This function will initial your board.
 */
void rt_hw_board_init()
{
    HAL_Init(); //初始化 HAL 库
    Stm32_Clock_Init(25,384,4,8); //设置时钟,96Mhz
    /* System Clock Update */
    SystemCoreClockUpdate();

    delay_init(HAL_RCC_GetHCLKFreq()/1000000);//1US跑的tick数
    uart_init(115200);
    led_init();
    /* Call components board initial (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif

#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif
}

void SysTick_Handler(void)
{
    /* enter interrupt */
    rt_interrupt_enter();

    rt_tick_increase();

    /* leave interrupt */
    rt_interrupt_leave();
    HAL_IncTick();
}

void rt_hw_us_delay(rt_uint32_t us)
{
    rt_uint32_t us_ticks;
	rt_uint32_t tnow, tcnt = 0;
    rt_uint32_t told = SysTick->VAL;
	rt_uint32_t reload = SysTick->LOAD;
	us_ticks = us * (SystemCoreClock / 1000000UL);
	/*获取当前时间*/
	while(RT_TRUE)
	{
		tnow = SysTick->VAL;
        tcnt += told > tnow ? told - tnow : reload - tnow + told;
		told = tnow;
		if(tcnt >= us_ticks)
		{
			break;
		}
	}
}
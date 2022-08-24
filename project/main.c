#include "stm32f1xx_hal.h"
#include "sys.h"
#include <stdio.h>
#include <stdlib.h>
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "rtthread.h"
#include "board.h"


void Delay(__IO uint32_t nCount);
void Delay(__IO uint32_t nCount)
{
    while(nCount--){}
}
static struct rt_thread led1_thread;

static rt_uint8_t rt_led1_thread_stack [1024] ALIGN(RT_ALIGN_SIZE);

void led1_thread_entry(void* p)
{
    while(1)
    {
      rt_thread_delay(200);
      //printf("this thread tid is %s\n",rt_thread_self()->name);
    }
}

void led1_thread2_entry(void* p)
{
  while(1)
  {
    
    rt_thread_delay(500);
    //rt_hw_us_delay(5000000);
    //printf("led2 off\n");
    led_off();
    
    rt_thread_delay(500);
    //rt_hw_us_delay(5000000);
    //printf("led2 on\n");
    led_on();
  }
}
void mtimerout()
{
  static int m;
  if(m == 0)
  {
    //LED1_OFF();
    ++m;
  }
  else if(m == 1)
  {
    m = 0;
    //LED1_ON();
  }
  
}
int main(void)
{
  struct rt_timer timer1;
  rt_timer_init(&timer1,"timer1",mtimerout,RT_NULL,50,RT_TIMER_FLAG_PERIODIC);
  rt_timer_start(&timer1);
  rt_thread_init(&led1_thread, "led1", led1_thread_entry, RT_NULL, &rt_led1_thread_stack[0], 1024, 3, 20);
  rt_thread_startup(&led1_thread);

  struct rt_thread* led2_thread = rt_thread_create("led2", led1_thread2_entry, RT_NULL, 1024, 3, 20);
  rt_thread_startup(led2_thread);
  while(RT_TRUE)
  {
      rt_thread_delay(5000);
      //rt_timer_stop(&timer1);
      //rt_timer_detach(&timer1);
  }
}
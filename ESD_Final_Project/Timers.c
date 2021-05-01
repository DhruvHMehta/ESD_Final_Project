/*
 * Timers.c
 *
 *  Created on: May 1, 2021
 *      Author: Dhruv
 *      Brief : Contains the hard-spin and timer based
 *              delays used in the application and SysTick
 *              ISR.
 */
#include <msp.h>
#include "Timers.h"

volatile uint32_t cur_time, ref_time, millis;

void Delay(uint32_t loop)
{
    volatile uint32_t i;

    for (i = 0 ; i < loop ; i++);
}

void delay_timer(uint8_t sec)
{
    ref_time = cur_time;
    while(cur_time - ref_time < sec);
    ref_time = cur_time;
}

void SysTick_Handler()
{
    __disable_irq();

    static uint16_t counter = 0;
    millis++;

    /* 1 second has elapsed */
    if(counter == 1000)
    {
        cur_time++;
        counter = 0;
    }
    counter++;

    __enable_irq();
}

/*
 * HCSR04.c
 *
 *  Created on: May 1, 2021
 *      Author: Dhruv
 *      Brief : Contains the sequence for polling the ultrasonic sensor
 *              and calculating distance from the object.
 */
#include "msp.h"
#include "cbfifo.h"
#include "Timers.h"
#include "peripherals.h"
#include "HCSR04.h"
#include <stdint.h>

extern volatile uint32_t millis;
volatile uint32_t ultra_read;

void Ultrasonic_GetData()
{
    uint8_t distance;

    P2->DIR |= BIT6;
    P2->OUT |= BIT6;          // generate pulse

    Delay(100);               // for 10us

    P2->OUT &= ~BIT6;         // stop pulse

    P2->IFG = 0;              // clear P2 interrupt just in case anything happened before
    P2->IES &= ~BIT7;         // rising edge on ECHO pin

    Delay(30000);             // delay for 30ms (after this time echo times out if there is no object detected)

    if (ultra_read == 580000){
        ultra_read=0;
    }
    __disable_irq();
    distance = ultra_read/58;     // converting ECHO lenght into cm
    __enable_irq();

    if(distance < 30)
    {
        P2->OUT |= BIT0;            //turning LED on if distance is less than 30cm and if distance isn't 0.
        P4->OUT &= ~(BIT0 | BIT1 | BIT6 | BIT7);
    }

    else
        P2->OUT &= ~BIT0;
}

/* Port2 ISR */
void PORT2_IRQHandler()
{
    __disable_irq();

    /* Check for Echo Interrupt */
    if(P2->IFG & BIT7)
    {
        /* Rising edge interrupt */
        if(!(P2->IES & BIT7))
        {
            /* Clear the timer and start a stopwatch */
            SysTick->VAL = 0;
            //TIMER_A0->CTL |= TIMER_A_CTL_CLR;
            millis = 0;

            /* Configure the interrupt for falling edge */
            P2->IES |=  BIT7;
        }
        else
        {
            /* Falling edge detected, calculate time in microseconds */
            ultra_read = millis*1000 + ((SYSTICK_RELOAD - SysTick->VAL) * 1000)/SYSTICK_RELOAD;

            /* Configure interrupt back to rising edge */
            P2->IES &=  ~BIT7;

        }

        /* Clear IFG */
        P2->IFG &= ~BIT7;
    }

    __enable_irq();
}


/*
 * peripherals.h
 *
 *  Created on: May 1, 2021
 *      Author: Dhruv
 */

#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_

#include "PCD8544_Character.h"
#include "PCD8544_Core.h"
#include "PCD8544_Pixel.h"

/* SysTick reload value for 1ms Ticks at 12Mhz Clock */
#define SYSTICK_RELOAD          (12000)

/*
 * @name ClockSource_Init
 *
 * @brief Initializes the system clock by setting the
 *        DCO to 12Mhz.
 *        SMCLK = 12Mhz
 *
 * @param none
 * @return none
 */
void ClockSource_Init();

/*
 * @name PWM_Init
 *
 * @brief Initializes PWM on pin 2.5 using Timer 0
 *        for 1.2kHz (~830uS period) at 60% duty cycle.
 *        Used for motor driver speed control.
 *
 * @param none
 * @return none
 */
void PWM_Init();

/*
 * @name Switch_Init
 *
 * @brief Initializes Switches SW1 and SW4 for debug.
 *        Toggle SW1 = Move Motors forward.
 *        Toggle SW2 = Move Motors backward.
 *
 * @param none
 * @return none
 */
void Switch_Init();

/*
 * @name GPIO_Init
 *
 * @brief Initializes GPIOs for motor direction control.
 *        Initializes indicator LED (RED), Obstacle LED
 *        (RED) and Sanitize LED (BLUE)
 *
 * @param none
 * @return none
 */
void GPIO_Init();

/*
 * @name SysTick_Init
 *
 * @brief Initializes the SysTick module for 100ms ticks.
 *        Used for generating accurate delays and for
 *        ultrasonic time measurement.
 *
 * @param none
 * @return none
 */
void SysTick_Init();

/*
 * @name I2C_Init
 *
 * @brief Initializes the EUSCI B1 I2C Module
 *        for the temperature sensor.
 *        Standard mode (100kHz) used.
 *
 * @param none
 * @return none
 */
void I2C_Init();

/*
 * @name Ultrasonic_Init
 *
 * @brief Initializes GPIO Pins for ultrasonic
 *        sensor Trigger and echo functionality.
 *        Echo pin configured as an interrupt.
 *
 * @param none
 * @return none
 */
void Ultrasonic_Init();

/*
 * @name UART_Init
 *
 * @brief Initializes the UART module for communication
 *        with the ESP8266.
 *        Baud rate = 115200, interrupt based using
 *        circular buffer.
 *
 * @param none
 * @return none
 */
void UART_Init();

/*
 * @name LCD_Init
 *
 * @brief Initializes, clears and displays the CU Logo
 *        on the Display module.
 *
 * @param none
 * @return none
 */
void LCD_Init();

/*
 * @name Enable_Interrupts
 *
 * @brief Enables interrupts in the NVIC for initialized modules
 *
 * @param none
 * @return none
 */
void Enable_Interrupts();


#endif /* PERIPHERALS_H_ */

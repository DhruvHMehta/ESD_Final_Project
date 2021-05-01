/*
 * Timers.h
 *
 *  Created on: May 1, 2021
 *      Author: Dhruv
 */

#ifndef TIMERS_H_
#define TIMERS_H_

#include <stdint.h>

/*
 * @name Delay
 *
 * @brief Hard-spin delay for the required loops.
 *
 * @param uint32_t loop - number of spins required.
 * @return none
 */
void Delay(uint32_t loop);

/*
 * @name delay_timer
 *
 * @brief An accurate, SysTick based delay using
 *        100ms ticks.
 *
 * @param uint8_t - sec - number of seconds to delay
 * @return none
 */
void delay_timer(uint8_t sec);

#endif /* TIMERS_H_ */

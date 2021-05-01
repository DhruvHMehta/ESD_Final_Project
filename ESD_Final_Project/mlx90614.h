/*
 * mlx90614.h
 *
 *  Created on: May 1, 2021
 *      Author: Dhruv
 */

#ifndef MLX90614_H_
#define MLX90614_H_

#include <stdint.h>

/*
 * @name CRC_8
 *
 * @brief Calculates the CRC-8 of the given bytes of data.
 *
 * @param uint8_t* p - pointer to buffer where the data starts.
 *        uint8_t len - length of data in bytes.
 *
 * @return uint8_t - CRC-8 result
 */
uint8_t CRC_8(uint8_t *p, uint8_t len);

/*
 * @name get_temp
 *
 * @brief Polls the SMBus temperature sensor (MLX90614)
 *        and returns the temperature value after checking
 *        the CRC.
 *
 * @param none
 * @return int16_t - temperature in celsius.
 */
int16_t get_temp();



#endif /* MLX90614_H_ */

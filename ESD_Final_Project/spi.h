/*
 * spi.h
 *
 *  Created on: 14-Apr-2021
 *      Author: Shrikant
 */

#ifndef SPI_H_
#define SPI_H_

/**
 * @brief initializes SPI
 *
 * @return none
 */
void ADXL345_SPI_Init();

/**
 * @brief Writes to a register on a SPI device
 *
 * @param addr The address of the register
 * @param data The byte to write to the register
 *
 * @return none
 */
void ADXL345_SPI_Write(uint8_t addr, uint8_t data);

/**
 * @brief Reads from a register on a SPI device
 *
 * @param addr The address of the register
 *
 * @return The returned data
 */
uint8_t ADXL345_SPI_Read(uint8_t addr);

/**
 * @brief Reads from two registers on a SPI device
 *
 * Assumes the first register is the LSB
 *
 * @param addr The address of the register
 *
 * @return The returned data
 */
uint16_t spi_read_double(uint8_t addr);



#endif /* SPI_H_ */

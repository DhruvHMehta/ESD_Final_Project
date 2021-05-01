/*
 * spi.h - Header File For Implementation Of SPI Module For ADXL345
 *
 */

#ifndef SPI_H_
#define SPI_H_

/**
 *  Initializes USCI B0 SPI Module for ADXL345 Communication
 *  PARAMETERS - NONE
 *  RETURNS        -  NONE
 */
void ADXL345_SPI_Init();

/**
 * Writes to specified ADXL345 register
 *
 *  PARAMETERS - Regsiter address ,Data to be written
 *  RETURNS        -  NONE
 *
 */
void ADXL345_SPI_Write(uint8_t addr, uint8_t data);

/**
 * Reads the value from specified ADXL345 register
 *
 * PARAMETERS - Register address
 * RETURNS        -  NONE
 */
uint8_t ADXL345_SPI_Read(uint8_t addr);



#endif /* SPI_H_ */

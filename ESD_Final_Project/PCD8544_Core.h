/*
 * PCD8544.h
 *
 *  Created on: 22-Apr-2021
 *      Author: Shrikant
 */

#ifndef PCD8544_CORE_H_
#define PCD8544_CORE_H_

#include "msp.h"

void PCD8544_SPI_Init(void);

void PCD8544_SPI_Write(unsigned char data);

void PCD8544_Init(void);

void PCD8544_Data_Write(unsigned char data);

void PCD8544_Command_Write(unsigned char data);

void PCD8544_SetCursor(unsigned char x, unsigned char y);

void PCD8544_Clear(void);



#endif /* PCD8544_CORE_H_ */

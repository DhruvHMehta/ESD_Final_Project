/*
 * PCD8544.h - Header file for implementation of Core GLCD functions
 *
 */

#ifndef PCD8544_CORE_H_
#define PCD8544_CORE_H_

#include "msp.h"

/**
 *  Configures  & Initializes the USCI B2 SPI Module for  Communication with PCD8544 LCD Driver
 *  PARAMETERS - NONE
 *  RETURNS        -  NONE
 */
void PCD8544_SPI_Init(void);

/**
 *  Sends data to PCD8544 LCD Driver over SPI
 *  PARAMETERS -  Data to be written
 *  RETURNS        -  NONE
 */
void PCD8544_SPI_Write(unsigned char data);

/**
 *  Configures & Initializes GLCD
 *  PARAMETERS -  NONE
 *  RETURNS        -  NONE
 */
void PCD8544_Init(void);

/**
 *  Writes to GLCD's Data Register
 *  PARAMETERS -  Data to be written
 *  RETURNS        -  NONE
 */
void PCD8544_Data_Write(unsigned char data);

/**
 *  Writes to GLCD's Command Register
 *  PARAMETERS -  Command to be written
 *  RETURNS        -  NONE
 */
void PCD8544_Command_Write(unsigned char data);

/**
 *  Moves Cursor to specified position
 *  PARAMETERS -  Row and Column 
 *  RETURNS        -  NONE
 */
void PCD8544_SetCursor(unsigned char x, unsigned char y);

/**
 *  Clears GLCD Screen
 *  PARAMETERS -  NONE
 *  RETURNS        -  NONE
 */
void PCD8544_Clear(void);



#endif /* PCD8544_CORE_H_ */

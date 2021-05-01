/*
 * PCD8544_Character.h - Header File for  implementation of character display functionality
 *
 */

#ifndef PCD8544_CHARACTER_H_
#define PCD8544_CHARACTER_H_

#include "PCD8544_Core.h"
#include "PCD8544_Pixel.h"

/**
 *  Displays a Character on Screen
 *  PARAMETERS -  Character to be displayed
 *  RETURNS        -  NONE
 */
void PCD8544_Putc(char c);

/**
 *  Displays a String on Screen
 *  PARAMETERS -  String to be displayed,row & column
 *  RETURNS        -  NONE
 */
void PCD8544_Puts(char *c,uint8_t x, uint8_t y);


#endif /* PCD8544_CHARACTER_H_ */

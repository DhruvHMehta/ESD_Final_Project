/*
 * PCD8544_Character.h
 *
 *  Created on: 23-Apr-2021
 *      Author: Shrikant
 */

#ifndef PCD8544_CHARACTER_H_
#define PCD8544_CHARACTER_H_

#include "PCD8544_Core.h"
#include "PCD8544_Pixel.h"


void PCD8544_Putc(char c);

void PCD8544_Puts(char *c,uint8_t x, uint8_t y);


#endif /* PCD8544_CHARACTER_H_ */

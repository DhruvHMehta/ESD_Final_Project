/*
 * PCD8544_Pixel.h
 *
 */

#ifndef PCD8544_PIXEL_H_
#define PCD8544_PIXEL_H_

#include "PCD8544_Core.h"


typedef enum {
    PCD8544_Pixel_White = 0,
    PCD8544_Pixel_Black = ! PCD8544_Pixel_White
} PCD8544_Pixel;



void DisplayBMP(const unsigned char BMP_Array[]);


#endif /* PCD8544_PIXEL_H_ */

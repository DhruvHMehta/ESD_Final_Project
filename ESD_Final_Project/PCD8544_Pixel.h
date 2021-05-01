/*
 * PCD8544_Pixel.h -  Source file containing implementation of graphic display functionality of GLCD
 *
 */

#ifndef PCD8544_PIXEL_H_
#define PCD8544_PIXEL_H_

#include "PCD8544_Core.h"

// Enumeration to hold Pixel State
typedef enum {
    PCD8544_Pixel_White = 0,
    PCD8544_Pixel_Black = ! PCD8544_Pixel_White
} PCD8544_Pixel;


/**
 *  Displays a Graphic Image  on GLCD
 *  PARAMETERS -  Bit Map Array of the Image to be displayed
 *  RETURNS        -  NONE
 */
void DisplayBMP(const unsigned char BMP_Array[]);


#endif /* PCD8544_PIXEL_H_ */

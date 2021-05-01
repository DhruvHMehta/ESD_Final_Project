/*
 * PCD8544_Pixel.c - Source file containing implementation of graphic display functionality of GLCD
 *
 */

#include "PCD8544_Pixel.h"


void DisplayBMP(const unsigned char BMP_Array[])
{

    unsigned short index = 0;

    for (index = 0; index < 504; index++)

    {
        PCD8544_Data_Write(BMP_Array[index]);                 //Send Bit Map array value to PCD8544 for display
    }

}






/*
 * PCD8544_Pixel.c
 *
 *  Created on: 23-Apr-2021
 *      Author: Shrikant
 */

#include "PCD8544_Pixel.h"


void DisplayBMP(const unsigned char BMP_Array[])
{

    unsigned short index = 0;

    for (index = 0; index < 504; index++)

    {
        PCD8544_Data_Write(BMP_Array[index]);
    }

}






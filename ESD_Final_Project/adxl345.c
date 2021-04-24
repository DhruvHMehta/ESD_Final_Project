/*
 * adxl345.c
 *
 *  Created on: 25-Apr-2021
 *      Author: Shrikant
 */

#include "adxl345.h"

#define Y_AXIS_THRESH (0x24)

#define FREE_FALL_ACC_THRESH (0x28)

#define FREE_FALL_TIME  (0x29)

void ADXL345_Init()
{
  ADXL345_SPI_Write(ADXL_POWER_CTL, 0x08); /* enable measurements */
  ADXL345_SPI_Write(ADXL_INT_ENABLE, 0x00); /* disable interrupts */
  ADXL345_SPI_Write(ADXL_INT_MAP, 0x04); /* map all interrupts to INT1 */
  ADXL345_SPI_Write(ADXL_DATA_FORMAT, 0x00); /* set range to 2G */

  //Free Fall
  ADXL345_SPI_Write(ADXL_THRESH_FF, FREE_FALL_ACC_THRESH);
  ADXL345_SPI_Write(ADXL_TIME_FF, FREE_FALL_TIME);


  //Actvity
  ADXL345_SPI_Write(ADXL_ACT_INACT_CTL,0x20);
  ADXL345_SPI_Write(ADXL_THRESH_ACT,Y_AXIS_THRESH);

  ADXL345_SPI_Write(ADXL_INT_ENABLE, 0x14); /* enable tap interrupts */

  ADXL345_SPI_Read(ADXL_INT_SOURCE); /* clear all interrupts */
}

void ADXL345_GPIO_Init()
{
  P4->SEL0 &= ~(BIT5 | BIT4 | BIT0); /* GPIO mode */
  P4->SEL1 &= ~(BIT5 | BIT4 | BIT0);
  P4->DIR &= ~(BIT5 | BIT4 | BIT0); /* configure as inputs */
  P4->REN |= BIT0; /* enable pullup/down resistor */
  P4->OUT |= BIT0; /* pullup resistor */
  P4->IES &= ~(BIT5 | BIT4); /* interrupt on low to high transition */
  P4->IES |= BIT0; /* interrupt on high to low transition */
  P4->IE |= BIT4; /* enable interrupt generation */


}

void ADXL345_Port4_Interrupt_Enable()
{

    ADXL345_SPI_Read(ADXL_INT_SOURCE); /* clear interrupts */
    P4->IFG &= ~(BIT4);
    NVIC_EnableIRQ(PORT4_IRQn);
    __enable_interrupts();

}




/*
 * adxl345.c - Source file containing implementaion of core ADXL345 functions
 *
 */

#include "adxl345.h"

#define Y_AXIS_THRESH (0x24)                                         // Macro definition for Acceleration threshold for activity detection

#define FREE_FALL_ACC_THRESH (0x28)                                  // Macro definition for Acceleration threshold for free fall detection

#define FREE_FALL_TIME  (0x29)                                       // Macro definition for time threshold for free fall detection

void ADXL345_Init()
{
  ADXL345_SPI_Write(ADXL345_POWER_CTL, 0x08);                        // enable measurements
  ADXL345_SPI_Write(ADXL345_INT_ENABLE, 0x00);                       // disable interrupts
  ADXL345_SPI_Write(ADXL345_INT_MAP, 0x04);                          // map Activity interrupt to INT1 & Freefall interrupt to INT2
  ADXL345_SPI_Write(ADXL345_DATA_FORMAT, 0x00);                      // acceleration range to 2G

  //Free Fall Interrupt Configuration
  ADXL345_SPI_Write(ADXL345_THRESH_FF, FREE_FALL_ACC_THRESH);
  ADXL345_SPI_Write(ADXL345_TIME_FF, FREE_FALL_TIME);


  //Actvity Interrupt COnfiguration
  ADXL345_SPI_Write(ADXL345_ACT_INACT_CTL,0x20);
  ADXL345_SPI_Write(ADXL345_THRESH_ACT,Y_AXIS_THRESH);

  ADXL345_SPI_Write(ADXL345_INT_ENABLE, 0x14);                        // enable free fall & activity interrupts

  ADXL345_SPI_Read(ADXL345_INT_SOURCE);                               // clear interrupts
}

void ADXL345_GPIO_Init()
{
  //Select GPIO functionality
  P4->SEL0 &= ~(BIT5 | BIT4 | BIT0);
  P4->SEL1 &= ~(BIT5 | BIT4 | BIT0);
    
  P4->DIR &= ~(BIT5 | BIT4 | BIT0);                                   // Set pin direction as input
  P4->REN |= BIT0;                                                    // enable pullup/down functionality
  P4->OUT |= BIT0;                                                    // enable pullup resistor
  P4->IES &= ~(BIT5 | BIT4);                                          // enable interrupt on low to high transition on pin
  P4->IES |= BIT0;                                                    // enable interrupt on high to low transition on pin
  P4->IE |= BIT4;                                                     // enable interrupt


}

void ADXL345_Port4_Interrupt_Enable()
{

    ADXL345_SPI_Read(ADXL345_INT_SOURCE);                             // clear ADXL345 interrupts by reading source register
    P4->IFG &= ~(BIT4);                                               // clear interrupt flag
    NVIC_EnableIRQ(PORT4_IRQn);                                       // enable Port4 interrupt in NVIC
    __enable_interrupts();                                            // enbale intterupts

}




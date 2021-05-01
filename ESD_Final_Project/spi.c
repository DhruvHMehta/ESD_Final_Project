/*
 * spi.c -Source File Containing Implementation Of SPI Module For ADXL345
 *
 */

#include "msp.h"
#include "spi.h"

void ADXL345_SPI_Init()
{
  /* Chip Select signal for SPI mode */
  P3->SEL0 &= ~(BIT0);
  P3->SEL1 &= ~(BIT0);
  P3->DIR |= BIT0;
  P3->OUT |= BIT0;

  /* Select SPI functionality of pins */
  P1->SEL0 |= BIT7 | BIT6 | BIT5;
  P1->SEL1 &= ~(BIT7 | BIT6 | BIT5);



  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;                           // Keep USCI Module in reset state
  EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_CKPL |                            // Clock Polarity High 
                    EUSCI_B_CTLW0_MSB |                             // MSB first
                    EUSCI_B_CTLW0_MST |                             // Master mode
                    EUSCI_B_CTLW0_MODE_2 |                          // 4-wire with CS active low 
                    EUSCI_B_CTLW0_SSEL__SMCLK |                     // SMCLK as source 
                    EUSCI_B_CTLW0_STEM |
                    EUSCI_B_CTLW0_SWRST;                
  EUSCI_B0->BRW = 0x10;
  EUSCI_B0->CTLW0 &= ~(EUSCI_B_CTLW0_SWRST);                        // Enable USCI Module
}

void ADXL345_SPI_Write(uint8_t addr, uint8_t data)
{
  
  while(EUSCI_B0->STATW & EUSCI_B_STATW_SPI_BUSY);     //Wait for SPI Bus to be Free

  P3->OUT &= ~(BIT0);                                  //Select ADXL345 using its CS Signal

  EUSCI_B0->TXBUF = ~BIT7 & addr;                      //Transmit Register address and Write Bit
  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));         //Wait until transfer is complete

  EUSCI_B0->TXBUF = data;                              //Transmit Data
  while(EUSCI_B0->STATW & EUSCI_B_STATW_SPI_BUSY);     //Wait until transfer is complete

  P3->OUT |= BIT0;                                     //Deselect ADXL345 using its CS Signal
}

uint8_t ADXL345_SPI_Read(uint8_t addr)
{
  uint8_t read_value;

  while(EUSCI_B0->STATW & EUSCI_B_STATW_SPI_BUSY);    //Wait for SPI Bus to be Free

  P3->OUT &= ~(BIT0);                                 //Select ADXL345 using its CS Signal

  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));        //Wait until transfer is complete
  EUSCI_B0->TXBUF = BIT7 | addr;                      //Transmit Register address and Write Bit

 
  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));        //Wait until transfer is complete
  EUSCI_B0->TXBUF = 0;                                //Transmit Blank Byte

  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));        //Wait until a bytes is received from ADXL345
  read_value = EUSCI_B0->RXBUF;                       //Store the received byte

  while(EUSCI_B0->STATW & EUSCI_B_STATW_SPI_BUSY);    //Wait for SPI Bus to be Free
  P3->OUT |= BIT0;                                    //Deselect ADXL345 using its CS Signal

  return read_value;
}




/*
 * spi.c
 *
 *  Created on: 14-Apr-2021
 *      Author: Shrikant
 */

#include "msp.h"
#include "spi.h"

void ADXL345_SPI_Init()
{
  /* SPI for accelerometer */
  P3->SEL0 &= ~(BIT0); /* CS as STE for SPI mode */
  P3->SEL1 &= ~(BIT0);
  P3->DIR |= BIT0; /* set pin to output */
  P3->OUT |= BIT0; /* initialize pin high */


  P1->SEL0 |= BIT7 | BIT6 | BIT5; /* MISO, MISO, CLK in SPI mode */
  P1->SEL1 &= ~(BIT7 | BIT6 | BIT5);



  EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; /* disable */
  EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_CKPL | /* clock inactive high */
                    EUSCI_B_CTLW0_MSB | /* MSB first */
                    EUSCI_B_CTLW0_MST | /* master mode */
                    EUSCI_B_CTLW0_MODE_2 | /* 4-wire with CS active low */
                    EUSCI_B_CTLW0_SSEL__SMCLK | /* SMCLK as source */
                    EUSCI_B_CTLW0_STEM | /* CS used to enable */
                    EUSCI_B_CTLW0_SWRST; /* stay disabled */
  EUSCI_B0->BRW = 0x10;
  EUSCI_B0->CTLW0 &= ~(EUSCI_B_CTLW0_SWRST); /* enable */
}

void ADXL345_SPI_Write(uint8_t addr, uint8_t data)
{
  /* wait for idle */
  while(EUSCI_B0->STATW & EUSCI_B_STATW_SPI_BUSY);

  /* select chip */
  P3->OUT &= ~(BIT0);

  /* send address and write bit */
  EUSCI_B0->TXBUF = ~BIT7 & addr;
  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));

  /* send data */
  EUSCI_B0->TXBUF = data;
  while(EUSCI_B0->STATW & EUSCI_B_STATW_SPI_BUSY);

  /* deselect chip */
  P3->OUT |= BIT0;
}

uint8_t ADXL345_SPI_Read(uint8_t addr)
{
  uint8_t ret;

  /* wait for idle */
  while(EUSCI_B0->STATW & EUSCI_B_STATW_SPI_BUSY);

  /* select chip */
  P3->OUT &= ~(BIT0);

  /* send address and read bit */
  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
  EUSCI_B0->TXBUF = BIT7 | addr;

  /* send blank byte */
  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG));
  EUSCI_B0->TXBUF = 0;

  while(!(EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG));
  ret = EUSCI_B0->RXBUF;

  /* deselect chip */
  while(EUSCI_B0->STATW & EUSCI_B_STATW_SPI_BUSY);
  P3->OUT |= BIT0;

  return ret;
}




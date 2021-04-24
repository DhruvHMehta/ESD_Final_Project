/*
 * PCD8544.c
 *
 */

#include "PCD8544_Core.h"

#define CE    0x01      /* P6.0 chip select */
#define RESET 0x40      /* P6.6 reset */
#define DC    0x80      /* P6.7 register select */


#define PCD8544_WIDTH  84
#define PCD8544_HEIGHT 48



void PCD8544_SPI_Init(void) {

    EUSCI_B2->CTLW0 = EUSCI_B_CTLW0_SWRST;

    EUSCI_B2->CTLW0 = (EUSCI_B_CTLW0_SWRST| EUSCI_B_CTLW0_UCSSEL_3
                    | EUSCI_B_CTLW0_SYNC | EUSCI_B_CTLW0_MST
                    | EUSCI_B_CTLW0_MSB | EUSCI_B_CTLW0_CKPL);

    EUSCI_B2->BRW = 12;
    EUSCI_B2->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;


    P3->SEL0 |= (BIT5 | BIT6);
    P3->SEL1 &= ~(BIT5 | BIT6);

    P6->DIR |= CE | RESET | DC;
    P6->OUT |= CE;
    P6->OUT &= ~RESET;

}

void PCD8544_SPI_Write(unsigned char data) {
    P6->OUT &= ~CE;
    EUSCI_B2->TXBUF = data;
    while(EUSCI_B2->STATW & 0x01) ;
    P6->OUT |= CE;
}


void PCD8544_Init(void) {
    PCD8544_SPI_Init();

    P6->OUT |= RESET;

    PCD8544_Command_Write(0x21);       /* extended command mode */
    PCD8544_Command_Write(0xB8);       /* LCD Vop for contrast */
    PCD8544_Command_Write(0x04);       /* temp coefficient */
    PCD8544_Command_Write(0x14);       /* LCD bias mode 1:48 */
    PCD8544_Command_Write(0x20);       /* normal command mode */
    PCD8544_Command_Write(0x0C);       /* display normal mode */
}


void PCD8544_Data_Write(unsigned char data) {
    P6->OUT |= DC;
    PCD8544_SPI_Write(data);
}


void PCD8544_Command_Write(unsigned char data) {
    P6->OUT &= ~DC;
    PCD8544_SPI_Write(data);
}

void PCD8544_SetCursor(unsigned char x, unsigned char y) {

    PCD8544_Command_Write(0x80 | x);
    PCD8544_Command_Write(0x40 | y);

}


void PCD8544_Clear(void) {
    int32_t index;
    for (index = 0 ; index < (PCD8544_WIDTH * PCD8544_HEIGHT / 8) ; index++)
        PCD8544_Data_Write(0x00);

    PCD8544_SetCursor(0, 0);
}



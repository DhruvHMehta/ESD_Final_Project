/*
 * PCD8544.c - Source file containing implementation of Core GLCD functions
 *
 */

#include "PCD8544_Core.h"

#define CE    BIT0           // Macro Definition for chip select pin
#define RESET BIT6           // Macro Definition for reset pin
#define DC    BIT7           // Macro Definition for register select pin


#define PCD8544_WIDTH  84    // Macro Definition for GLCD Width
#define PCD8544_HEIGHT 48    // Macro Definition for GLCD Height



void PCD8544_SPI_Init(void) {

    EUSCI_B2->CTLW0 = EUSCI_B_CTLW0_SWRST;                                // Keep USCI Module in reset state

    EUSCI_B2->CTLW0 = (EUSCI_B_CTLW0_SWRST| EUSCI_B_CTLW0_UCSSEL_3       // Select Clock Source as SMCLK
                    | EUSCI_B_CTLW0_SYNC | EUSCI_B_CTLW0_MST             // Synchronous mode Enable , Select MSB first
                    | EUSCI_B_CTLW0_MSB | EUSCI_B_CTLW0_CKPL);           // Clock Polarity High

    EUSCI_B2->BRW = 12;
    EUSCI_B2->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;                             // Enable USCI Module

    // Select SPI functionality for Pins
    P3->SEL0 |= (BIT5 | BIT6);
    P3->SEL1 &= ~(BIT5 | BIT6);
    
    
    P6->DIR |= CE | RESET | DC;                                         // Set direction of chip select,reset & register select pin as output
    P6->OUT |= CE;                                                      // Set chip select signal
    P6->OUT &= ~RESET;                                                  // Clear reset signal

}

void PCD8544_SPI_Write(unsigned char data) {
    
    P6->OUT &= ~CE;                                                    // Select PCD8544 for SPI communication
    EUSCI_B2->TXBUF = data;                                            // Transmit Data
    while(EUSCI_B2->STATW & 0x01) ;                                    // Wait until transfer is complete
    P6->OUT |= CE;                                                     // Deselect PCD8544 for SPI communication
    
}


void PCD8544_Init(void) {
    
    // Enable SPI Module for communication with
    PCD8544_SPI_Init();

    P6->OUT |= RESET;

    PCD8544_Command_Write(0x21);                                       // extended command mode
    PCD8544_Command_Write(0xB8);                                       // LCD Vop for contrast
    PCD8544_Command_Write(0x04);                                       // temp coefficient
    PCD8544_Command_Write(0x14);                                       // LCD bias mode 1:48
    PCD8544_Command_Write(0x20);                                       // normal command mode
    PCD8544_Command_Write(0x0C);                                       // display normal mode
}


void PCD8544_Data_Write(unsigned char data) {
    
    P6->OUT |= DC;                                                     // Select Data Register to write into
    PCD8544_SPI_Write(data);
    
}


void PCD8544_Command_Write(unsigned char data) {
    
    P6->OUT &= ~DC;                                                   // Select Command Register to write into
    PCD8544_SPI_Write(data);
    
}

void PCD8544_SetCursor(unsigned char x, unsigned char y) {

    PCD8544_Command_Write(0x80 | x);
    PCD8544_Command_Write(0x40 | y);

}


void PCD8544_Clear(void) {
    int32_t index;
    
    //Write 0 to all addresses to clear GLCD
    for (index = 0 ; index < (PCD8544_WIDTH * PCD8544_HEIGHT / 8) ; index++)
        PCD8544_Data_Write(0x00);

    PCD8544_SetCursor(0, 0);
}



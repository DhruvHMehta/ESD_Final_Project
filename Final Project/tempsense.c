#include "msp.h"

int temp;

unsigned char temperature;
unsigned char temp_low;
int temp_high;
unsigned char PEC;

#define MLX90614_I2C_ADDRESS     (0x5A)

void I2C_init(){

    P1->SEL0 |= BIT6 | BIT7;                // I2C pins

    // Enable eUSCIB0 interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((EUSCIB0_IRQn) & 31);

    // Configure USCI_B0 for I2C mode
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST; // Software reset enabled
    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST | // Remain eUSCI in reset mode
                EUSCI_B_CTLW0_MODE_3 |          // I2C mode
                EUSCI_B_CTLW0_MST |             // Master mode
                EUSCI_B_CTLW0_SYNC |            // Sync mode
                EUSCI_B_CTLW0_SSEL__SMCLK;      // SMCLK
    //EUSCI_B0->CTLW1 |= EUSCI_B_CTLW1_ASTP_2;// Automatic stop generated
                                                // after EUSCI_B0->TBCNT is reached
    EUSCI_B0->BRW = 30;                     // baudrate = SMCLK / 30 = 100kHz
    //EUSCI_B0->TBCNT = 0x0005;               // number of bytes to be received
    EUSCI_B0->I2CSA = MLX90614_I2C_ADDRESS ;               // Slave address (MLX 90614)

    EUSCI_B0->IE |= EUSCI_A_IE_RXIE | EUSCI_A_IE_TXIE | EUSCI_B_IE_NACKIE ;  // Enable receive interrupt

    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;// Release eUSCI from reset
}

char get_temp(void)
{
    EUSCI_B0->CTLW0 |= UCTR;
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;
    EUSCI_B0->TXBUF |= 0x27;                        //(0100 + 0111)
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_TXIFG0));
    EUSCI_B0->CTLW0 |= UCTR;
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_RXIFG0));
    temp_low = EUSCI_B0->RXBUF;
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_RXIFG0));
    temp_high = EUSCI_B0->RXBUF;
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_RXIFG0));
    PEC = EUSCI_B0->RXBUF;
    EUSCI_B0->CTLW0 |=EUSCI_B_CTLW0_TXSTP;
    temp_high=(temp_high<<8);                 //MSB um 4 Bits
    temp = temp_low | temp_high;              //MSB und LSB
    __no_operation();
    return temp;
}



/**
 * main.c
 */
void main(void)
{
WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // stop watchdog timer
I2C_init();
temperature = get_temp();
}
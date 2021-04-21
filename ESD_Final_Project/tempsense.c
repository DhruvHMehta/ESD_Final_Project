#include "msp.h"

unsigned char temperature;
uint8_t TXData[3] = {0xB4, 0x07, 0xB5};
uint8_t RXData[3];
uint8_t i, RXDataPointer;

#define MLX90614_I2C_ADDRESS     (0x5A)

void I2C_init(){

    P1->SEL0 |= BIT6 | BIT7;                     // I2C pins

    // Enable eUSCIB0 interrupt in NVIC module
    NVIC->ISER[0] = 1 << ((EUSCIB0_IRQn) & 31);

    // Configure USCI_B0 for I2C mode
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;     // Software reset enabled
    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST |     // Remain eUSCI in reset mode
                EUSCI_B_CTLW0_MODE_3 |          // I2C mode
                EUSCI_B_CTLW0_MST |             // Master mode
                EUSCI_B_CTLW0_SYNC |            // Sync mode
                EUSCI_B_CTLW0_SSEL__SMCLK;      // SMCLK


    EUSCI_B0->BRW = 30;                         // baudrate = SMCLK / 30 = 100kHz
    EUSCI_B0->I2CSA = MLX90614_I2C_ADDRESS ;    // Slave address (MLX 90614)

    EUSCI_B0->IE |= EUSCI_A_IE_RXIE | EUSCI_A_IE_TXIE | EUSCI_B_IE_NACKIE ;

    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;// Release eUSCI from reset
}

uint8_t get_temp(void)
{
    uint16_t temp_low, temp_high, temp;
    uint8_t PEC;

    /* Turn on Transmitter */
    EUSCI_B0->CTLW0 |= UCTR;

    /* Send Start Bit */
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;

    /* Slave Address = 0x5A, Write Command */
    EUSCI_B0->TXBUF = 0xB4;
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_TXIFG0));

    /* Command = Read RAM 0x07 (Ta) */
    EUSCI_B0->TXBUF = 0x07;
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_TXIFG0));

    /* Send Repeated Start Bit */
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;

    /* Turn on Receiver */
    EUSCI_B0->CTLW0 &= ~UCTR;

    /* Slave Address = 0x5A, Read Command */
    EUSCI_B0->TXBUF = 0xB5;
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_RXIFG0));

    /* Read LSByte of temperature */
    temp_low = EUSCI_B0->RXBUF;
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_RXIFG0));

    /* Read MSByte of temperature */
    temp_high = EUSCI_B0->RXBUF;

    /* Send Stop Bit */
    EUSCI_B0->CTLW0 |=EUSCI_B_CTLW0_TXSTP;
    while (!(EUSCI_B0->IFG &  EUSCI_B_IFG_RXIFG0));

    /* Read Packet Error Checking Byte */
    PEC = EUSCI_B0->RXBUF;

    temp_high = (temp_high << 8);

    /* Calculation of temp from analog-Kelvin to Celsius */
    temp = ((temp_low | temp_high)/50) - 273;

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

/*
void EUSCIB0_IRQHandler()
{
    if (EUSCI_B0->IFG & EUSCI_B_IFG_NACKIFG)
    {
        EUSCI_B0->IFG &= ~ EUSCI_B_IFG_NACKIFG;

        // I2C start condition
        EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;
    }

    /*
    if (EUSCI_B0->IFG & EUSCI_B_IFG_RXIFG0)
    {
        EUSCI_B0->IFG &= ~ EUSCI_B_IFG_RXIFG0;

        // Get RX data
        RXData[RXDataPointer++] = EUSCI_B0->RXBUF;

        if (RXDataPointer > sizeof(RXData))
        {
            RXDataPointer = 0;
        }

    }

    if (EUSCI_B0->IFG & EUSCI_B_IFG_TXIFG0)
        {
            if(i == 2)
            {
                EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_TXSTT;
            }
             EUSCI_B0->TXBUF = TXData[i++];
        }
}
*/

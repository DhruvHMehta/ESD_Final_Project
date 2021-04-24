#include "msp.h"
#include "cbfifo.h"
#include "PCD8544_Character.h"
#include "PCD8544_Core.h"
#include "PCD8544_Pixel.h"
#include <string.h>


/**
 * main.c
 */

/* I2C Temperature Sensor Slave Address */
#define MLX90614_I2C_ADDRESS     (0x5A)

/* SysTick reload value for 1ms Ticks at 12Mhz Clock */
#define SYSTICK_RELOAD          (12000)

/* Buffer to receive data */
volatile char command[100];
volatile uint8_t cmdptr, processflag;
volatile uint32_t cur_time, ref_time, millis, ultra_read;

/* AT Commands List */
static char* AT = "AT\r\n";
static char* CWMODE = "AT+CWMODE=3\r\n";
static char* STATIC_IP = "AT+CIPSTA=\"192.168.43.253\"\r\n";
static char* MULTIPLE_CON_ON = "AT+CIPMUX=1\r\n";
static char* TCP_CREATE = "AT+CIPSERVER=1,80\r\n";
static char* JOIN_AP = "AT+CWJAP_CUR=\"OnePlus 6T\",\"password\"\r\n";
static char* CONN_STATUS = "AT+CIPSTATUS\r\n";
static char* IP_DATALEN = "AT+CIPSEND=0,756\r\n";

/* CRC-8 Table */
static const uint8_t crc_table[] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31,
    0x24, 0x23, 0x2a, 0x2d, 0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d, 0xe0, 0xe7, 0xee, 0xe9,
    0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1,
    0xb4, 0xb3, 0xba, 0xbd, 0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea, 0xb7, 0xb0, 0xb9, 0xbe,
    0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16,
    0x03, 0x04, 0x0d, 0x0a, 0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a, 0x89, 0x8e, 0x87, 0x80,
    0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8,
    0xdd, 0xda, 0xd3, 0xd4, 0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44, 0x19, 0x1e, 0x17, 0x10,
    0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f,
    0x6a, 0x6d, 0x64, 0x63, 0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13, 0xae, 0xa9, 0xa0, 0xa7,
    0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef,
    0xfa, 0xfd, 0xf4, 0xf3
};

static const unsigned char logo [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x38, 0x38, 0x3c, 0x3c, 0x3e, 0x3e,
0x3e, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x7f, 0xff, 0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xf8, 0xf0, 0xe0, 0xc0, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0,
0xc0, 0xe0, 0xe0, 0xf0, 0xf0, 0xf0, 0xf8, 0xf8, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xfc, 0xfc, 0x7c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
0x3c, 0xfc, 0xfc, 0xfc, 0xcc, 0xc0, 0xc0, 0xc0, 0xc0, 0xff, 0xff, 0xff, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0xff, 0xff, 0xff,
0xff, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe, 0xfc, 0xfc, 0xfc, 0xf8, 0xf8, 0xf0, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x03, 0xf3, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x01, 0x00, 0x00, 0x00, 0x30, 0x7f, 0x7f, 0x7f, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00,
0x7e, 0x7f, 0x7f, 0x07, 0x03, 0x03, 0x03, 0xe7, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xfc, 0xfc, 0xff, 0xff, 0xff,
0xff, 0x3f, 0x0f, 0x07, 0x07, 0xe3, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xf0, 0xf8, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0xfc, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x40, 0x78,
0x78, 0x78, 0x78, 0x78, 0x7c, 0x7e, 0x7f, 0x7f, 0x7f, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xfe, 0xfe, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x1f, 0x0f, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xe0, 0xf8, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x07, 0x07,
0x07, 0x07, 0x0f, 0x0f, 0x0f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x0f, 0x0f, 0x0f, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0xc0, 0xe0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff, 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x1f, 0x3f,
0x3f, 0x3f, 0x1f, 0x1f, 0x1f, 0x0f, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x1f, 0x7f, 0xff, 0xff, 0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x1c, 0x3f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x1f, 0x0f, 0x07, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

void putUART(const char *_ptr)
{
        EUSCI_A0->TXBUF = (unsigned char) *_ptr;
}

void delay_timer(uint8_t sec)
{
    ref_time = cur_time;
    while(cur_time - ref_time < sec);
    ref_time = cur_time;
}

static void Delay(uint32_t loop)
{
    volatile uint32_t i;

    for (i = 0 ; i < loop ; i++);
}


uint8_t CRC_8(uint8_t *p, uint8_t len)
{
        uint16_t i;
        uint16_t crc = 0x0;

        while (len--)
        {
            /* Get index, then calculate crc using xor */
            i = (crc ^ *p++) & 0xFF;
            crc = (crc_table[i] ^ (crc << 8)) & 0xFF;
        }

        /* Mask off only a byte */
        return crc & 0xFF;
}

void ClockSource_Init()
{
    /* Unlock CS module for register access */
    CS->KEY = CS_KEY_VAL;

    /* Reset tuning parameters */
    CS->CTL0 = 0;

    /* Set DCO to 12MHz (nominal, center of 8-16MHz range) */
    CS->CTL0 = CS_CTL0_DCORSEL_3;

    /* Select ACLK = REFO
     *        SMCLK = DCO
     *        MCLK = DCO
     */
    CS->CTL1 = CS_CTL1_SELA_2 |
               CS_CTL1_SELS_3 |
               CS_CTL1_SELM_3;

    /* Lock CS module from unintended accesses */
    CS->KEY = 0;
}

void PWM_Init()
{
    /* TA0.2 */
    P2->DIR |= BIT5;

    /* Pin Mux Settings for TA0.2 */
    P2->SEL0 |= BIT5;
    P2->SEL1 &= ~BIT5;

    /* Set Capture and Compare 0 to 10000
     * For SMCLK = 12Mhz, Period = ~8us */
    TIMER_A0->CCR[0] = 10000;

    /* Set Initial Duty Cycle = 60% */
    TIMER_A0->CCR[2] = 6000;

    /* Set Timer to SET/RESET Mode */
    TIMER_A0->CCTL[2] = TIMER_A_CCTLN_OUTMOD_6;

    /* Select SMCLK Source (12Mhz) and Count Up, Start */
    TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 |
                    TIMER_A_CTL_MC__UP |
                    TIMER_A_CTL_CLR;
}

void SysTick_Init()
{
    /* Initialize Counter Value to 0 */
    SysTick->VAL = 0;

    /* Set the LOAD Value to 4800000 for 100ms Ticks */
    SysTick->LOAD = (SYSTICK_RELOAD);

    /* Set the Control Register for Internal ClockSource (12Mhz)
     * Enable Interrupt on Overflow and Enable the Timer */
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);

}


/* Pin Setup for Switch Interrupt Mode */
void Switch_Init()
{
    /* Configuring Port 1 Pin 1 and Pin 4 for Switch Operation */
    P1->DIR &= ~(uint8_t) (BIT4 | BIT1);

    /* Pull-up */
    P1->OUT |= (BIT1 | BIT4);
    P1->REN |= (BIT1 | BIT4);

    /* Pin Mux for I/O */
    P1->SEL0 |= 0;
    P1->SEL1 |= 0;

    /* Interrupt on high-to-low transition */
    P1->IES |= (BIT1 | BIT4);

    /* Clear all P1 interrupt flags */
    P1->IFG = 0;

    /* Enable interrupt for P1.1 and P1.4 */
    P1->IE |= (BIT1 | BIT4);

}

void GPIO_Init()
{
    /* Manual control of Relay for debugging
     * P4.0, 4.1 F/B of Left Motor
     * P4.6, 4.7 F/B of Right Motor
     *  */

    P4->DIR |= BIT0 | BIT1 | BIT6 | BIT7;
    P4->OUT &= ~(BIT0 | BIT1 | BIT6 | BIT7);

    /* Red Indicator LED */
    P1->DIR |= BIT0;
    P1->OUT &= ~BIT0;
}

void UART_Init()
{
    /* Configure UART pins (Pin Multiplexing) */
    P1->SEL0 |= BIT2 | BIT3;

    /* Configure UART
     * Put eUSCI in reset
     * Remain eUSCI in reset
     * Configure eUSCI clock source for SMCLK
     */
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST;
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST |
            EUSCI_B_CTLW0_SSEL__SMCLK;

    /* Baud Rate calculation
     * 12000000/(16*115200) = 6.5104
     * Fractional portion = 0.5104
     * User's Guide Table 21-4: UCBRSx = 0x20
     * UCBRFx = int ( (6.5104-6)*16) = 8
     */
    EUSCI_A0->BRW = 6;

    EUSCI_A0->MCTLW = (8 << EUSCI_A_MCTLW_BRF_OFS) | (0x20 << EUSCI_A_MCTLW_BRS_OFS) |
                       EUSCI_A_MCTLW_OS16;

    /* Initialize eUSCI */
    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST;

    /* Clear eUSCI RX interrupt flag */
    EUSCI_A0->IFG &= ~(EUSCI_A_IFG_RXIFG);

    /* Enable USCI_A0 TX and RX interrupt */
    EUSCI_A0->IE |= (EUSCI_A_IE_RXIE | EUSCI_A_IE_TXIE);

}

void I2C_Init(){

    /* I2C pins (Pin multiplexing) */
    P1->SEL0 |= BIT6 | BIT7;

    /* Configure USCI_B0 for I2C mode
     * Software reset enabled
     * Remain eUSCI in reset mode
     * I2C mode
     * Master mode
     * Sync mode
     * SMCLK
     */
    EUSCI_B0->CTLW0 |= EUSCI_B_CTLW0_SWRST;
    EUSCI_B0->CTLW0 = EUSCI_B_CTLW0_SWRST |
                EUSCI_B_CTLW0_MODE_3 |
                EUSCI_B_CTLW0_MST |
                EUSCI_B_CTLW0_SYNC |
                EUSCI_B_CTLW0_SSEL__SMCLK;

    /* baudrate = SMCLK / 120 = 100kHz */
    EUSCI_B0->BRW = 120;

    /* Slave address (MLX 90614) */
    EUSCI_B0->I2CSA = MLX90614_I2C_ADDRESS ;

    /* Enable interrupts for Receive, Transmit and No Acknowledge */
    EUSCI_B0->IE |= EUSCI_A_IE_RXIE | EUSCI_A_IE_TXIE | EUSCI_B_IE_NACKIE ;

    /* Release eUSCI from reset */
    EUSCI_B0->CTLW0 &= ~EUSCI_B_CTLW0_SWRST;
}

void Ultrasonic_Init()
{
    /* Echo pin */
    P2->DIR &= ~BIT7;

    /* Trigger pin */
    P2->DIR |= BIT6;

    /* P2.7 Pull Down */
    P2->REN |= BIT7;

    /* P2.6, 2.7 Low initially */
    P2->OUT &= ~(BIT6 | BIT7);

    /* Pin Muxing for I/O */
    P2->SEL0 &= ~BIT7;
    P2->SEL1 &= ~BIT7;

    /* Configure rising edge interrupt for echo */
    P2->IFG = 0;
    P2->IE |= BIT7;
    P2->IES &= ~BIT7;
}

void LCD_Init()
{
    PCD8544_Init();
    PCD8544_Clear();
    DisplayBMP(logo);
}

void Ultrasonic_GetData()
{
    uint8_t distance;

    P2->DIR |= BIT6;
    P2->OUT |= BIT6;          // generate pulse

    //P4->OUT |= BIT5;
    Delay(100);               // for 10us
    //P4->OUT &= ~BIT5;

    P2->OUT &= ~BIT6;         // stop pulse

    P2->IFG = 0;              // clear P2 interrupt just in case anything happened before
    P2->IES &= ~BIT7;         // rising edge on ECHO pin
    //P4->OUT |= BIT5;
    Delay(30000);             // delay for 30ms (after this time echo times out if there is no object detected)
    //P4->OUT &= ~BIT5;
    //Delay(60000);
    if (ultra_read == 580000){
        ultra_read=0;
    }
    __disable_irq();
    distance = ultra_read/58;     // converting ECHO lenght into cm
    __enable_irq();

    if(distance < 30 && distance != 0)
    {
        P1->OUT |= BIT0;            //turning LED on if distance is less than 30cm and if distance isn't 0.
        P4->OUT &= ~(BIT0 | BIT1 | BIT6 | BIT7);
    }

    else
        P1->OUT &= ~BIT0;
}

void Enable_Interrupts()
{
    /* Enable Port 1 interrupt on the NVIC */
    NVIC->ISER[1] = 1 << ((PORT1_IRQn) & 31);

    /* Enable eUSCIA0 interrupt in NVIC module */
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);

    /* Enable TA0 interrupt in NVIC module */
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

    /* Enable Port2 interrupt in NVIC module */
    NVIC->ISER[1] = 1 << ((PORT2_IRQn) & 31);

    /* Enable eUSCIB0 interrupt in NVIC module */
    NVIC->ISER[0] = 1 << ((EUSCIB0_IRQn) & 31);
}


void clear_buffer()
{
    __disable_irq();

    /* Zero out the receive buffer */
    for(uint16_t i = 0; i < sizeof(command)/sizeof(char); i++)
    {
        command[i] = 0;
    }
    cmdptr = 0;

    __enable_irq();
}

int16_t get_temp(void)
{
    uint16_t temp_low, temp_high, temp;
    uint8_t PEC;
    uint8_t crcbuf[5] = {0xB4, 0x07, 0xB5, 0x00, 0x00};

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

    crcbuf[3] = temp_low;
    crcbuf[4] = temp_high;

    /* Checking CRC with PEC Byte */
    if(PEC != CRC_8(crcbuf, 5))
        return -100;

    temp_high = (temp_high << 8);

    /* Calculation of temp from analog-Kelvin to Celsius */
    temp = ((temp_low | temp_high)/50) - 273;

    return temp;
}

void ESP_Send(char* ATCmd, uint16_t len)
{
    /* Enqueue data onto transmit buffer and enable transmit intr */
    cbfifo_enqueue(ATCmd, len);
    EUSCI_A0->IE |= EUSCI_A_IE_TXIE;
}

void ESP_Init()
{
    unsigned int i;

    /* Check if OK */

    ESP_Send(AT, strlen(AT));
    delay_timer(1);

    if(command[6] == 'O' && command[7] == 'K')
        clear_buffer();

    /* Set Current Wifi Mode - SoftAP + Station */
    ESP_Send(CWMODE, strlen(CWMODE));

    for(i = 0; i < 65535; i++);
    clear_buffer();

    /* Set Static IP */
    ESP_Send(STATIC_IP, strlen(STATIC_IP));

    for(i = 0; i < 65535; i++);
    clear_buffer();

    /* Allow Multiple Connections */
    ESP_Send(MULTIPLE_CON_ON, strlen(MULTIPLE_CON_ON));

    for(i = 0; i < 65535; i++);
    clear_buffer();

    /* Create TCP Server on Port 80 */
    ESP_Send(TCP_CREATE, strlen(TCP_CREATE));

    for(i = 0; i < 65535; i++);
    clear_buffer();

    /* Join an Access Point */
    ESP_Send(JOIN_AP, strlen(JOIN_AP));

    /* Wait till connection is established */
    delay_timer(10);

    clear_buffer();
}

void main(void)
{

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	P4->DIR |= BIT5;
	P4->OUT &= ~BIT5;

	/* Intialize all modules */
	ClockSource_Init();
	PWM_Init();
	Switch_Init();
	GPIO_Init();
	SysTick_Init();
    I2C_Init();
    Ultrasonic_Init();
	UART_Init();
	LCD_Init();

	/* Enable Interrupts and configure ESP */
    Enable_Interrupts();
    ESP_Init();

    /* Check Connection Status */
    ESP_Send(CONN_STATUS, strlen(CONN_STATUS));

    if(command[39] == '2')
    clear_buffer();

    while(command[21] != '3')
    {
    clear_buffer();
    ESP_Send(CONN_STATUS, strlen(CONN_STATUS));
    delay_timer(1);
    }

    clear_buffer();

    /* Send DataLength */
    ESP_Send(IP_DATALEN, strlen(IP_DATALEN));
    delay_timer(2);

   // if(command[25] == '>')
    //{
        cbfifo_enqueue("<!DOCTYPE html><html><body><h1>SanBot Control Page</h1><p>Please choose an option</p><p><b>TURN LEFT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"LEFT\"><button>LEFT</button></a></br></p><p><b>TURN RIGHT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"RIGHT\"><button>RIGHT</button></a></br></p><p><b>MOVE FORWARD</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"FORWARD\"><button>FORWARD</button></a></br></p><p><b>MOVE BACK</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"BACK\"><button>BACK</button></a></br></p><p><b>STOP BOT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"STOP\"><button>STOP</button></a></br></p><p><b>Check Temperature</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"TEMP\"><button>TEMP</button></a></br></p></body></html>\r\n", 756);
        EUSCI_A0->IE |= EUSCI_A_IE_TXIE;
        // }

        while(1)
        {
            /* To match the command and store temperature value */
            char* match;
            char LCD_tempsense[25];
            int16_t tempsense;

            if(processflag == 1)
            {
                __disable_irq();

                command[cmdptr++] = '\0';

                /* Find the first '/' as the command is next char */
                match = strchr(command, '/');
                match++;

                /* Map received character to command */
                switch(*match)
               {
                /* Move Back */
                case 'B':
                    P4->OUT |= (BIT0 | BIT6);
                    P4->OUT &= ~(BIT1 | BIT7);
                    break;

                /* Move Forward */
                case 'F':
                    P4->OUT |= (BIT1 | BIT7);
                    P4->OUT &= ~(BIT0 | BIT6);
                    break;

                /* Turn Right */
                case 'R':
                    P4->OUT |= (BIT1 | BIT6);
                    P4->OUT &= ~(BIT0 | BIT7);
                    break;

                /* Turn Left */
                case 'L':
                    P4->OUT |= (BIT0 | BIT7);
                    P4->OUT &= ~(BIT1 | BIT6);
                    break;

                /* Stop Bot */
                case 'S':
                    P4->OUT &= ~(BIT0 | BIT1 | BIT6 | BIT7);
                    break;

                /* Get the temperature value and check it */
                case 'T':
                    /* Disable Port 1 interrupt on the NVIC */
                    NVIC->ICER[1] = 1 << ((PORT1_IRQn) & 31);

                    /* Disable eUSCIA0 interrupt in NVIC module */
                    NVIC->ICER[0] = 1 << ((EUSCIA0_IRQn) & 31);

                    /* Disable TA0 interrupt in NVIC module */
                    NVIC->ICER[0] = 1 << ((TA0_0_IRQn) & 31);

                    __enable_irq();
                    tempsense = get_temp();

                    PCD8544_Clear();

                    if(tempsense > 37)
                    {
                        P1->OUT |= BIT0;
                        PCD8544_Puts("You have a fever",0,4);
                    }

                    else if(tempsense == -100)
                    {
                        /* Retake temperature */
                        PCD8544_Puts("Please try again",0,4);

                    }

                    else P1->OUT &= ~BIT0;

                    sprintf(LCD_tempsense, "Body Temperature         %d", tempsense);
                    PCD8544_Puts(LCD_tempsense,0,2);

                    __disable_irq();
                    Enable_Interrupts();
                    break;
               }

                clear_buffer();
                processflag = 0;

                __enable_irq();

            }

            /* Check ultrasonic sensor value every time loop is run
             * Stop the bot if it encounters an obstacle
             */
            Ultrasonic_GetData();
        }

	//}


}

/* Port1 ISR */
void PORT1_IRQHandler(void)
{
    volatile uint16_t i;

    /* Check for interrupt on Pin 1 */
    if(P1->IFG & BIT1)
    {
        P4->OUT ^= BIT1;
        P4->OUT ^= BIT6;
    }

    /* Check for interrupt on Pin 4 */
    else if(P1->IFG & BIT4)
    {
        P4->OUT ^= BIT3;
        P4->OUT ^= BIT7;
    }

    // Delay for switch debounce
    for(i = 0; i < 10000; i++);

    P1->IFG &= ~BIT0;
    P1->IFG &= ~BIT7;
}

/* Port2 ISR */
void PORT2_IRQHandler(void)
{
    __disable_irq();

    /* Check for Echo Interrupt */
    if(P2->IFG & BIT7)
    {
        /* Rising edge interrupt */
        if(!(P2->IES & BIT7))
        {
            /* Clear the timer and start a stopwatch */
            SysTick->VAL = 0;
            //TIMER_A0->CTL |= TIMER_A_CTL_CLR;
            millis = 0;

            /* Configure the interrupt for falling edge */
            P2->IES |=  BIT7;
        }
        else
        {
            /* Falling edge detected, calculate time in microseconds */
            ultra_read = millis*1000 + ((SYSTICK_RELOAD - SysTick->VAL) * 1000)/SYSTICK_RELOAD;

            /* Configure interrupt back to rising edge */
            P2->IES &=  ~BIT7;

        }

        /* Clear IFG */
        P2->IFG &= ~BIT7;
    }

    __enable_irq();
}

/* UART0 ISR */
void EUSCIA0_IRQHandler(void)
{
    /* To store data to be transmitted */
    char temp_buf[1];

    /* Check for RX interrupt */
    if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG)
    {
        /* Mask only lower byte */
        if(cmdptr <= 97)
        command[cmdptr++] = (EUSCI_A0->RXBUF & 0xFF);

        else
        {
            /* Buffer is full, a command has arrived */
            processflag = 1;
            temp_buf[0] = (EUSCI_A0->RXBUF & 0xFF);
        }

        /* Check if the TX buffer is empty first */
        while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));

    }

    /* Check for TX interrupt */
    if((EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG) && (EUSCI_A0->IE & EUSCI_A_IE_TXIE))
    {
        /* If there is data, TX it */
        if(cbfifo_dequeue(temp_buf,1))
        {
            putUART(temp_buf);
        }
        /* Turn off the interrupt */
        else EUSCI_A0->IE &= ~EUSCI_A_IE_TXIE;

    }
}

/*TA1 ISR */
void TA1_0_IRQHandler(void) {

    static uint16_t counter = 0;

    /* 1 second has elapsed */
    if(counter == 1000)
    {
        cur_time++;
        counter = 0;
    }

    counter++;
    /* Clear the IFG and reload timer */
    TIMER_A1->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A1->CCR[0] += 1500;
}

void SysTick_Handler()
{
    __disable_irq();

    static uint16_t counter = 0;
    millis++;

    /* 1 second has elapsed */
    if(counter == 1000)
    {
        cur_time++;
        counter = 0;
    }
    counter++;
    //P4->OUT ^= BIT5;

    __enable_irq();
}

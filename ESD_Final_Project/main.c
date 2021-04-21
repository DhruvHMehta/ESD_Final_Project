#include "msp.h"
#include "cbfifo.h"
#include <string.h>


/**
 * main.c
 */

volatile char command[100];
volatile uint8_t cmdptr, processflag;
volatile uint32_t cur_time,ref_time;

void putUART(const char *_ptr)
{
        EUSCI_A0->TXBUF = (unsigned char) *_ptr;
}

void ClockSource_Init()
{
    CS->KEY = CS_KEY_VAL;                   // Unlock CS module for register access
    CS->CTL0 = 0;                           // Reset tuning parameters
    CS->CTL0 = CS_CTL0_DCORSEL_3;           // Set DCO to 12MHz (nominal, center of 8-16MHz range)
    CS->CTL1 = CS_CTL1_SELA_2 |             // Select ACLK = REFO
               CS_CTL1_SELS_3 |             // SMCLK = DCO
               CS_CTL1_SELM_3;              // MCLK = DCO
    CS->KEY = 0;                            // Lock CS module from unintended accesses
}

void PWM_Init()
{
    /* TA0.1 and TA0.2 */
    P2->DIR |= BIT4 | BIT5;

    /* Pin Mux Settings for TA0.1 and TA0.2 */
    P2->SEL0 |= BIT4 | BIT5;
    P2->SEL1 &= ~(BIT4 | BIT5);

    /* Set Capture and Compare 0 to 10000
     * For SMCLK = 12Mhz, Period = ~8us */
    TIMER_A0->CCR[0] = 10000;

    /* Set Initial Duty Cycle = 55% */
    TIMER_A0->CCR[1] = 5000;
    TIMER_A0->CCR[2] = 5000;

    /* Set Timer to SET/RESET Mode */
    TIMER_A0->CCTL[1] = TIMER_A_CCTLN_OUTMOD_6;
    TIMER_A0->CCTL[2] = TIMER_A_CCTLN_OUTMOD_6;

    /* Select SMCLK Source (3Mhz) and Count Up, Start */
    TIMER_A0->CTL = TIMER_A_CTL_TASSEL_2 |
                    TIMER_A_CTL_MC__UP |
                    TIMER_A_CTL_CLR;
}

/* Pin Setup for Switch Interrupt Mode */
void Switch_Init()
{
    /* Configuring Port 1 Pin 1 and Pin 4 for Switch Operation */
    P1->DIR &= ~(uint8_t) (BIT4 | BIT1);
    P1->OUT |= (BIT1 | BIT4);                // Pull-up
    P1->REN |= (BIT1 | BIT4);                // Enable pull-up resistor (P1.4 output high)
    P1->SEL0 |= 0;                           // I/O Pin Mux
    P1->SEL1 |= 0;                           // I/O Pin Mux
    P1->IES |= (BIT1 | BIT4);                // Interrupt on high-to-low transition
    P1->IFG = 0;                            // Clear all P1 interrupt flags
    P1->IE |= (BIT1 | BIT4);                 // Enable interrupt for P1.1 and P1.4

}

void UART_Init()
{
    // Configure UART pins
    P1->SEL0 |= BIT2 | BIT3;                // set 2-UART pin as secondary function

    // Configure UART
    EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; // Put eUSCI in reset
    EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | // Remain eUSCI in reset
            EUSCI_B_CTLW0_SSEL__SMCLK;      // Configure eUSCI clock source for SMCLK

    // Baud Rate calculation
    // 12000000/(16*115200) = 6.5104
    // Fractional portion = 0.5104
    // User's Guide Table 21-4: UCBRSx = 0x20
    // UCBRFx = int ( (6.5104-6)*16) = 8
    EUSCI_A0->BRW = 6;                     // 12000000/16/115200

    EUSCI_A0->MCTLW = (8 << EUSCI_A_MCTLW_BRF_OFS) | (0x20 << EUSCI_A_MCTLW_BRS_OFS) |
                       EUSCI_A_MCTLW_OS16;

    EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; // Initialize eUSCI
    EUSCI_A0->IFG &= ~(EUSCI_A_IFG_RXIFG);    // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= (EUSCI_A_IE_RXIE | EUSCI_A_IE_TXIE);          // Enable USCI_A0 TX and RX interrupt

}

void TA0_Init()
{
    TIMER_A0->CCTL[0] = TIMER_A_CCTLN_CCIE; // TACCR0 interrupt enabled
    TIMER_A0->CCR[0] = 50000;
    TIMER_A0->CTL = TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_ID__8 | // SMCLK, continuous mode
                    TIMER_A_CTL_MC__CONTINUOUS;
}

void Enable_Interrupts()
{
    /* Enable Port 1 interrupt on the NVIC */
    NVIC->ISER[1] = 1 << ((PORT1_IRQn) & 31);

    /* Enable eUSCIA0 interrupt in NVIC module */
    NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);

    /* Enable TA0 interrupt in NVIC module */
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);
}

void clear_buffer()
{
    __disable_irq();

    for(uint16_t i = 0; i < sizeof(command)/sizeof(char); i++)
    {
        command[i] = 0;
    }
    cmdptr = 0;

    __enable_irq();
}


void main(void)
{
    unsigned int i,j;

	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	ClockSource_Init();
	PWM_Init();
	Switch_Init();
	TA0_Init();

    /* Manual control of Relay */
    P3->DIR |= BIT2 | BIT3 | BIT6 | BIT7;
    P3->OUT &= ~(BIT2 | BIT3 | BIT6 | BIT7);

	UART_Init();

    Enable_Interrupts();

	//while(1)
	//{

	    /*
	    static int i = 0;

	    TIMER_A0->CCR[2] += 100;
	    TIMER_A0->CCR[1] += 100;

	    if(TIMER_A0->CCR[2] == 10000)
	    {
	        TIMER_A0->CCR[2] = 7500;
	    }

        if(TIMER_A0->CCR[1] == 10000)
        {
            TIMER_A0->CCR[1] = 7500;
        }

	    for(i = 0; i < 65535; i++);
	    */

        /* Check if OK */
	    cbfifo_enqueue("AT\r\n",4);
	    EUSCI_A0->IE |= EUSCI_A_IE_TXIE;

	    for(i = 0; i < 65535; i++);

	    if(command[6] == 'O' && command[7] == 'K')
	        clear_buffer();

	    /* Set Current Wifi Mode - SoftAP + Station */
	    cbfifo_enqueue("AT+CWMODE=3\r\n",13);
	    EUSCI_A0->IE |= EUSCI_A_IE_TXIE;

	    for(i = 0; i < 65535; i++);

	    clear_buffer();

        /* Set Static IP */
        cbfifo_enqueue("AT+CIPSTA=\"192.168.43.253\"\r\n",28);
        EUSCI_A0->IE |= EUSCI_A_IE_TXIE;

        for(i = 0; i < 65535; i++);

        clear_buffer();

        /* Allow Multiple Connections */
        cbfifo_enqueue("AT+CIPMUX=1\r\n",13);
        EUSCI_A0->IE |= EUSCI_A_IE_TXIE;

        for(i = 0; i < 65535; i++);

        clear_buffer();

        /* Create TCP Server on Port 80 */
        cbfifo_enqueue("AT+CIPSERVER=1,80\r\n",19);
        EUSCI_A0->IE |= EUSCI_A_IE_TXIE;

        for(i = 0; i < 65535; i++);

        clear_buffer();

        /* Join an Access Point */
        cbfifo_enqueue("AT+CWJAP_CUR=\"OnePlus 6T\",\"password\"\r\n",38);
        EUSCI_A0->IE |= EUSCI_A_IE_TXIE;

        ref_time = cur_time;
        while(cur_time - ref_time <= 10);
        ref_time = cur_time;

        clear_buffer();
        cbfifo_enqueue("AT+CIPSTATUS\r\n",14);
        EUSCI_A0->IE |= EUSCI_A_IE_TXIE;

        if(command[39] == '2')
        clear_buffer();

        ref_time = cur_time;

        while(command[21] != '3')
        {
        clear_buffer();
        cbfifo_enqueue("AT+CIPSTATUS\r\n",14);
        EUSCI_A0->IE |= EUSCI_A_IE_TXIE;
        while(cur_time - ref_time < 1);
        ref_time = cur_time;
        }

        clear_buffer();

        cbfifo_enqueue("AT+CIPSEND=0,541\r\n",18);
        EUSCI_A0->IE |= EUSCI_A_IE_TXIE;

        ref_time = cur_time;
        while(cur_time - ref_time < 2);
        ref_time = cur_time;

       // if(command[25] == '>')
        //{
            cbfifo_enqueue("<!DOCTYPE html><html><body><h1>SanBot Control Page</h1><p>Please choose an option</p><p><b>TURN LEFT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"LEFT\"><button>LEFT</button></a></br></p><p><b>TURN RIGHT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"RIGHT\"><button>RIGHT</button></a></br></p><p><b>MOVE FORWARD</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"FORWARD\"><button>FORWARD</button></a></br></p><p><b>MOVE BACK</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"BACK\"><button>BACK</button></a></br></p></body></html>\r\n", 541);
            EUSCI_A0->IE |= EUSCI_A_IE_TXIE;
            // }

        while(1)
        {
            char* match;

            if(processflag == 1)
            {
                __disable_irq();

                P3->OUT &= ~(BIT2 | BIT3 | BIT6 | BIT7);

                command[cmdptr++] = '\0';

                match = strchr(command, '/');
                match++;

                switch(*match)
               {
                case 'F':
                    P3->OUT |= BIT2;
                    P3->OUT |= BIT6;
                    break;

                case 'B':
                    P3->OUT |= BIT3;
                    P3->OUT |= BIT7;
                    break;

                case 'L':
                    P3->OUT |= BIT3;
                    P3->OUT |= BIT6;
                    break;

                case 'R':
                    P3->OUT |= BIT2;
                    P3->OUT |= BIT7;
                    break;
               }

                clear_buffer();
                processflag = 0;

                __enable_irq();
            }
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
        P3->OUT ^= BIT2;
        P3->OUT ^= BIT6;
    }

    /* Check for interrupt on Pin 4 */
    else if(P1->IFG & BIT4)
    {
        P3->OUT ^= BIT3;
        P3->OUT ^= BIT7;
    }

    // Delay for switch debounce
    for(i = 0; i < 10000; i++);

    P1->IFG &= ~BIT1;
    P1->IFG &= ~BIT4;
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
        else EUSCI_A0->IE &= ~EUSCI_A_IE_TXIE;

    }
}

/*TA0 ISR */
void TA0_0_IRQHandler(void) {

    static uint8_t counter = 0;

    if(counter == 20)
    {
        cur_time++;
        counter = 0;
    }
    counter++;
    TIMER_A0->CCTL[0] &= ~TIMER_A_CCTLN_CCIFG;
    TIMER_A0->CCR[0] += 50000;              // Add Offset to TACCR0
}

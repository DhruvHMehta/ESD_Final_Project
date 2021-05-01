#include "msp.h"
#include "cbfifo.h"
#include "PCD8544_Character.h"
#include "PCD8544_Core.h"
#include "PCD8544_Pixel.h"
#include "spi.h"
#include "adxl345.h"
#include "mlx90614.h"
#include "HCSR04.h"
#include "peripherals.h"
#include "Timers.h"
#include <string.h>
#include <stdio.h>

/**
 * main.c
 *
 * Brief : The application entry point to a remotely operated
 *         bot which is controlled over a local network using
 *         a webpage. The ESP8266 is connected to an access
 *         point using WiFi and exchanges data with the MSP432
 *         over UART.
 *
 *         Bot commands are send over the webpage to move the
 *         bot in different directions, change speed, check
 *         the temperature of an object and display it on an
 *         LCD display. An ultrasonic sensor is used for object
 *         detection. An accelerometer is used for bump and fall
 *         detection.
 */


/* Buffer to receive data */
volatile char command[100];
volatile uint8_t cmdptr, processflag;

/* AT Commands List */
static char* AT = "AT\r\n";
static char* CWMODE = "AT+CWMODE=3\r\n";
static char* STATIC_IP = "AT+CIPSTA=\"192.168.43.253\"\r\n";
static char* MULTIPLE_CON_ON = "AT+CIPMUX=1\r\n";
static char* TCP_CREATE = "AT+CIPSERVER=1,80\r\n";
static char* JOIN_AP = "AT+CWJAP_CUR=\"OnePlus 6T\",\"password\"\r\n";
static char* CONN_STATUS = "AT+CIPSTATUS\r\n";
static char* IP_DATALEN = "AT+CIPSEND=0,1329\r\n";

/* Transmits data over UART */
void putUART(const char *_ptr)
{
        EUSCI_A0->TXBUF = (unsigned char) *_ptr;
}

/*
 * @name clear_buffer
 *
 * @brief Clears the receive buffer of data
 *
 * @param none
 * @return none
 */
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

/*
 * @name ESP_Send
 *
 * @brief Enqueues data onto the transmit buffer to send
 *        to the ESP and turns on transmit interrupts
 *
 * @param char* ATCmd - AT Command to send
 * @param uint16_t len - Length of command in bytes
 * @return none
 */
void ESP_Send(char* ATCmd, uint16_t len)
{
    /* Enqueue data onto transmit buffer and enable transmit intr */
    cbfifo_enqueue(ATCmd, len);
    EUSCI_A0->IE |= EUSCI_A_IE_TXIE;
}

/*
 * @name ESP_Init
 *
 * @brief Initializes the ESP8266 and connects to an AP
 *
 * @param none
 * @return none
 */
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
    /* stop watchdog timer */
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	/* Initialize all modules */
	ClockSource_Init();
	PWM_Init();
	Switch_Init();
	GPIO_Init();
	SysTick_Init();
    I2C_Init();
    Ultrasonic_Init();
	UART_Init();
	LCD_Init();
    ADXL345_SPI_Init();
    ADXL345_GPIO_Init();
    ADXL345_Init();

	/* Enable Interrupts and configure ESP */
    Enable_Interrupts();
    ESP_Init();

    /* Check Connection Status */
    ESP_Send(CONN_STATUS, strlen(CONN_STATUS));

    /* No device connected */
    if(command[39] == '2')
    clear_buffer();

    /* Loop until a device is connected
     * Check the Connection Status every
     * 1 second.
     */
    while(command[21] != '3')
    {
    clear_buffer();
    ESP_Send(CONN_STATUS, strlen(CONN_STATUS));
    delay_timer(1);
    }
    clear_buffer();


    /* Device connected, send DataLength */
    ESP_Send(IP_DATALEN, strlen(IP_DATALEN));
    delay_timer(2);

    /* Send the webpage from MSP to ESP over UART */
    cbfifo_enqueue("<!DOCTYPE html><html><head><style>body {background-color:#eedc9b;}</style></head><body><h1>SanBot Control Page</h1>"
            "<p>Please choose an option</p>"
            "<p><b>TURN LEFT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"LEFT\"><button>LEFT</button></a></br></p>"
            "<p><b>TURN RIGHT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"RIGHT\"><button>RIGHT</button></a></br></p>"
            "<p><b>MOVE FORWARD</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"FORWARD\"><button>FORWARD</button></a></br></p>"
            "<p><b>MOVE BACK</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"BACK\"><button>BACK</button></a></br></p>"
            "<p><b>HALT BOT</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"HALT\"><button>HALT</button></a></br></p>"
            "<p><b>Check Temperature</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"TEMP\"><button>TEMP</button></a></br></p>"
            "<p><b>Sanitize</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"SANITIZE\"><button>SANITIZE</button></a></br></p>"
            "<p><b>UV OFF</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"OFF\"><button>OFF</button></a></br></p>"
            "<p><b>Speed Control</b>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"+\"><button>+</button></a>&nbsp;<a href=\"-\"><button>-</button></a></br></p></body></html>\r\n", 1329);

    EUSCI_A0->IE |= EUSCI_A_IE_TXIE;


    /* Enter infinite loop
     *
     * Check for a new command received
     * Poll the ultrasonic sensor for object detection
     *
     *  */
        while(1)
        {
            /* To match the command and store temperature value */
            char* match;
            char LCD_tempsense[25];
            int16_t tempsense;

            /* A command has arrived */
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

                /* Halt Bot */
                case 'H':
                    P4->OUT &= ~(BIT0 | BIT1 | BIT6 | BIT7);
                    break;

                /* Turn on Sanitize LED */
                case 'S':
                    P2->OUT |= BIT2;
                    break;

                /* Turn off Sanitize LED */
                case 'O':
                    P2->OUT &= ~BIT2;
                    break;

                /* Increase speed of bot */
                case '+':
                    if(TIMER_A0->CCR[2] <= 9000)
                        TIMER_A0->CCR[2] += 1000;
                    break;

                /* Reduce speed of bot */
                case '-':
                    if(TIMER_A0->CCR[2] >= 6000)
                        TIMER_A0->CCR[2] -= 1000;
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

                    /* Clear display */
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

}

/* UART0 ISR
 * Dequeues data from transmit buffer and sends it to the ESP over UART
 * Enqueues data onto the receive buffer and sets a flag to notify buffer full
 *  */
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

/* Port1 ISR
 * Debug switches SW1, SW4
 *  */
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

/* Port 4 ISR
 * Accelerometer Bump/Fall interrupt
 * */
void PORT4_IRQHandler()
{


  if(P4->IFG & BIT4)
  {
    /* clear interrupt */
    uint32_t i;
    for(i = 0; i < 10000; i++);

    ADXL345_SPI_Read(ADXL345_INT_SOURCE);
    P4->IFG &= ~(BIT4);

    /*
     *
     * Rough Surface Detected
     *
     * */
    PCD8544_Clear();
    PCD8544_Puts("Caution : Rough  Surface Detected",0,1);

  }


  if(P4->IFG & BIT5)
  {
    P4->IFG &= ~(BIT5);

    /*
      *
      * Bot is Free Falling
      *
      * */

  }
}




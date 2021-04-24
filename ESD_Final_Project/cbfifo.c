/************************************************************************
 * Copyright (C)​ 2021 by Dhruv Mehta
 *
 * ​Redistribution, ​​modification ​​or ​​use ​​of ​​this ​​software ​​in​​source ​​or ​​binary
 * forms ​​is ​​permitted ​​as ​​long​​ as ​​the ​​files​​ maintain ​​this ​​copyright.
 *
 * Users ​​are​​ ​​permitted ​​to​​ modify ​​this ​​and ​​use​​ it ​​to​​ learn​​ about​​ the​​ field
 * ​​of ​​embedded​​ ​​software.
 *
 * ​Dhruv Mehta and the University of Colorado are not liable for any misuse
 *  of this material​​.
 *
 ************************************************************************/

/* @file    cbfifo.c
 *
 * @brief   Contains an implementation for a circular buffer FIFO.
 *
 * @tools   GCC compiler v8.1.0 / Visual Studio Code IDE
 * @author  Dhruv Mehta
 * @date    02/07/2021
 * @version 1.0
 *
 ************************************************************************/
#include <stdint.h>
#include "cbfifo.h"

#define SIZE (2048)                          /* Size of Buffer */

struct Buffer{
uint8_t CirBuf[SIZE];                       /* Buffer */
uint16_t wrloc;                              /* Write Location (where to write next) */
uint16_t rdloc;                              /* Read Location (where to read from next) */
uint16_t size;                               /* Size of Buffer, fixed by SIZE macro */
};

struct Buffer Cbfifo = {.size = SIZE};

static size_t cbfifo_freespace();

static size_t cbfifo_freespace()
{
    return (Cbfifo.size - cbfifo_length() - 1); /* Total size minus number of occ elements - 1 */
}

size_t cbfifo_enqueue(void *buf, size_t nbyte)
{
    if(buf == NULL)
    {
        return -1;                              /* Input buffer is NULL */
    }
    uint16_t enq_size = 0;
    uint16_t freespace = cbfifo_freespace();

    if(nbyte > freespace)
    {
        nbyte = freespace;                      /* Cannot write more than available space */
    }
    if(nbyte == 0)
    {
        return 0;                               /* Returns 0 on no write operation */
    }

    for(uint16_t i = 0; i < nbyte; i++)
    {
        Cbfifo.CirBuf[Cbfifo.wrloc] = *((uint8_t *)buf);        /* Write to buffer */
        (uint8_t *)buf++;
        enq_size++;
        Cbfifo.wrloc = (Cbfifo.wrloc + 1) & (Cbfifo.size - 1); /* Wrapped addition of wrloc */
    }

    return enq_size;
}

size_t cbfifo_dequeue(void *buf, size_t nbyte)
{
    if(buf == NULL)
    {
        return -1;                                /* Input buffer is NULL */
    }
    uint16_t deq_size = 0;
    uint16_t length = cbfifo_length();

    if(nbyte > length)
    {
        nbyte = length;                           /* Cannot fetch more than available length */
    }

    if(nbyte == 0)
    {
        return 0;                               /* Returns 0 on no read operation */
    }
    for(int i = 0; i < nbyte; i++)
    {
        *((uint8_t *)buf) = Cbfifo.CirBuf[Cbfifo.rdloc];        /* Read from buffer */
        buf++;
        deq_size++;

        Cbfifo.rdloc = (Cbfifo.rdloc + 1) & (Cbfifo.size - 1);  /* Wrapped addition of rdloc */
    }

    return deq_size;
}

size_t cbfifo_length()
{
    return ((Cbfifo.wrloc - Cbfifo.rdloc) & (Cbfifo.size - 1)); /* Wrapped subtraction of wrloc and rdloc */
}

size_t cbfifo_capacity()
{
    return SIZE-1;                                              /* Constant value, restricted to SIZE - 1 */
}

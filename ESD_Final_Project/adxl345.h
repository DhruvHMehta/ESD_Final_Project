/*
 * adxl345.h
 *
 */

#ifndef ADXL345_H_
#define ADXL345_H_

#include "msp.h"
#include "spi.h"

/* ADXL345 Register Definitions
 *
 * Source - ADXL345 Datasheet ,Table 16. Register MAP
 *
 */
#define ADXL345_THRESH_ACT     (0x24)
#define ADXL345_ACT_INACT_CTL  (0x27)
#define ADXL345_THRESH_FF      (0x28)
#define ADXL345_TIME_FF        (0x29)
#define ADXL345_POWER_CTL      (0x2D)
#define ADXL345_INT_ENABLE     (0x2E)
#define ADXL345_INT_MAP        (0x2F)
#define ADXL345_INT_SOURCE     (0x30)
#define ADXL345_DATA_FORMAT    (0x31)

void ADXL345_Init();

void ADXL345_GPIO_Init();

void ADXL345_Port4_Interrupt_Enable();


#endif /* ADXL345_H_ */

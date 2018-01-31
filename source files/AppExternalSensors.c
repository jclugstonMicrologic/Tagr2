//###########################################################################
//
// FILE:   AppExternalSensors.c
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 30 JAN 2018 | JDC  | Original version 
//###########################################################################


#include "stm32f4xx_i2c.h"
#include "AppExternalSensors.h"

void InitExtSensors(void)
{  
    I2C_InitTypeDef I2C_InitStructure;
    
    I2C_Init(I2C1, &I2C_InitStructure);
}

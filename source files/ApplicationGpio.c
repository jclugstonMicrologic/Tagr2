
//###########################################################################
//
// FILE:   ApplicationGpio.c
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 31 JAN 2018 | JDC  | Original version 
//###########################################################################


#include "Tagr.h"

GPIO_InitTypeDef  GPIO_InitStructure;

void InitGpio
(
    void
)
{   
    /* enable the GPIO Clocks */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);       
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);       
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);     
    
    /* configure the following GPIO pins as outputs */    
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_3;
    GPIO_Init(GPIOE, &GPIO_InitStructure);              
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOF, &GPIO_InitStructure);          
    
    /* configure the following GPIO pins as inputs */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);              
    
    /* configure the following GPIO pins as inputs */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);                  
    
}

void AssertStatusLed
(
    Int8U led
)
{
    switch( led )
    {
        case 0:      
            STATUS_LED1_ON;                                    
            break;
        case 1:      
            STATUS_LED2_ON;                                    
            break;
        case 2:      
            CHMM_STATUS_LED_ON;                                    
            break;
        case 3:      
            UPLOAD_STATUS_LED_ON;                                    
            break;
        case 4:      
            DONE_STATUS_LED_ON;                                    
            break;
        case 5:      
            SPARE_STATUS_LED_ON;                                    
            break;                  
    }
}

void NegateStatusLed
(
    Int8U led
)
{
    switch( led )
    {
        case 0:      
            STATUS_LED1_OFF;                                    
            break;
        case 1:      
            STATUS_LED2_OFF;                                   
            break;
        case 2:      
            CHMM_STATUS_LED_OFF;                  
            break;
        case 3:      
            UPLOAD_STATUS_LED_OFF;                                    
            break;
        case 4:      
            DONE_STATUS_LED_OFF;                                    
            break;
        case 5:      
            SPARE_STATUS_LED_OFF;                                    
            break;                  
    }  
}


    


//###########################################################################
//
// FILE:   ApplicationGpio.h
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 31 JAN 2018 | JDC  | Original version 
//###########################################################################

#ifndef APPLICATION_GPIO_H
#define APPLICATION_GPIO_H

#define STATUS_LED1         GPIO_Pin_9
#define STATUS_LED2         GPIO_Pin_10
#define DONE_STATUS_LED     GPIO_Pin_8
#define UPLOAD_STATUS_LED   GPIO_Pin_7
#define CHMM_STATUS_LED     GPIO_Pin_6
#define SPARE_STATUS_LED    GPIO_Pin_11

#define STATUS_LED1_ON          GPIO_SetBits(GPIOF, STATUS_LED1)
#define STATUS_LED1_OFF         GPIO_ResetBits(GPIOF, STATUS_LED1)

#define STATUS_LED2_ON          GPIO_SetBits(GPIOF, STATUS_LED2)
#define STATUS_LED2_OFF         GPIO_ResetBits(GPIOF, STATUS_LED2)

#define CHMM_STATUS_LED_ON      GPIO_SetBits(GPIOF, CHMM_STATUS_LED)
#define CHMM_STATUS_LED_OFF     GPIO_ResetBits(GPIOF, CHMM_STATUS_LED)

#define UPLOAD_STATUS_LED_ON    GPIO_SetBits(GPIOF, UPLOAD_STATUS_LED)
#define UPLOAD_STATUS_LED_OFF   GPIO_ResetBits(GPIOF, UPLOAD_STATUS_LED)

#define DONE_STATUS_LED_ON      GPIO_SetBits(GPIOF, DONE_STATUS_LED)
#define DONE_STATUS_LED_OFF     GPIO_ResetBits(GPIOF, DONE_STATUS_LED)

#define SPARE_STATUS_LED_ON     GPIO_SetBits(GPIOF, SPARE_STATUS_LED)
#define SPARE_STATUS_LED_OFF    GPIO_ResetBits(GPIOF, SPARE_STATUS_LED)

#define ALL_LEDS_ON             STATUS_LED1_ON; STATUS_LED2_ON; \
                                UPLOAD_STATUS_LED_ON; DONE_STATUS_LED_ON 
        
#define ALL_LEDS_OFF            STATUS_LED1_OFF; STATUS_LED2_OFF; \
                                UPLOAD_STATUS_LED_OFF; DONE_STATUS_LED_OFF 
    
        
void InitGpio(void);
void AssertStatusLed(Int8U led);
void NegateStatusLed(Int8U led);


#endif



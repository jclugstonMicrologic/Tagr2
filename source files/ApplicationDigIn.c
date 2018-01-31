//###########################################################################
// NRE Electronics Copyright 2005
//
//	COMMON SOURCE FILE
//
// FILE:   
//
// TITLE:  function definitions
//
//	Note that corrections are applied without incrementing the rev number
//  while updates/additions result in a rev number increment
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 14 AUG 2012 | JDC  | 
//      |             |      | 
// 	|	      |      | 
//###########################################################################


/*
*****************************************************************************
 L O C A L   I N C L U D E   F I L E S
*****************************************************************************
*/
#include "Tagr.h"


/*
*****************************************************************************
 L O C A L    T Y P E    D E F I N I T I O N S
*****************************************************************************
*/

/*
*****************************************************************************
 L O C A L    M A C R O S
*****************************************************************************
*/

/*
*****************************************************************************
 P U B L I C   D A T A
*****************************************************************************
*/
DIGITAL_IN_DATA DigitalInData[MAX_NUM_DIG_IN];

/*
*****************************************************************************
 P R I V A T E   G L O B A L   D A T A
*****************************************************************************
*/
Int8U Usb1OverCurrent;
Int8U Usb2OverCurrent;
Int8U Fault3VRstFlag;
Int8U FaultReg1RstFlag;

/*
*****************************************************************************
 P R I V A T E   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/


/*
*****************************************************************************
 P R O C E D U R E S
*****************************************************************************
*/


/*
*|----------------------------------------------------------------------------
*|  Module: ApplicationDigIn
*|  Routine:  InitDigitalIn
*|  Description:
*|   Initialises associated software and hardware.  It sets up the callback
*|   functions that may be invoked too.
*|----------------------------------------------------------------------------
*/
void InitDigitalIn
(
    void
)
{
   Int8U j;
   
   DigitalInData[FAULT_USB1].digInCallBackPtr =GetDigin1Status; 
   DigitalInData[FAULT_USB1].activeState =0;
   DigitalInData[FAULT_USB1].callBackPtr1 =FaultUsb1Negate;
   DigitalInData[FAULT_USB1].callBackPtr2 =FaultUsb1;
   DigitalInData[FAULT_USB1].callBackPtr3 =NULL;
   DigitalInData[FAULT_USB1].callBackPtr4 =NULL;
   
   DigitalInData[FAULT_USB2].digInCallBackPtr =GetDigin1Status; 
   DigitalInData[FAULT_USB2].activeState =0;   
   DigitalInData[FAULT_USB2].callBackPtr1 =FaultUsb2Negate;
   DigitalInData[FAULT_USB2].callBackPtr2 =FaultUsb2;
   DigitalInData[FAULT_USB2].callBackPtr3 =NULL;
   DigitalInData[FAULT_USB2].callBackPtr4 =NULL;

   DigitalInData[FAULT_3V_RST].digInCallBackPtr =GetDigin2Status; 
   DigitalInData[FAULT_3V_RST].activeState =0;   
   DigitalInData[FAULT_3V_RST].callBackPtr1 =Fault3VRstNegate;
   DigitalInData[FAULT_3V_RST].callBackPtr2 =NULL;
   DigitalInData[FAULT_3V_RST].callBackPtr3 =Fault3VRst;
   DigitalInData[FAULT_3V_RST].callBackPtr4 =NULL;
   
   DigitalInData[FAULT_REG1_RST].digInCallBackPtr =GetDigin2Status; 
   DigitalInData[FAULT_REG1_RST].activeState =0;   
   DigitalInData[FAULT_REG1_RST].callBackPtr1 =FaultReg1RstNegate;
   DigitalInData[FAULT_REG1_RST].callBackPtr2 =NULL;
   DigitalInData[FAULT_REG1_RST].callBackPtr3 =FaultReg1Rst;
   DigitalInData[FAULT_REG1_RST].callBackPtr4 =NULL;

   
   for(j =0; j<MAX_NUM_DIG_IN; j++)
   {
      DigitalInData[j].bounce = 0;
      DigitalInData[j].start  = 0;
      DigitalInData[j].machineState = SWITCH_IDLE_STATE;
   }

   Usb1OverCurrent =0;
   Usb2OverCurrent =0;
   Fault3VRstFlag   =0;
   FaultReg1RstFlag =0;
} // InitDigitalIn()


/*
*|----------------------------------------------------------------------------
*|  Module: ApplicationDigIn
*|  Routine:  DigitalInMachine
*|  Description:
*|    The state machine that detects the condition of the switches.
*|----------------------------------------------------------------------------
*/
void DigitalInMachine
(
   DIGITAL_IN_DATA *pDigIn,
   Int8U switchId
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   int id;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */


   /*
   *************************
    C O D E
   *************************
   */
   id = switchId;

   switch( pDigIn->machineState )
   {
         case SWITCH_IDLE_STATE:
            pDigIn->machineState = NORMAL_NEGATED_STATE;
            break;
         case NORMAL_NEGATED_STATE:
            /* switch is negated */
            if( pDigIn->digInCallBackPtr !=NULL )           
            {
                if( pDigIn->digInCallBackPtr(id) )
                {
                    /* switch looks asserted, start debounce counter */
                    pDigIn->start =TimerTicks;
                    pDigIn->machineState = DEBOUNCE_ASSERTING_STATE;
                }
            }
            break;
         case DEBOUNCE_ASSERTING_STATE:
            /* switch may be changing to asserted state, wait for debounce */
            if( !pDigIn->digInCallBackPtr(id) )
            {
               /* switch not asserted after all */
               pDigIn->machineState = NORMAL_NEGATED_STATE;
            }
            else if( (TimerTicks -pDigIn->start) >BOUNCE_COUNTS )
            {
               /* debounced, switch is asserted */
               pDigIn->start =TimerTicks;
               pDigIn->machineState = ASSERTED_STATE;
            }
            break;
         case ASSERTED_STATE:
            /* switch has just been asserted, measure how long it is asserted */
            if( !pDigIn->digInCallBackPtr(id) )
            {
               pDigIn->start = 0;
               /* switch looks like its being negated */
               pDigIn->machineState = DEBOUNCE_NEGATING_STATE;
            }
            else if( (TimerTicks - pDigIn->start) >BOUNCE_COUNTS )
            {
               /* switch just detected as being held asserted */
               /* invoke callback */
               if( pDigIn->callBackPtr3 != NULL )
               {
                  pDigIn->callBackPtr3();
               }

               pDigIn->start =TimerTicks;
               pDigIn->assertCounts =0;
               pDigIn->machineState = HELD_ASSERTED_STATE;
            }
            break;
         case DEBOUNCE_NEGATING_STATE:
            /* switch may be negating */
            if( pDigIn->digInCallBackPtr(id) )
            {
               /* bounce detected */
               pDigIn->start =0;               
               pDigIn->machineState = ASSERTED_STATE;
            }
            else if( (TimerTicks - pDigIn->start) >BOUNCE_COUNTS )              
            {
               /* switch went to negated state */
               /* invoke callback */
               if( pDigIn->callBackPtr1 != NULL )
               {
                  pDigIn->callBackPtr1();
               }

               pDigIn->machineState = NORMAL_NEGATED_STATE;
            }
            break;
         case HELD_ASSERTED_STATE:
            /* switch is being held in asserted */
            if( !pDigIn->digInCallBackPtr(id) )
            {
               /* switch looks like its negating */
               pDigIn->start = 0;
               pDigIn->machineState = DEBOUNCE_RELEASING_HELD_STATE;
            }
            else if( (TimerTicks - pDigIn->start) >pDigIn->bounce )
            {
               /* invoke callback periodically since switch held asserted */
               if( pDigIn->callBackPtr2 != NULL )
               {
                  pDigIn->callBackPtr2();
               }
               pDigIn->start =TimerTicks;
               
               pDigIn->machineState =HELD_ASSERTED_STATE2;
               
            }
            break;
         case HELD_ASSERTED_STATE2:            
            // this state prevents the DEBOUNCE_RELEASING_HELD_STATE from being
            // run, therefore preventing its callback from being invoked 
           
            /* switch is being held in asserted */
            if( !pDigIn->digInCallBackPtr(id) )
            {
               /* switch looks like its negating */
               pDigIn->start = 0;
               pDigIn->machineState = DEBOUNCE_RELEASING_HELD_STATE2;
            }
            else if( (TimerTicks - pDigIn->start) >pDigIn->bounce )
            {
               /* invoke callback periodically since switch held asserted */
               if( pDigIn->callBackPtr2 != NULL )
               {
                  pDigIn->callBackPtr2();
               }
               pDigIn->start =TimerTicks;
               
               pDigIn->machineState =HELD_ASSERTED_STATE2;
               
               if( ++pDigIn->assertCounts >HELD_ASSERT_COUNTS )
                  pDigIn->machineState =HELD_ASSERTED_NEW_RATE_STATE;
            }           
            break;
         case HELD_ASSERTED_NEW_RATE_STATE:
            /* switch is being held in asserted */
            if( !pDigIn->digInCallBackPtr(id) )
            {
               /* switch looks like its negating */
               pDigIn->start = 0;
               pDigIn->machineState = DEBOUNCE_RELEASING_HELD_STATE2;
            }
            else if( (TimerTicks-pDigIn->start) >pDigIn->bounce ) 
            {
               /* invoke callback periodically since switch held asserted */
               if( pDigIn->callBackPtr2 != NULL )
               {
                  // maybe add a new callback ( *callBackPtr5 )
                  pDigIn->callBackPtr2();
                  
                  pDigIn->start =TimerTicks;
               }
            }
            break;
         case DEBOUNCE_RELEASING_HELD_STATE:
         case DEBOUNCE_RELEASING_HELD_STATE2:
            /* switch may be negating */
            if( pDigIn->digInCallBackPtr(id) )
            {
               /* bounce detected */
               if( pDigIn->assertCounts >=HELD_ASSERT_COUNTS )
                  pDigIn->machineState =HELD_ASSERTED_NEW_RATE_STATE;
               else
                  pDigIn->machineState =HELD_ASSERTED_STATE;

               pDigIn->start += pDigIn->bounce;
            }
            else if( (TimerTicks-pDigIn->start) >BOUNCE_COUNTS )                 
            {
               /* switch went to negated state */
               /* invoke callback for 'held' state being exited */
               if( pDigIn->callBackPtr4 != NULL &&
                   pDigIn->machineState ==DEBOUNCE_RELEASING_HELD_STATE
                 )
               {
                  pDigIn->callBackPtr4();
               }
               else if( pDigIn->callBackPtr1 !=NULL )
               {
                  pDigIn->callBackPtr1(); 
               }
                 

               pDigIn->machineState = NORMAL_NEGATED_STATE;
            }
            break;
         default:
            break;
      }

} // end DigitalInMachine()



/*
*|----------------------------------------------------------------------------
*|  Module: ApplicationDigIn
*|  Routine:  GetDigin1Status
*|  Description: Monitor PORTC digital in 
*|
*|----------------------------------------------------------------------------
*/
Int8U GetDigin1Status
(
    Int8U digIn
)
{

    Int16U digInMask;

    if( digIn ==0 )    
        digInMask =(1<<0);
    else if( digIn ==1 )    
        digInMask =(1<<8);   
    else if( digIn ==5 )    
        digInMask =(1<<9);       
                
    if( GPIO_ReadInputDataBit(GPIOC, digInMask) ==DigitalInData[digIn].activeState )        
        return true;
    else
        return false; 
}

/*
*|----------------------------------------------------------------------------
*|  Module: ApplicationDigIn
*|  Routine:  GetDigin2Status
*|  Description: Monitor PORTA digital in 
*|
*|----------------------------------------------------------------------------
*/
Int8U GetDigin2Status
(
    Int8U digIn
)
{

    Int16U digInMask;

    if( digIn ==2 )    
        digInMask =(1<<5);
    else if( digIn ==3 )    
        digInMask =(1<<6);    
    else if( digIn ==4 )
        digInMask =(1<<8);        
                
    if( GPIO_ReadInputDataBit(GPIOA, digInMask) ==DigitalInData[digIn].activeState )        
        return true;
    else
        return false; 
}



void FaultUsb1(void)
{
    Usb1OverCurrent =1;  
//    ALL_LEDS_ON;
}

void FaultUsb1Negate(void)
{
    Usb1OverCurrent =0;  
//    ALL_LEDS_OFF;  
}

void FaultUsb2(void)
{
    Usb2OverCurrent =1;    
//    ALL_LEDS_ON;
}

void FaultUsb2Negate(void)
{
    Usb2OverCurrent =0;  
    //ALL_LEDS_OFF;  
}

void Fault3VRst(void)
{
   Fault3VRstFlag =1;
}

void Fault3VRstNegate(void)
{
   Fault3VRstFlag =0;  
}

void FaultReg1Rst(void)
{
   FaultReg1RstFlag =1;  
}

void FaultReg1RstNegate(void)
{
   FaultReg1RstFlag =0;  
}

/* ApplicationDigIn.c */
















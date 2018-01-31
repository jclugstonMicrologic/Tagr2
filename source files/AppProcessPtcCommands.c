//###########################################################################
//
// FILE:   AppPorcessPtcCommands.c
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 30 MAY 2016 | JDC  | Original version 
//###########################################################################

#include "CommonOsTimer.h"

#include "ApplicationGpio.h"
#include "AppProcessPtcCommands.h"

/**********************************************************************
	Function: void ProcessPtcCommand(char *)
 	Description: 
**********************************************************************/
Int8U ProcessPtcCommand
(
   char *pdata,
   Int8U len
)
{    
    char cmd;
    Int8U length;
    
    cmd = *pdata;
    
    length =len;
    
    switch( cmd ) 
    {
        case 0:
	    break;			
        case '1':            
            DONE_STATUS_LED_OFF;
            break;            
        case '2':
            DONE_STATUS_LED_ON;
            break;
        case '#':
            *pdata++ =((TimerTicks &0x000000ff));
            *pdata++ =((TimerTicks &0x0000ff00)>>8);
            *pdata++ =((TimerTicks &0x00ff0000)>>16);      
            *pdata++ =((TimerTicks &0xff000000)>>24);                      
           
            length =sizeof(TimerTicks);
            
            DONE_STATUS_LED_ON;
            break;
	default:
            // leave as is, will echo back to client
            DONE_STATUS_LED_OFF;
    	    break;
    }	
	
    return length;
    
} // end ProcessPtcCommands()


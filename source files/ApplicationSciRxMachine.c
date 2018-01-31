//###########################################################################
//
// FILE:   NFCOMMON_SciRxMachine.c
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 30 AUG 2012 | JDC  | Original version 
//###########################################################################


#include "Tagr.h"

#define SERIAL_TIMEOUT_500MSEC (500)  //500msec
#define SERIAL_TIMEOUT_100MSEC (100)  //100msec
#define SERIAL_TIMEOUT_1SEC    (1000) //500msec

// states of the terminal receive machine
enum
{
    SERIAL_RX_IDLE_STATE =0,
    SERIAL_RX_HEADER_STATE,
    SERIAL_RX_PAYLOAD_STATE,

    SERIAL_RX_PASS_THRU_STATE,
  	       
    SERIAL_LAST_STATE   
};

SERIAL_DATA SerialDataCom1;
SERIAL_DATA SerialDataCom2;


void InitSerialData
(
    SERIAL_DATA *serialDataPtr
)
{
    serialDataPtr->common.machState =SERIAL_RX_IDLE_STATE;
    serialDataPtr->common.timeoutTimer =0;	  
    serialDataPtr->errorCnt =0;

    memset( serialDataPtr->rxBuffer, 0x00, sizeof(serialDataPtr->rxBuffer) );
}

void InitSerialReceiver
(	
    SERIAL_DATA *serialDataPtr,
    COM_TypeDef sciPort,
    Int32U baudRate
)
{
    InitSerialData(serialDataPtr);
    
    switch(sciPort)
    {
        case COM1:
            serialDataPtr->callBackPtr =ProcessCommandCom1;
            break;
        case COM2:
            serialDataPtr->callBackPtr =ProcessCommandCom2;
            break;            
    }
    
    SciInitSerialPort(sciPort, baudRate );
}


//###########################################################################
// Bool SerialCheckTimeout()
//---------------------------------------------------------------------------
// check the calling machines timeout conditions
//---------------------------------------------------------------------------
Int16U SerialCheckTimeout
(
    COMMON *commonDataPtr,	
    Int32U timeout  //msec
)
{
   if( (TimerTicks - commonDataPtr->timeoutTimer) >timeout )
   {
      return TRUE;
   }
   
   return FALSE;   
} // end SerialCheckTimeout()


//###########################################################################
// void SerialStateProcess()
//---------------------------------------------------------------------------
// move the calling machines state, and reset timeoutTimer
//---------------------------------------------------------------------------
void SerialStateProcess
(
    COMMON *commonDataPtr,
    Int16S state
)
{
    if( state == -1 ) // go to next state
        commonDataPtr->machState ++;
    else             // go to specific state
        commonDataPtr->machState =state;
		
    commonDataPtr->timeoutTimer =TimerTicks;
	
} // end SerialStateProcess()


//###########################################################################
// void SerialRxMachine
//
// serial protocol
// <DLE><STX><NbrBytesMSB><NbrBytesLSB><CommandMSB><CommandLSB><PAYLOAD><crcMSB><crcLSB>
//
//---------------------------------------------------------------------------
int SerialRxMachine
(
    SERIAL_DATA *serialDataPtr,
    char sciPort
)
{
#ifdef ENABLE_TERMINAL
  	static Uint16 SetWhat =0;
	Uint16 SetStatus, SetData;
#endif	  	

    static Int32U SciResetTimer[2] ={0, 0};
    static Int16U RxMachSubState[2] ={0, 0};    
    
    char rxByte =NULL;    
    int status =0;    	     
    Int16U calcCrc, rxCrc;
               
    switch( serialDataPtr->common.machState )
    {
        case SERIAL_RX_IDLE_STATE:
            if( SciGetByte( sciPort,&rxByte ) )	
	    {
                if( rxByte ==SERIAL_HEAD1 ) //DLE
                {
		    serialDataPtr->byteCnt =0;					
        	    RxMachSubState[sciPort] =0;
		    serialDataPtr->rxBuffer[serialDataPtr->byteCnt++] =rxByte;
		    SerialStateProcess( &serialDataPtr->common, -1);
		}

	        SciResetTimer[sciPort] =TimerTicks;
	    }
	    else if( (TimerTicks -SciResetTimer[sciPort]) >50 ) // seconds
	    {
    	        SciResetTimer[sciPort] =TimerTicks;
	    }
    	    break;
	case SERIAL_RX_HEADER_STATE:
	    if( serialDataPtr->common.machState !=SERIAL_RX_HEADER_STATE )
	        return status;

    	    while( serialDataPtr->byteCnt <SIZEOF_HEAD )
	    {
	        while( SciGetByte( sciPort,&rxByte ) )
	        {
		    serialDataPtr->rxBuffer[serialDataPtr->byteCnt] =rxByte;
					
         	    switch( RxMachSubState[sciPort] )
	            {
	                case 0:
	        	    if( rxByte !=SERIAL_HEAD2 )
	        	    {
			        // error
		        	serialDataPtr->common.machState =0;
		        	status = -1;	        				
	        	    }
	        	    break;
	        	case 1: // get number bytes in packet MSB
	        	    serialDataPtr->nbrBytes = (rxByte<<8);							        			
	        	    break;
	        	case 2: // get number bytes in packet LSB
	        	    serialDataPtr->nbrBytes |= rxByte;	        			
		            break;
	        	case 3: // get command byte MSB
			    serialDataPtr->command =( (rxByte<<8) &0xff00);
	        	    break;	        				        					        				
	        	case 4: // get command byte LSB
			    serialDataPtr->command |=rxByte;	
			    SerialStateProcess( &serialDataPtr->common, -1 );
	        	    break;	        				  	        				
	            }
	        			
    		    if( ++serialDataPtr->byteCnt >=SIZEOF_HEAD )
                        break;
                	
	             RxMachSubState[sciPort] ++;
                }
	        	
                if( SerialCheckTimeout( &serialDataPtr->common, SERIAL_TIMEOUT_100MSEC) )
                {
                    status = -1;
                    break;
                }   
	    }
// purposely fall into next state			
	case SERIAL_RX_PAYLOAD_STATE: // get remaining bytes			
            while( SciGetByte( sciPort,&rxByte ) )
            {
		serialDataPtr->rxBuffer[serialDataPtr->byteCnt++] =rxByte;

		if( serialDataPtr->byteCnt ==serialDataPtr->nbrBytes)
		{
		    // termination bytes found
                    // check rx CRC with calculated CRC
					
                    // calculate the CRC on received data
                    calcCrc = crc((unsigned char *)serialDataPtr->rxBuffer, (serialDataPtr->byteCnt-2), 0xffff);
					
                    // get the received CRC
                    rxCrc =(serialDataPtr->rxBuffer[serialDataPtr->byteCnt-2]<<8) | serialDataPtr->rxBuffer[serialDataPtr->byteCnt-1];

                    if( rxCrc ==calcCrc )					
                    {
                        // pass, message is valid, process project specific commands
                        if( serialDataPtr->callBackPtr !=NULL )
                            serialDataPtr->callBackPtr( serialDataPtr->command);                               
                    }
                    else
                        serialDataPtr->errorCnt ++;                      
                                        
                    if(serialDataPtr->common.machState !=SERIAL_RX_PASS_THRU_STATE)
                    {       
                        // SERIAL_RX_PASS_THRU_STATE state is special case, 
                        // puts us in pass thru mode, so there 
                        // are no packets                                             
                        InitSerialData(serialDataPtr);
                    }
                    
                    return -1;
		} 
                
                serialDataPtr->common.timeoutTimer =TimerTicks;
	    }       	
            if( SerialCheckTimeout( &serialDataPtr->common, SERIAL_TIMEOUT_500MSEC) )
            {
                status =-1;
               	break;
            }	    	       			       			
            break;																		
        case SERIAL_RX_PASS_THRU_STATE:
            break;

    }		
	
    if( status == -1 )
        InitSerialData(serialDataPtr);
		
    return status;
    
} // end Serial_TerminalRxMachine()






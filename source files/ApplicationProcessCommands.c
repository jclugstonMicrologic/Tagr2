//###########################################################################
//
// FILE:   NFCOMMON_ProcessCommands.c
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

Int16U ReceivedCommand;

#define MAX_TX_BUF_SIZE	    1100

Int8U MsgReceived;
Int8U EvrHeartBeat;            
Int8U EvrFault;
Int8U GetConfigInfo;
Int8U GetConfigReceived;

Int8U TempBuf[1056];

Int8U SynchRequest =false;

EVENT_CONFIG_DATA EventConfigData;

RTC_DATA FileRtc;

char PayloadStr[MAX_TX_BUF_SIZE];

void Packet_ReadFlash(char *, char *payloadPtr,Int16U *nbrTxBytesPtr);
void Packet_EraseFlash(char *dataBufPtr, char *payloadPtr, Int16U *nbrTxBytesPtr);
void Packet_NandStatus(char *payloadPtr,Int16U *nbrTxBytesPtr);
void Packet_GetLogInfo(char *payloadPtr,Int16U *nbrTxBytesPtr);

void Packet_ConfigInfo(Int16U *payloadPtr,Int16U *nbrTxBytesPtr);
void Packet_Version(char *payloadPtr,Int16U *nbrTxBytesPtr);
void Packet_RequestFailure(Int16U cmdCode,char *payloadPtr,Int16U *nbrTxBytesPtr);
void Packet_BaudChange(Int16U *payloadPtr,Int16U *nbrTxBytesPtr);
void Packet_Ack(Int16U *payloadPtr,Int16U *nbrTxBytesPtr);

void Packet_ProcessKeyEvent(void);


void BuildPacket(Int16U *packetPtr,Int16U *dataPtr,Int16U nbrWords);
void BuildCharPacket(char *packetPtr, char *dataPtr,Int16U nbrWords);

void SendPacket( Int16U sciPort, char *packetPtr, Int16U nbrBytes);
void SendEraseEventLog(void);

void RouteCommand(Int16U cmd, char *dataBufPtr, Int16U nbrBytes);

void PopulateRtc( RTC_DATA *pFileRtc, char *dataBufPtr );

void SwapByteArray
(
    char *sourcePtr, 
    char *destPtr
)
{
    // swap every two bytes
    for( int j = 0; j < 512; j+=2)
    {
        *destPtr++ = sourcePtr[j+1];
        *destPtr++ = sourcePtr[j];
    }

}


/**********************************************************************
	Function: void ProcessCommandCom1(Int16U)
 	Description: Main Unit Port (UART6)
                     Some messages are routed to PC software
                     so the case break statement is replaced 
                     with a return
**********************************************************************/
void ProcessCommandCom1
(
   Int16U cmd
)
{
    Int16U nbrBytesToTx;	
    Int16U cmdCode;
   
    ReceivedCommand =cmd;

    // any valid message from the SAER can be considered the heartbeat message
    EvrHeartBeat =1;           
#if 0    
    switch( ReceivedCommand ) //terminalDataPtr->command )
    {
        case CMD_GET_VERSION:
	    Packet_Version( PayloadStr, &nbrBytesToTx );
	    break;			
        case CMD_RDP_STATUS_LED_ON:
            statusLed =SerialDataCom1.rxBuffer[6];
            
            AssertStatusLed(statusLed);
            
	    // this means just ACK the software
	    BuildCharPacket( PayloadStr, 0, 0 );
            nbrBytesToTx =0;
	    break;			            
        case CMD_RDP_STATUS_LED_OFF:
            statusLed =SerialDataCom1.rxBuffer[6];
            
            NegateStatusLed(statusLed);
            
	    // this means just ACK the software
	    BuildCharPacket( PayloadStr, 0, 0 );
            nbrBytesToTx =0;
	    break;		            
        case CMD_EVR_PACKET:
            if( !SynchRequest )
            {
                if( SerialDataCom1.nbrBytes ==140 )
                {
                  LogData.info.packetSize =132;
                  packetNbr = (SerialDataCom1.rxBuffer[112]<<24 | SerialDataCom1.rxBuffer[113]<<16 | SerialDataCom1.rxBuffer[110]<<8 | SerialDataCom1.rxBuffer[111]);
                }
                else if( SerialDataCom1.nbrBytes ==112 )
                {
                  LogData.info.packetSize =104;
                  packetNbr = (SerialDataCom1.rxBuffer[96]<<24 | SerialDataCom1.rxBuffer[97]<<16 | SerialDataCom1.rxBuffer[94]<<8 | SerialDataCom1.rxBuffer[95]);
                }
                
                if( packetNbr ==0 )
                    PrevPacketNbr =0;
       
                if( packetNbr >=PrevPacketNbr  )
                {
                    PrevPacketNbr =packetNbr;
              
                    LogData.info.logReady =true;        
                    LogQueEventData( &LogData, &SerialDataCom1.rxBuffer[6] );
                   
                    ChmmLogData.info.logReady =true;
                    LogQueEventData( &ChmmLogData, &SerialDataCom1.rxBuffer[6] );
                }
            }
            
	    // this means just ACK the software            
	    BuildCharPacket( PayloadStr, 0, 0 );
            nbrBytesToTx =0;          
//            return;
            break;
        case CMD_GET_CONFIG_INFO:
            GetConfigReceived =1;
            SwapByteArray( &SerialDataCom1.rxBuffer[6], (char *)&EventConfigData );             
            
            nbrBytesToTx = sizeof(EventConfigData);           
            return;
        case CMD_GET_RTC:
            MsgReceived =3;
            PopulateRtc( &FileRtc, &SerialDataCom1.rxBuffer[6]);
            return;
        case CMD_HEARTBEAT_PACKET:          
//            EvrHeartBeat =1;           
	    // this means just ACK the software            
	    BuildCharPacket( PayloadStr, 0, 0 );
            nbrBytesToTx =0;               
            
            if( SerialDataCom1.rxBuffer[7] ==0x01 ) // EVR faults active
            {
                EvrFault =1;
            }
            else
                EvrFault =0; 
            
            return;
            break;    
        case CMD_READ_FLASH_LOG:           
        case CMD_GET_RTD_DATA:
        case CMD_SET_LAST_PAGE_DUMPED:          
        case CMD_ROUTE_RTD_DATA:
        case CMD_GET_EVENT_DATA:
        case CMD_ROUTE_GET_CONFIG_INFO:
        case CMD_GET_GENERAL_STATS:
        case CMD_GET_THRESHOLDS:          
        case CMD_GET_FAULTS:
        case CMD_GET_SPARE_SIGNALS:
        case CMD_SET_NCORDER_SIM_DATA:
        case CMD_GET_NCORDER_SIM_DATA:
        case CMD_SET_CUSTOMER:
            MsgReceived =1;          
            
            if( !SynchRequest )
            {
                nbrBytesToTx = (SerialDataCom1.rxBuffer[2]<<8) | SerialDataCom1.rxBuffer[3] ; 
                              
                SendPacket(SCI_DIAG_PORT, SerialDataCom1.rxBuffer, nbrBytesToTx );
                
                if( ReceivedCommand ==CMD_ROUTE_GET_CONFIG_INFO )
                   SwapByteArray( &SerialDataCom1.rxBuffer[6], (char *)&EventConfigData );
            }
            else
            {
                LogData.info.logReady =true;        
                
                for(j=0; j<8; j++)
                {
                    LogQueEventData( &LogData, &SerialDataCom1.rxBuffer[6+j*132] );
                }
            }
            return;                        
	default:		
            // unknown command, return error condition
	    cmdCode =10;
            ReceivedCommand =CMD_REQUEST_FAILURE;		
    	    Packet_RequestFailure( cmdCode, PayloadStr, &nbrBytesToTx );
    	    break;
    }	

    
    if( nbrBytesToTx> MAX_TX_BUF_SIZE)
    {
        // we got a problem
        nbrBytesToTx =(MAX_TX_BUF_SIZE-SIZEOF_HEAD_TAIL);
    }
#endif
    
    /* pass into routine the total number of bytes to transmit */
    SendPacket(SCI_MAIN_PORT, PayloadStr, (nbrBytesToTx +SIZEOF_HEAD_TAIL) );
	
//    return status;
				
} // end ProcessCommandCom1()

/**********************************************************************
	Function: void ProcessCommandCom2(Int16U)
 	Description: Diagnostic Port (UART3)
                     Some messages are routed to Standalone recorder
                     so the case break statement is replaced 
                     with a return
**********************************************************************/
void ProcessCommandCom2
(
//   SERIAL_DATA *terminalDataPtr
   Int16U cmd
)
{
    Int16U nbrBytesToTx;	
    Int16U cmdCode;
         
    ReceivedCommand =cmd;     

#if 0    
    switch( ReceivedCommand ) //terminalDataPtr->command )
    {
        case CMD_GET_VERSION:
	    Packet_Version( PayloadStr, &nbrBytesToTx );
	    break;			
        case CMD_RDP_STATUS_LED_ON:           
            ALL_LEDS_ON;
            
	    // this means just ACK the software
	    BuildCharPacket( PayloadStr, 0, 0 );
            nbrBytesToTx =0;
	    break;			            
        case CMD_RDP_STATUS_LED_OFF:           
            ALL_LEDS_OFF;            
            
	    // this means just ACK the software
	    BuildCharPacket( PayloadStr, 0, 0 );
            nbrBytesToTx =0;
	    break;		                        
        case CMD_SYNCH_RDU:
            SynchRequest =true;
            break;
        case (Int16U)CMD_READ_FLASH_PAGE:
	    Packet_ReadFlash( SerialDataCom2.rxBuffer, PayloadStr, &nbrBytesToTx );
	    break;            
        case (Int16U)CMD_ERASE_FLASH_LOG:
            // send erase command to standalone event recorder (NLIMIT)
            SendEraseEventLog();
            
            // erase the RDU flash
	    Packet_EraseFlash( SerialDataCom2.rxBuffer, PayloadStr, &nbrBytesToTx );
	    break;   
        case CMD_GET_NAND_STATUS:
            Packet_NandStatus( PayloadStr, &nbrBytesToTx );          
            break;
        case CMD_UDPATE_BB_TABLE:
//            InitBBTableCreation =TRUE;
            CreateBadBlockTable();
            break;
        case CMD_GET_LOG_INFO:
        case CMD_GET_FLASH_INFO:
            Packet_GetLogInfo(PayloadStr, &nbrBytesToTx );
            break;            
        case CMD_PASS_THRU:         
            if( SerialDataCom1.common.machState ==3 )
            {
                // we are in pass thru mode, leave it now
                SerialStateProcess( &SerialDataCom1.common, 0);
                SerialStateProcess( &SerialDataCom2.common, 0);                       
                
                #ifdef BOARD_TEST
                OsStopTimer( &OsTimer[TIMER_PROGRESS_LED] );
                UPLOAD_STATUS_LED_OFF;
                #endif                
            }
            else
            {
                // else enter pass thru mode
                SerialStateProcess( &SerialDataCom1.common, 3);
                SerialStateProcess( &SerialDataCom2.common, 3);
                
                #ifdef BOARD_TEST
                OsStartPeriodicTimer( &OsTimer[TIMER_PROGRESS_LED], (Int32U)50, BlinkUploadProgressLed );
                #endif                                      
                
            }
            
            RouteCommand(ReceivedCommand, NULL, NULL);
            return;      
        case CMD_GET_CONFIG_INFO:
            ReceivedCommand =CMD_ROUTE_GET_CONFIG_INFO;          
            RouteCommand(ReceivedCommand, NULL, NULL);
            return;            
        case CMD_READ_FLASH_LOG:
        case CMD_GET_RTD_DATA:
        case CMD_ROUTE_RTD_DATA:    
        case CMD_GET_EVENT_DATA:    
        case CMD_SET_RTC:
        case CMD_SET_LAST_PAGE_DUMPED:
        case CMD_GET_GENERAL_STATS:
        case CMD_GET_THRESHOLDS:          
        case CMD_GET_SPARE_SIGNALS:
        case CMD_SET_NCORDER_SIM_DATA:
        case CMD_GET_NCORDER_SIM_DATA:
        case CMD_SET_CUSTOMER:
            RouteCommand(ReceivedCommand, &SerialDataCom2.rxBuffer[6], (SerialDataCom2.nbrBytes-SIZEOF_HEAD_TAIL) );
            return;
        case CMD_CHANGE_RECORD_INTERVAL:       
            GetConfigInfo =false;
            RouteCommand(ReceivedCommand, &SerialDataCom2.rxBuffer[6], (SerialDataCom2.nbrBytes-SIZEOF_HEAD_TAIL) );
            return;
        case CMD_SET_SPARE_SIGNALS:
            RouteCommand(ReceivedCommand, &SerialDataCom2.rxBuffer[6], (SerialDataCom2.nbrBytes-SIZEOF_HEAD_TAIL) );
            return;          
        case CMD_START_CHAMBER_TEST:            
        case CMD_STOP_CHAMBER_TEST:                      
        case CMD_GET_FAULTS: 
            RouteCommand(ReceivedCommand, NULL, NULL);
            return;                                          
        case CMD_RESET:                      
            NVIC_SystemReset();
            break;                              
	default:		
            // unknown command, return error condition
	    cmdCode =10;

            ReceivedCommand =CMD_REQUEST_FAILURE;		
    	    Packet_RequestFailure( cmdCode, PayloadStr, &nbrBytesToTx );
    	    break;
    }	
    
    if( nbrBytesToTx> MAX_TX_BUF_SIZE)
    {
        // we got a problem
        nbrBytesToTx =(MAX_TX_BUF_SIZE-SIZEOF_HEAD_TAIL);
    }
#endif
    
    /* pass into routine the total number of bytes to transmit */
    SendPacket(SCI_DIAG_PORT, PayloadStr, (nbrBytesToTx +SIZEOF_HEAD_TAIL) );
	
//    return status;
				
} // end ProcessCommandCom2()



/**********************************************************************
	Function: void Packet_ReadFlash(Uint16 *, Uint16 *)
 	Description: 

**********************************************************************/
void Packet_ReadFlash
(
    char *dataBufPtr,
    char *payloadPtr,
    Int16U *nbrTxBytesPtr
)
{
    Int32U pageNbr;

    // skip over the header
    dataBufPtr +=SIZEOF_HEAD;

    // skip over the bank (legacy)
    dataBufPtr +=4;

    // get the page number to read
    pageNbr = ((Int32U)dataBufPtr[0] <<24) |
	      ((Int32U)dataBufPtr[1] <<16) |
	              (dataBufPtr[2]  <<8)  |
	              (dataBufPtr[3]  );


    // this does not skip bad blocks
    ReadFlashAddress( (Int16U *)TempBuf, (pageNbr*NAND_BYTES_PER_PAGE), sizeof(TempBuf) );

    // 1 page of flash memory
    BuildCharPacket( payloadPtr, (char *)TempBuf, sizeof(TempBuf) ); //NAND_PAGE_SIZE1);
       
    *nbrTxBytesPtr =sizeof(TempBuf); 
}


void Packet_EraseFlash
(
    char *dataBufPtr,
    char *payloadPtr,
    Int16U *nbrTxBytesPtr
)
{
    char locoId[6];
    char fileName[15];
     
    LogData.info.logErase =1;
  
    // delete CHMM file as well
    memset( locoId, 0x00, sizeof(locoId) );
        
    locoId[0] =EventConfigData.locoId[0];
    locoId[1] =EventConfigData.locoId[1];
    locoId[2] =EventConfigData.locoId[2];
    locoId[3] =EventConfigData.locoId[3];            
    locoId[4] =EventConfigData.locoId[4];            
            
    strcpy( fileName, "0:CHM");
    strcat( fileName, locoId);
    strcat( fileName, ".bli");
        
    f_unlink(fileName);
    
    // this means just ACK the software
    BuildCharPacket( payloadPtr, 0, 0 );
    *nbrTxBytesPtr =0;
}



/**********************************************************************
	Function: void Packet_NandStatus(char *, Uint16 *)
 	Description: 

**********************************************************************/
void Packet_NandStatus
(
    char *payloadPtr,
    Int16U *nbrTxBytesPtr
)
{    
    Int16U eraseFlag =0;  
    
    if( LogData.info.logErase ==3 )
    {
        // PC software requires a two byte response
        eraseFlag =0x0303;
    }    
    
    BuildCharPacket( payloadPtr, (char *)&eraseFlag, 2 );

    *nbrTxBytesPtr =2;  
}


/**********************************************************************
	Function: void Packet_GetLogInfo(char *, Uint16 *)
 	Description: 

**********************************************************************/
void Packet_GetLogInfo
(
    char *payloadPtr,
    Int16U *nbrTxBytesPtr
)
{
    memset( TempBuf, 0x00, sizeof(TempBuf) );

    memcpy( TempBuf, &LogData.info, sizeof(LogData.info) );
  
    BuildCharPacket( payloadPtr, (char *)TempBuf, sizeof(LogData.info) );
    *nbrTxBytesPtr =sizeof(LogData.info);
}


/**********************************************************************
	Function: void Packet_Version(Uint16 *, Uint16 *)
 	Description: 

**********************************************************************/
void Packet_Version
(
    char *payloadPtr,
    Int16U *nbrTxBytesPtr	
)
{
    char tempData[25];

    strcpy( tempData, FW_VERSION);
    
    BuildCharPacket( payloadPtr, tempData, strlen( (char *)tempData ) );
	
    *nbrTxBytesPtr =strlen( (char *)tempData );
}


/**********************************************************************
	Function: void Packet_GetFlashInfo()
 	Description:

**********************************************************************/
void Packet_GetFlashInfo
(
    Int16U *payloadPtr,
    Int16U *nbrTxBytesPtr
)
{
}


/**********************************************************************
	Function: void Packet_RequestFailure(Uint16, Uint16 *, Uint16 *)
 	Description:

**********************************************************************/
void Packet_RequestFailure
(
    Int16U cmdCode,
    char *payloadPtr,
    Int16U *nbrTxBytesPtr
)
{
    BuildCharPacket( payloadPtr, (char *)cmdCode, sizeof(cmdCode) );
    *nbrTxBytesPtr =(sizeof(cmdCode)<<1);
}


/**********************************************************************
	Function: void Packet_Ack(Uint16 *, Uint16 *)
 	Description: 

**********************************************************************/
void Packet_Ack
(
    Int16U *payloadPtr,
    Int16U *nbrTxBytesPtr	
)
{
    Int16U ack =0xcc33;
    BuildPacket( payloadPtr, &ack, sizeof(ack) );
    *nbrTxBytesPtr =(sizeof(ack)<<1);
}


//###########################################################################
// void BuildPacket
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void BuildPacket
(
    Int16U *packetPtr,
    Int16U *dataPtr,
    Int16U nbrWords
)
{
    Int16U calculatedCRC, j;
    Int16U payloadSize;

    // get payload size in bytes
    payloadSize =(nbrWords<<1) +SIZEOF_HEAD_TAIL;
	
    // populate the synch bytes
    *packetPtr++ =SERIAL_HEAD1;
    *packetPtr++ =SERIAL_HEAD2;

    // populate number of bytes in packet
    *packetPtr++ =((payloadSize &0xff00)>>8);
    *packetPtr++ = (payloadSize&0x00ff);	

    // populate command response (echo received command)
    *packetPtr++ =((ReceivedCommand &0xff00)>>8);
    *packetPtr++ = (ReceivedCommand &0x00ff);		
	
    // build the payload
    for(j=0; j<(nbrWords<<1); j+=2)
    {
        // we need to break this up into individual bytes for CRC calculation
        *packetPtr++ =( (*dataPtr &0xff00) >>8);
        *packetPtr++ =( (*dataPtr &0x00ff));
    	
        // move to next data word
        *dataPtr ++;
    }

    // go back to beginning of packetPtr
    packetPtr -= ( (nbrWords<<1) +SIZEOF_HEAD );
	
    // calculate the crc on entire packet so far
    calculatedCRC = crc( (unsigned char *)packetPtr, ( (nbrWords<<1)+SIZEOF_HEAD), 0xffff); //0x0000);			

    // move packetPtr to end of payload
    packetPtr += ( (nbrWords<<1) +SIZEOF_HEAD);

    // append CRC
    *packetPtr++ =((calculatedCRC &0xff00)>>8);
    *packetPtr++ = (calculatedCRC &0x00ff);
				
}// end BuildPacket() 

			
//###########################################################################
// void BuildCharPacket
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void BuildCharPacket
(
    char *packetPtr,
    char *dataPtr,
    Int16U nbrBytes
)
{
    Int16U calculatedCRC, j;
    Int16U payloadSize;

    // get payload size in bytes
    payloadSize =nbrBytes +SIZEOF_HEAD_TAIL;
	
    // populate the synch bytes
    *packetPtr++ =SERIAL_HEAD1;
    *packetPtr++ =SERIAL_HEAD2;

    // populate number of bytes in packet
    *packetPtr++ =((payloadSize &0xff00)>>8);
    *packetPtr++ = (payloadSize&0x00ff);	
    
    // populate command response
    *packetPtr++ =((ReceivedCommand &0xff00)>>8);	
    *packetPtr++ = (ReceivedCommand &0x00ff);		
            
    // build the payload
    for(j=0; j<nbrBytes; j++)
    {
        // we need to break this up into individual bytes for CRC calculation
        *packetPtr++ = *dataPtr ++;	
    }
    
    // go back to beginning of packetPtr
    packetPtr -= ( nbrBytes +SIZEOF_HEAD );
            
    // calculate the crc on entire packet so far
    calculatedCRC = crc( (unsigned char *)packetPtr, (nbrBytes+SIZEOF_HEAD), 0xffff);			
    
    // move packetPtr to end of payload
    packetPtr += ( nbrBytes +SIZEOF_HEAD);
    
    // append CRC
    *packetPtr++ =((calculatedCRC &0xff00)>>8);
    *packetPtr++ = (calculatedCRC &0x00ff);

}// end BuildCharPacket() 



//###########################################################################
// void SendPacket
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SendPacket
(
    Int16U sciPort,
    char *packetPtr,
    Int16U nbrBytes
)
{
    Int16U j;

    // send the packet
    for( j=0; j<nbrBytes; j++)
    {
        SciSendByte(sciPort, (*packetPtr++) );
    }          

} // end SendPacket()


//###########################################################################
// void SendGetConfigInfo
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SendGetConfigInfo(void)
{
    Int16U payloadStr[20];

    // flag to know we have the config info 
    // write to flash drive cannot proceed without this vital information    
    MsgReceived =0;
    GetConfigReceived =0;

    // request info from standalone recorder now    
    ReceivedCommand =CMD_GET_CONFIG_INFO;
    BuildCharPacket( (char *)payloadStr, 0, 0);
    SendPacket(SCI_MAIN_PORT, ( char *)payloadStr, SIZEOF_HEAD_TAIL );
}


//###########################################################################
// void SendGetRtc
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SendGetRtc(void)
{
    Int16U payloadStr[50];

    // flag to know we have the config info 
    // write to flash drive cannot proceed without this vital information    
    MsgReceived =0;

    // request info from standalone recorder now    
    ReceivedCommand =CMD_GET_RTC;
    BuildCharPacket( (char *)payloadStr, 0, 0);
    SendPacket(SCI_MAIN_PORT, ( char *)payloadStr, SIZEOF_HEAD_TAIL );
}


//###########################################################################
// void SendEraseEventLog
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SendEraseEventLog(void)
{
    Int16U payloadStr[20];
   
    ReceivedCommand =CMD_ERASE_FLASH_LOG;
    BuildCharPacket( (char *)payloadStr, 0, 0);
    SendPacket(SCI_MAIN_PORT, ( char *)payloadStr, SIZEOF_HEAD_TAIL );
}

//###########################################################################
// void SendGetConfigInfo
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void GetFlashLog(Int32U page, char *pDataBuf)
{
    Int16U payloadStr[20];

    MsgReceived =0;
    
    // request info from standalone recorder now        
    ReceivedCommand =CMD_READ_FLASH_LOG;
    BuildCharPacket( (char *)payloadStr, pDataBuf, 8);
    SendPacket(SCI_MAIN_PORT, ( char *)payloadStr, (SIZEOF_HEAD_TAIL+8) );
}

//###########################################################################
// void RouteCommand
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void RouteCommand
(
    Int16U cmd,
    char *dataBufPtr,    
    Int16U nbrBytes    
)
{
    Int16U payloadStr[50];

    if( GetHeartBeatState() ==2 ||
        GetConfigReceived ==0
      )
    {
        // do not send messages to SAER if no comms has been established
        return;
    }
      
    // flag to know we have the info 
    MsgReceived =0;  

    // request info from standalone recorder now        
    ReceivedCommand =cmd;
    BuildCharPacket( (char *)payloadStr, dataBufPtr, nbrBytes);
    SendPacket(SCI_MAIN_PORT, ( char *)payloadStr, (SIZEOF_HEAD_TAIL+nbrBytes) );
}


//###########################################################################
// void PopulateRtc
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void PopulateRtc
(
    RTC_DATA *pFileRtc,
    char *dataBufPtr
)
{
    *dataBufPtr ++;
    
    pFileRtc->year = (*dataBufPtr);
    dataBufPtr+=2;
    
    pFileRtc->year +=( *dataBufPtr *10);
    dataBufPtr+=2;
    
    pFileRtc->month =(*dataBufPtr);
    dataBufPtr+=2;
    
    pFileRtc->month +=( *dataBufPtr *10);
    dataBufPtr+=2;
    
    pFileRtc->day =(*dataBufPtr);
    dataBufPtr+=2;
    
    pFileRtc->day +=( *dataBufPtr *10);
    dataBufPtr+=2;
    
    pFileRtc->hour =(*dataBufPtr);
    dataBufPtr+=2;
    
    pFileRtc->hour +=( *dataBufPtr *10);
    dataBufPtr+=2;
    
    pFileRtc->min =(*dataBufPtr);
    dataBufPtr+=2;
    
    pFileRtc->min +=( *dataBufPtr *10);
    dataBufPtr+=2;

    pFileRtc->sec =(*dataBufPtr);
    dataBufPtr+=2;
    
    pFileRtc->sec +=( *dataBufPtr *10);
    dataBufPtr+=2;
    
}


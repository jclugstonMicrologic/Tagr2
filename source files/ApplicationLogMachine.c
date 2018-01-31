//###########################################################################
//
// FILE:   ApplicationLogMachine.c
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 28 OCT 2012 | JDC  | Original version 
//###########################################################################


#include "RDU.h"


LOG_DATA LogData;

LOG_DATA ChmmLogData;

Int8U InitBBTableCreation;

Int8U TempDataBuf[150];
 
void LogStatusLed2On(void)
{
    STATUS_LED1_ON;
}

void LogStatusLed2Off(void)
{
    STATUS_LED1_OFF;  
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: InitLog
*|  Description: Initialize logging parameters
*|----------------------------------------------------------------------------
*/
void InitLog
(
    LOG_DATA *pLogData,
    Int8U logType
)
{
    InitBBTableCreation =FALSE;
  
    pLogData->info.logErase  =3;

    pLogData->in =0;
    pLogData->full =0;
    pLogData->out =0;
            
    //memset(pLogData, 0x0000, sizeof(LOG_DATA) );
    switch( logType )
    {
        case 0:               
            memset(pLogData->buffer, 0x00, sizeof(pLogData->buffer) );

            pLogData->info.address  =RDU_LOG_ADDR_START; // first log block
            pLogData->info.wrapAddr =0;

            pLogData->info.logStart =RDU_LOG_ADDR_START;
            pLogData->info.logEnd   =RDU_LOG_ADDR_END;
              
            pLogData->info.logReady =0;
            pLogData->info.packetSize =RDU_LOG_PACKET_SIZE;
            pLogData->info.machState =LOG_IDLE_STATE;
            
            
            pLogData->callBackPtr1 =NULL;//LogStatusLed2On;
            pLogData->callBackPtr2 =NULL;//LogStatusLed2Off;
            break;
        case 1:               
            memset(pLogData->buffer, 0x00, sizeof(pLogData->buffer) );

            pLogData->info.address  =0; 
            pLogData->info.wrapAddr =0;

            pLogData->info.logStart =0;
            pLogData->info.logEnd   =0;
              
            pLogData->info.logReady =0;
            pLogData->info.packetSize =RDU_LOG_PACKET_SIZE;
            pLogData->info.machState =LOG_IDLE_STATE;
            break;            
    }
    
}  // end InitLog()


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: LogStateProcess
*|  Description: move the calling machines state, and reset timeoutTimer
*|----------------------------------------------------------------------------
*/
void LogStateProcess
(
    LOG_DATA *pLogData,
    LOG_STATES state
)
{
    if( state == LOG_NEXT_STATE ) // go to next state
        pLogData->info.machState ++;
    else             // go to specific state
        pLogData->info.machState =state;

    pLogData->timeoutTimer =TimerTicks;

} // end LogStateProcess()


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: LogManageAddresses
*|  Description: manage log address (wrap, end of memory, etc)
*|----------------------------------------------------------------------------
*/
void LogManageAddresses
(
    LOG_DATA *pLogData
)
{
    Int32U eraseBlock =0;
    
    if( pLogData->info.address ==pLogData->info.wrapAddr )
    {
        pLogData->info.wrapAddr +=NAND_BYTES_PER_BLOCK;
        
        eraseBlock =1;
    }
    if( pLogData->info.address >=pLogData->info.logEnd )
    {
        pLogData->info.address  =pLogData->info.logStart;
        pLogData->info.wrapAddr =pLogData->info.logStart+NAND_BYTES_PER_BLOCK;
        
        eraseBlock =1;
    }  
    
    if( eraseBlock )
    {     
        while( !EraseFlashBlockWithCheck(pLogData->info.address/NAND_BYTES_PER_BLOCK) )
        {
            // erase failed, this has now been flagged as a bad block
            // skip to next block
            pLogData->info.address +=NAND_BYTES_PER_BLOCK;            
            pLogData->info.wrapAddr +=NAND_BYTES_PER_BLOCK;
        }          
//        EraseFlashBlock(logDataPtr->info.address/NAND_BYTES_PER_BLOCK);
    }
    
} // end LogManageAddresses()


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: LogIncrementAddress
*|  Description: 
*|----------------------------------------------------------------------------
*/
void LogIncrementAddress
(
    LOG_DATA *pLogData
)
{
    pLogData->info.address +=pLogData->info.packetSize;

    if( pLogData->info.packetSize ==104 )
    {
      if( (pLogData->info.address %2112) ==2080 )
      {
          // keep on page boundaries
          pLogData->info.address =(pLogData->info.address/2112 +1)*2112;
      }
    }
    else if( (pLogData->info.address %pLogData->info.packetSize) !=0 )
    {
      // keep on page boundaries
      pLogData->info.address =(pLogData->info.address/2112 +1)*2112;  
    }
      
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: LogFindEndOfMemory
*|  Description: Find log memory end for a wrapping memory type                        
*|               nbrSeqBytes->number of sequential 0xff bytes required
*|               to indicate end of log
*|----------------------------------------------------------------------------
*/
void LogFindEndOfMemory
(
    LOG_DATA *pLogData,
    Int16U nbrSeqBytes // number of sequential 0xff bytes
)
{        
    Int8U eraseValue[64];
    Int64U nextBlockAddr;
    
    memset(eraseValue, 0xff, sizeof(eraseValue) );
    
    pLogData->info.address =RDU_LOG_ADDR_START;
    
    // start the search
    while(1)
    {
        // check if current page is within a bad block, skip to next block if it is
        HandleBadBlock( &pLogData->info.address );
                
        ReadFlashAddress( (Int16U *)pLogData->buffer, pLogData->info.address, pLogData->info.packetSize );      
    
        if( memcmp(&pLogData->buffer, &eraseValue, nbrSeqBytes) !=0 )    
        {
            // not erased, check the end of the block
            pLogData->info.address +=(NAND_BYTES_PER_BLOCK - pLogData->info.packetSize);
            
            ReadFlashAddress( (Int16U *)pLogData->buffer, pLogData->info.address, pLogData->info.packetSize );                      
                
            if( memcmp(&pLogData->buffer, &eraseValue, nbrSeqBytes) !=0 )    
            {
                // end of block is not erased, continue on to start of next block
                pLogData->info.address +=pLogData->info.packetSize;
                continue;
            }
            else
            {
                // end of block is erased, so log memory end is within this block 
                break;
            }
        }
        else
        {
            // start of block is erased, this is end of memory           
            return;
        }
    }

    while(1)
    {
        ReadFlashAddress( (Int16U *)pLogData->buffer, pLogData->info.address, pLogData->info.packetSize );      
    
        if( memcmp(&pLogData->buffer, &eraseValue, nbrSeqBytes) ==0 )    
        {
            // erased, continue reading backwards through block
            pLogData->info.address -=pLogData->info.packetSize;
        }
        else
        {
            // not erased, move forward one packet size, we know that location is erased, and it is the 
            // end of log memory
            pLogData->info.address +=pLogData->info.packetSize;          
            
            // check the next block, if it is erased, we have not wrapped yet, 
            // if data is present, then memory wrap has occured            
            nextBlockAddr =(pLogData->info.address/NAND_BYTES_PER_BLOCK +1) *NAND_BYTES_PER_BLOCK;
            
            ReadFlashAddress( (Int16U *)pLogData->buffer, nextBlockAddr, pLogData->info.packetSize );      
            
            if( memcmp(&pLogData->buffer, &eraseValue, nbrSeqBytes) ==0 )    
            {              
                // erased
                pLogData->info.wrapAddr =RDU_LOG_ADDR_START;
            }
            else
            {
                // not erased, we have wrapped
                pLogData->info.wrapAddr =nextBlockAddr;              
            }
            break;
        }
    }
       
} // end LogFindEndOfMemory()


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: LogMachine(LOG_DATA *)
*|  Description: Handle the states of the logging machine
*|----------------------------------------------------------------------------
*/
void LogMachine
(
    LOG_DATA *pLogData
)
{
    Int16U calcCrc, packetCrc;
    Int16U startBlock, endBlock, j;
    Int64U address;

    Int32U packetNbr =0;        
    static Int16U PrevPacketNbr =0;       

    if( InitBBTableCreation )
    {
        CreateBadBlockTable();		

	// reinit all logs now
	InitLog(&LogData, 0);
		
	InitBBTableCreation =FALSE;
    }
    
    switch( pLogData->info.machState )
    {
        case LOG_IDLE_STATE:
            // do initilization stuff here
            if( pLogData->info.packetSize > sizeof(pLogData->buffer) ||
                pLogData ==NULL
              )
            {
                // this is no good, programmer must fix this
                LogStateProcess(pLogData, LOG_LAST_STATE);  
                return;
            }
  
            LogFindEndOfMemory( pLogData, 64 );            

            memset( TempDataBuf, 0x00, sizeof(TempDataBuf) );
                   
            // goto next state
            LogStateProcess(pLogData, LOG_NEXT_STATE);
            break;
        case LOG_RUN_STATE:
            if( pLogData->info.logReady )
            {
                LogManageAddresses( pLogData );
                  
                if(pLogData->callBackPtr1 !=NULL )
                    pLogData->callBackPtr1();
    
                // check if current page is within a bad block
                HandleBadBlock( &pLogData->info.address );
            
                //while( LogDequeEventData( pLogData, TempDataBuf ) )  
                if( LogDequeEventData( pLogData, TempDataBuf ) )  
                {
                    // log packets from the event recorder already contain a CRC
                    // on the data, check it now to confirm data packet is OK
                    calcCrc = crc((unsigned char *)TempDataBuf, (pLogData->info.packetSize-2), 0x0000);
                    packetCrc = (TempDataBuf[pLogData->info.packetSize-2]<<8 | TempDataBuf[pLogData->info.packetSize-1]);

                    if( packetCrc != calcCrc )
                    {
                        // flag this packet as known garbage        
//                        memset( TempDataBuf, 0xaa, sizeof(TempDataBuf) );
                        LogStateProcess(pLogData, LOG_RUN_STATE);                                                  
                        pLogData->info.logReady =0;
                        
                        return;
                    }
                    
#if 0                    
                    packetNbr = (TempDataBuf[106]<<24 | TempDataBuf[107]<<16 | TempDataBuf[104]<<8 | TempDataBuf[105]);
                    
                    if( packetNbr ==0 )
                        PrevPacketNbr =0;
   
                    if( packetNbr <PrevPacketNbr  )
                    {
                        pLogData->info.logReady =0;              
                        return;
                    }
                    PrevPacketNbr =packetNbr;
#endif
                    
                    // we have data to log, log it now
                    if( WriteFlashAddress( (Int16U *)TempDataBuf, pLogData->info.address, pLogData->info.packetSize ) )                
                    {
                        // goto next state                                    
                        LogStateProcess(pLogData, LOG_CONFIRM_STATE);
//                        pLogData->info.logReady =0;                    
                    }
                    else
                    {
                        // fail, page is not erased, try next section
                        pLogData->info.address +=pLogData->info.packetSize;                                 
                        LogStateProcess(pLogData, LOG_WRITE_FAIL_STATE );
                        break;
                    }                  

                } // end dequeing                
                else
                    pLogData->info.logReady =0;                    
            }      
            else if( pLogData->info.logErase ==1 )
            {
                LogStateProcess(pLogData, LOG_ERASE_STATE);                  
            }
            break;
        case LOG_CONFIRM_STATE: // read data back and confirm integrity

            memset(TempDataBuf, 0x00, pLogData->info.packetSize);
          
            if( !CheckFlashStatus() )
            {
                // flash busy
                return;
            }

            // read back our data and check CRC
            ReadFlashAddress( (Int16U *)TempDataBuf, pLogData->info.address, pLogData->info.packetSize );                  
            calcCrc = crc((unsigned char *)TempDataBuf, (pLogData->info.packetSize-2), 0x0000);
            packetCrc = (TempDataBuf[pLogData->info.packetSize-2]<<8 | TempDataBuf[pLogData->info.packetSize-1]);
            
            if( packetCrc == calcCrc )
            {
                // write passed
                LogIncrementAddress(pLogData);

                LogStateProcess(pLogData, LOG_RUN_STATE);                          
            }
            else  
            {
                // write failure              
                SPARE_STATUS_LED_ON;
                
            //    pLogData->info.address +=pLogData->info.packetSize;
                LogIncrementAddress(pLogData);
                
                LogStateProcess(pLogData, LOG_RUN_STATE);                                          
            }             
            
            if(pLogData->callBackPtr2 !=NULL )
                pLogData->callBackPtr2(); 
            break;
        case LOG_WRITE_FAIL_STATE:            
            // we have data to log, log it now
            if( WriteFlashAddress( (Int16U *)TempDataBuf, pLogData->info.address, pLogData->info.packetSize ) )                
            {
                LogStateProcess(pLogData, LOG_CONFIRM_STATE);
                pLogData->info.logReady =0;                    
            }
            else
            {
                // fail, page is not erased, try next section
                //pLogData->info.address +=pLogData->info.packetSize;     
                
                LogIncrementAddress(pLogData);
                
                LogStateProcess(pLogData, LOG_WRITE_FAIL_STATE );
                break;
            }                  
                    
            LogStateProcess(pLogData, LOG_CONFIRM_STATE);
            pLogData->info.logReady =false;                    
            break;
        case LOG_ERASE_STATE:         
            startBlock =RDU_LOG_ADDR_START/NAND_BYTES_PER_BLOCK;
            endBlock   =RDU_LOG_ADDR_END/NAND_BYTES_PER_BLOCK;    
  
            for(j=startBlock; j<endBlock; j++)
            {
                // check if current block is within a bad block
                address =j*135168;
                HandleBadBlock( &address );    
                
                EraseFlashBlock( address/135168 );
            }
    
            // flag letting software know erase is complete
            pLogData->info.logErase =3;
    
            InitLog(&LogData, LOG_IDLE_STATE);
      
            // erase complete
            LogStateProcess(pLogData, LOG_IDLE_STATE); 
            break;           
    }
    
} // end LogMachine()



/**********************************************************************
	Function: Bool LogQueEventData()
 	Description: 

**********************************************************************/
Int16U LogQueEventData
(
    LOG_DATA *pLogData,
    char *sourcePtr
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   Int16U qIn, qFull, qOut;

   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   // read q variables into locals
   qIn = pLogData->in;
   qFull = pLogData->full;
   qOut = pLogData->out;
   
   if( (qIn!=qOut)|| !qFull )
   {
      memcpy( &pLogData->buffer[qIn], sourcePtr, 132); 
      
      qIn +=132; // update pointer
      
      if( qIn>(2112-1) ) qIn = 0; // check for wraparound

      pLogData->full =0x00;
      pLogData->in =qIn;

      return TRUE; // return true for successful read
   }
   else
   {
      return FALSE; // return false as no data available
   }

} // end LogQueEventData()


/**********************************************************************
	Function: Bool LogDequeEventData()
 	Description: 

**********************************************************************/
Int16U LogDequeEventData
(
    LOG_DATA *pLogData,
    Int8U *tempPtr
)
{
   /*
   ***********************
    L O C A L   D A T A
   ***********************
   */
   Int16U rxIn, rxFull, rxOut;


   /*
   *************************
    E R R O R   C H E C K S
   *************************
   */

   /* Read rx buffer variables into locals */
   rxIn = pLogData->in;
   rxFull = pLogData->full;
   rxOut = pLogData->out;
   
   if( (rxIn!=rxOut)||rxFull )
   {
      memcpy( tempPtr, &pLogData->buffer[rxOut], 132);
      
      rxOut +=132; /* update pointer..*/
      
      if( rxOut>(2112-1) ) rxOut = 0;/* check for wraparound */

      pLogData->full =0x00;
      pLogData->out =rxOut;

      return 1; /* return 1 for successful read */
   }
   else
   {
      return 0; /* return 0 as no byte available */
   }

}// end LogDequeEventData()


#ifdef jeff
/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: LogGetStartEnd(LOG_DATA *)
*|  Description: Check if current address resides within a known bad block,
*|               if it does, move the address to the next block
*|----------------------------------------------------------------------------
*/
void LogGetStartEnd
(
    LOG_DATA *pLogData,
)
{
    if( pLogData->info.wrapAddr <pLogData->info.address )            
    {
        ReadLocation =pLogData->info.logStart;
        ReadEnd      =pLogData->info.address;
    }
    else
    {
        StReadLocation =pLogData->info.wrapAddr; 
        ReadEnd      =pLogData->info.logEnd;
                
        // need to get the front end as well (most recent data is there)
        GetFrontEnd =1;
    }
}
#endif







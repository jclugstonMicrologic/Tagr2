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
#include "AppPtcMessaging.h"
#include "udpserverClient.h"

typedef struct
{
    Int8U protocolHdr;
    Int16U id;
    Int8U version;
    Int8U flags;
    Int32U dataLength;
    Int32U msgNbr;
    Int32U timeStamp;
    Int8U varHrdSize;
        
}MSG_HEADER;

typedef struct
{
    Int16U tag;
    Int32U size;
    Int8U dataVersionValue;
    Int8U sourceComponentValue;
    Int8U dataDictionaryVersion;
    Int8U msgGroupValue;
    Int8U ligClassCMsgNbr;
    Int8U nbrDataTagsValue;
    Int8U validity;
    Int8U epoch[6];
        
}LIG_MSG_HEADER;

typedef struct
{
    Int16U tag;
    Int32U size;
    Int8U value;
}TAG;

typedef struct
{
    MSG_HEADER mgsHeader; 
    
    LIG_MSG_HEADER ligMsgHeader;
    
    TAG tag; // message body
    
}PTC_MESSAGE;

PTC_MESSAGE PtcMessage;


void BuildPtcBody(Int16U tag, Int16U value);

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


void AppendToBuffer(char *pDataBuf, Int32U value, Int16U *pIndex, Int8U size)
{   
    switch(size)
    {
        case 1:
            *pDataBuf ++ =value;
            break;
        case 2:
            *pDataBuf ++ =(value &0xff00)>>8;
            *pDataBuf ++ =(value &0x00ff);
            break;            
        case 3:
            *pDataBuf ++ =(value &0xff0000)>>16;
            *pDataBuf ++ =(value &0x00ff00)>>8;
            *pDataBuf ++ =(value &0x0000ff);            
            break;                        
        case 4:
            *pDataBuf ++ =(value &0xff000000)>>24;            
            *pDataBuf ++ =(value &0x00ff0000)>>16;
            *pDataBuf ++ =(value &0x0000ff00)>>8;
            *pDataBuf ++ =(value &0x000000ff);            
            break;                        
    }
    
    *pIndex +=size;  
}




/**********************************************************************
	Function: void InitPtcMessage()
 	Description: 
**********************************************************************/
void InitPtcMessage(void)
{
    memset( &PtcMessage, 0x00, sizeof(PtcMessage) );
}


/**********************************************************************
	Function: void SendPtcMesssge(  )
 	Description: 
**********************************************************************/
void SendPtcMesssge(Int16U tag, Int16U value)
{
    char dataBuffer[100];
    int length =0;
         
    BuildPtcBody(tag, value);
    
    length =BuildPtcMessage( dataBuffer );
    
    udpclient_send( dataBuffer, length );
}


/**********************************************************************
	Function: void BuildPtcPacket( char * )
 	Description: 
**********************************************************************/
void BuildPtcPacket(char *pDataBuf)
{
    Int16U index =0;
    
    AppendToBuffer( &pDataBuf[index], PtcMessage.mgsHeader.protocolHdr, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.mgsHeader.id, &index, 2);
    AppendToBuffer( &pDataBuf[index], PtcMessage.mgsHeader.version, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.mgsHeader.flags, &index, 1);        
    AppendToBuffer( &pDataBuf[index], PtcMessage.mgsHeader.dataLength, &index, 3);          
    AppendToBuffer( &pDataBuf[index], PtcMessage.mgsHeader.msgNbr, &index, 4);      
    AppendToBuffer( &pDataBuf[index], PtcMessage.mgsHeader.timeStamp, &index, 4);      
    AppendToBuffer( &pDataBuf[index], PtcMessage.mgsHeader.varHrdSize, &index, 1);      
    
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.tag, &index, 2);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.size, &index, 4);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.dataVersionValue, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.sourceComponentValue, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.dataDictionaryVersion, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.msgGroupValue, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.ligClassCMsgNbr, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.nbrDataTagsValue, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.validity, &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.epoch[0], &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.epoch[1], &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.epoch[2], &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.epoch[3], &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.epoch[4], &index, 1);
    AppendToBuffer( &pDataBuf[index], PtcMessage.ligMsgHeader.epoch[5], &index, 1);
    
    AppendToBuffer( &pDataBuf[index], PtcMessage.tag.tag, &index, 2);
    AppendToBuffer( &pDataBuf[index], PtcMessage.tag.size, &index, 4);
    AppendToBuffer( &pDataBuf[index], PtcMessage.tag.value, &index, 1);    
}


/**********************************************************************
	Function: Int16U BuidlPtcMessage( char *, Int16U )
 	Description: 
**********************************************************************/
Int16U BuildPtcMessage(char *pDataBuf)
{   
    Int16U length =0;
  
    PtcMessage.mgsHeader.protocolHdr =0x02;
    PtcMessage.mgsHeader.id =0x0ffff;
    PtcMessage.mgsHeader.version =0x02;
    PtcMessage.mgsHeader.flags =0x09;
    PtcMessage.mgsHeader.dataLength =0x000335;
    PtcMessage.mgsHeader.msgNbr ++;
    PtcMessage.mgsHeader.timeStamp =0x00000000;
    PtcMessage.mgsHeader.varHrdSize =0x00;
      
    PtcMessage.ligMsgHeader.tag =0xffff;
    PtcMessage.ligMsgHeader.size =0x0000000d;
    PtcMessage.ligMsgHeader.dataVersionValue =0x02;
    PtcMessage.ligMsgHeader.sourceComponentValue =0x4d;
    PtcMessage.ligMsgHeader.dataDictionaryVersion =0x01;
    PtcMessage.ligMsgHeader.msgGroupValue =0x03;
    PtcMessage.ligMsgHeader.ligClassCMsgNbr =0x03;
    PtcMessage.ligMsgHeader.nbrDataTagsValue =0x2f;
    PtcMessage.ligMsgHeader.validity  =0x00;
    memset(PtcMessage.ligMsgHeader.epoch, 0x00, sizeof(PtcMessage.ligMsgHeader.epoch) );
           
    BuildPtcPacket(pDataBuf);
      
    length =43;//sizeof(PtcMessage);

    return length;
}


void BuildPtcBody(Int16U tag, Int16U value)
{
    PtcMessage.tag.tag =tag;
    PtcMessage.tag.size =0x00000001;
    PtcMessage.tag.value =value;
}




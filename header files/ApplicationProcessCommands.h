
//###########################################################################
//
// FILE:   NFCOMMON_ProcessCommands.h
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 30 AUG 2012 | JDC  | Original version 
//###########################################################################

#ifndef PROCESS_COMMANDS_H
#define PROCESS_COMMANDS_H

#include "ApplicationSciRxMachine.h"

// Nalysis/NForce serial commands
#define CMD_GET_RTD_DESCRIPTIONS1       (0x5000)
#define CMD_GET_RTD_DESCRIPTIONS2       (0x5001)
#define CMD_GET_RTD_DESCRIPTIONS3       (0x5002)
#define CMD_GET_RTD_DESCRIPTIONS4       (0x5003)
#define CMD_GET_RTD_DATA		(0x5004)
#define CMD_GET_RTD_FLAGS		(0x5005)
#define CMD_GET_GENERAL_STATS		(0x5006)
//#define CMD_GET_FAULTS			(0x5007)
#define CMD_GET_CANBUS_ACCESS		(0x5008)
#define CMD_GET_IDRV			(0x5009)
#define CMD_GET_FLASH_INFO		(0x500A)
#define CMD_GET_CUSTOM_DATA             (0x500B)
#define CMD_GET_CUSTOM_DESCRIPTIONS     (0x500C)
#define CMD_GET_CUSTOM_CODES		(0x500D)
#define CMD_GET_CUSTOM_STRINGS		(0x500E)
#define CMD_GET_NALYSIS_REQUEST		(0x500F)
#define CMD_GET_CAN_MAILBOX		(0x5010)

#define CMD_SET_RTC			(0x6000)
#define CMD_SET_WHEEL_DIA		(0x6001)
#define CMD_SET_LOCO_ID			(0x6002)
#define CMD_SET_NALYSIS_REQUEST		(0x6003)


#define CMD_GET_RADIO_SETTINGS	        (0x5555)
#define CMD_SET_RADIO_SETTINGS  	(0x5556)
#define CMD_GET_GPS_DATA       		(0x5557)
#define CMD_GET_VERSION 		(0x5558)
#define CMD_GET_RTC 			(0x5559)
#define CMD_GET_RTC_NVRAM		(0x555A)
#define CMD_GET_NCONNECT_STATUS         (0x555B)


#define CMD_REQUEST_FAILURE		(0xB5D5)
#define CMD_GET_CONFIG_INFO		(0xBED0)
#define CMD_READ_FLASH_PAGE 		(0xBED1)
#define CMD_READ_FLASH_LOG		(0xBED2)

#define CMD_CHANGE_BAUD			(0xBED3)
#define CMD_CHANGE_RECORD_INTERVAL	(0xBED4)
#define CMD_ERASE_FLASH_LOG   		(0xBED5)

#define CMD_ERASE_EPL_LOG   		(0xBED6)
#define CMD_ERASE_FAULT_LOG		(0xBED7)

#define CMD_START_EVENT_LOG   		(0xBED8)
#define CMD_GET_NAND_STATUS		(0xBED9)
#define CMD_SET_LAST_PAGE_DUMPED   	(0xBEDA)
#define CMD_GET_LAST_PAGE_DUMPED   	(0xBEDB)
#define CMD_SET_THRESHOLDS		(0xBEDC)
#define CMD_UDPATE_BB_TABLE		(0xBEDD)
#define CMD_GET_LOG_INFO                (0xBEDF)
#define CMD_GET_THRESHOLDS              (0xBEE0)
#define CMD_WRITE_FLASH                 (0xBEE1)
#define CMD_SET_SPARE_SIGNALS           (0xBEE2)
#define CMD_GET_SPARE_SIGNALS           (0xBEE3)
#define CMD_SET_CUSTOMER                (0xBEEA)


// EVR/RDP serial commands
#define CMD_RDP_STATUS_LED_ON           (0x7000)
#define CMD_RDP_STATUS_LED_OFF          (0x7001)
#define CMD_EVR_PACKET                  (0x7002)  
#define CMD_HEARTBEAT_PACKET		(0x7003)
#define CMD_ROUTE_RTD_DATA      	(0x7004)
#define CMD_ROUTE_GET_CONFIG_INFO       (0x7005)
#define CMD_PASS_THRU                   (0x7006)
#define CMD_START_CHAMBER_TEST          (0x7007)
#define CMD_STOP_CHAMBER_TEST           (0x7008)
#define CMD_GET_FAULTS                  (0x7009)
#define CMD_GET_EVENT_DATA              (0x700A)
#define CMD_SET_NCORDER_SIM_DATA        (0x700B)
#define CMD_GET_NCORDER_SIM_DATA        (0x700C)
#define CMD_SYNCH_RDU                   (0x700D)

#define CMD_RESET                       (0xFFFE)

#define PAGE1_MASK  (0x0001)
#define PAGE2_MASK  (0x0002)
#define PAGE3_MASK  (0x0004)
#define PAGE4_MASK  (0x0008)
#define PAGE5_MASK  (0x0010)
#define PAGE6_MASK  (0x0020)
#define PAGE7_MASK  (0x0040)
#define PAGE8_MASK  (0x0080)
#define PAGE9_MASK  (0x0100)
#define PAGE10_MASK (0x0200)
#define PAGE11_MASK (0x0400)
#define PAGE12_MASK (0x0800)
#define PAGE13_MASK (0x1000)
#define PAGE14_MASK (0x2000)
#define PAGE15_MASK (0x4000)
#define PAGE16_MASK (0x8000)


#define MAX_NALYSIS_KEYS  8
#define MAX_NALYSIS_PAGES 8
#define MAX_FEEDBACK      8

typedef struct
{
    Int16U valid;
    Int16U value;
}FEEDBACK;

typedef struct
{
    Int16U keyPress;
    Int16S page; 	
    Int16U spare1;
    Int16U spare2;
 	 	
    // feedback for Nalysis
    FEEDBACK feedback[MAX_FEEDBACK];
	
}NALYSIS_REQUEST;


typedef struct
{
   void (*callBackPtr)();
}KEY_PAGE;

typedef struct
{
    KEY_PAGE page[MAX_NALYSIS_PAGES]; 
}NAYLSIS_KEY;

typedef struct
{
   Int16U customerId[16];
   Int16U locoId[6];
   Int16U spare1[25];
   Int16U wheelDia;
   Int16U ppr;
   
   Int16U digitals[5];
   Int16U analogs[2];
   
   Int32U start[6];
   Int32U address[6];
   Int32U wrapAddr[6];
   Int32U endAddr[6];
   
   Int16U spare2[158]; //[213];
   
   Int16U crc;        
}EVENT_CONFIG_DATA;

typedef struct
{
    Int8U year;
    Int8U month;
    Int8U day;
    Int8U hour;
    Int8U min;
    Int8U sec;
    
}RTC_DATA;

extern NAYLSIS_KEY NalysisKey[MAX_NALYSIS_KEYS];

extern NALYSIS_REQUEST NalysisRequest;

extern EVENT_CONFIG_DATA EventConfigData;

extern RTC_DATA FileRtc;

extern Int8U MsgReceived;
extern Int8U EvrHeartBeat;
extern Int8U EvrFault;

extern Int8U GetConfigInfo;
extern Int8U GetConfigReceived;

extern Int8U SynchRequest;

extern void ProcessCommandCom1(Int16U cmd);
extern void ProcessCommandCom2(Int16U cmd);

void SendGetConfigInfo(void);
void SendGetRtc(void);
void GetFlashLog(Int32U page, char *pDataBuf);

#endif




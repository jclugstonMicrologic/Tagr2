
//###########################################################################
//
// FILE:   NFCOMMON_SciRxMachine.h
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 30 AUG 2012 | JDC  | Original version 
//###########################################################################


#ifndef SERIAL_MACHINE_H
#define SERIAL_MACHINE_H

#include "includes.h"

#define NALYSIS_ENABLED

#define SERIAL_HEAD1 (Int16U)0x0010 //DLE
#define SERIAL_HEAD2 (Int16U)0x0002 //STX

#define SIZEOF_HEAD ((Int16U)6) // [DLE][STX][nbrByteMSB][nbrByteLSB][cmdMSB][cmdLSB]
#define SIZEOF_TAIL (2)         // [crcMSB][crcLSB]
#define SIZEOF_HEAD_TAIL (SIZEOF_HEAD +SIZEOF_TAIL)

enum
{
    SCI_MAIN_PORT =0,
    SCI_DIAG_PORT
};


typedef struct
{
    Int32U timeoutTimer;
    Int16U machState;
    Int16U subState;

}COMMON;


typedef struct
{
    Int16U command;
    Int16U nbrBytes;
    Int16U rxPacketNbr;
    Int16U calcChecksum;

    Int16U byteCnt;

    char rxBuffer[4200];

    Int16U in;   // Queue input/write pointer
    Int16U full; // Queue full flag 
    Int16U out;  // Queue output/read pointer
    
    Int32U errorCnt;
    COMMON common;

    void (*callBackPtr)(Int16U);
    
}SERIAL_DATA;


extern SERIAL_DATA SerialDataCom1;
extern SERIAL_DATA SerialDataCom2;

extern void InitSerialReceiver( SERIAL_DATA *serialDataPtr, COM_TypeDef sciPort, Int32U baudRate);
int SerialRxMachine( SERIAL_DATA *serialDataPtr, char sciPort);

extern void SerialStateProcess(COMMON *commonDataPtr, Int16S state);
extern Int16U SerialCheckTimeout(COMMON *commonDataPtr, Int32U timeout);

#endif




//###########################################################################
//
// FILE:   AppPtcMessaging.h
//
// TITLE:  
//
//###########################################################################
//
//  Ver | dd mmm yyyy | Who  | Description of changes
// =====|=============|======|===============================================
//  000 | 30 MAY 2016 | JDC  | Original version 
//###########################################################################

#ifndef PROCESS_PTC_COMMANDS_H
#define PROCESS_PTC_COMMANDS_H

#include "includes.h"
#include "stm32f4xx.h"

#define EMP_ABH_POSITION        0x1004
#define EMP_STATE_OF_BAIL       0x1005
#define EMP_BELL_MAG_VALVE      0x1006
#define EMP_COND_VALVE_EMERG    0x1007

#define EMP_AB_SETUP_MODE       0x100F
#define EMP_IBH_POS             0x1010

#define EMP_AIR_BRAKE_SETUP     0x1013

#define EMP_REVERSER_DIR        0x1023
#define EMP_THROTTLE_POS        0x1027

#define EMP_EOT_BP_PRESS        0xA0001
#define EMP_EOT_EOM             0xA000C
#define EMP_EOT_END_IN_MOTION   0xA000D


void InitPtcMessage(void);

Int8U ProcessPtcCommand(char  *pdata, Int8U len);
Int16U BuildPtcMessage(char *pDataBuf);
void SendPtcMesssge(Int16U tag, Int16U value);
#endif




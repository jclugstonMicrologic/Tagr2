

#ifndef TAGR_H
#define TAGR_H

#include "includes.h"
#include "stm32f4xx.h"

#include "Common_Sci_Hi.h"
#include "Common_crc.h"

#include "CommonOsTimer.h"
#include "COMMON_FlashFd.h"

#include "ApplicationDigIn.h"
#include "ApplicationProcessCommands.h"
#include "ApplicationSciRxMachine.h"
#include "ApplicationLogMachine.h"
#include "ApplicationGpio.h"
#include "ApplicationOsTimer.h"

#include "AppExternalSensors.h"
#include "AppSDCard.h"

#include "usbh_core.h"
#include "usbh_usr.h"
#include "usbh_msc_core.h"
#include "flash_if.h"


#ifndef BOARD_TEST
 #define FW_VERSION "820-0246-a08"
#else
 #define FW_VERSION "820-0246-tst"
#endif

#define FW_DEC_VERSION ((Int16U)07)

void BlinkUploadProgressLed(void);
void BlinkUploadCompleteLed(void);

Int8U GetHeartBeatState(void);

#endif

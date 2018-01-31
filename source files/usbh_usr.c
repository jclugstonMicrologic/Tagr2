
/**
  ******************************************************************************
  * @file    FW_upgrade/src/usbh_usr.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   This file includes the usb host user callbacks
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbh_usr.h"
#include "flash_if.h"
#include "command.h"
#include "string.h"

#include "Tagr.h"

int QQ =0;

/** @addtogroup STM32F4-Discovery_FW_Upgrade
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBH_Usr_cb_TypeDef USR_Callbacks =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  USBH_USR_MSC_Application,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
};

FATFS fatfs;
FIL file;
FIL fileR;
DIR dir;
FILINFO fno;

FATFS fatfsHs;
FIL ChmmFile;

int Usb_Port =0;

static uint8_t USBH_USR_ApplicationState = USH_USR_FS_INIT;
extern USB_OTG_CORE_HANDLE          USB_OTG_Core;

static uint8_t USBH_USR_HS_ApplicationState = USH_USR_HS_INIT;
extern USB_OTG_CORE_HANDLE          USB_OTG_HS_Core;

extern USBH_HOST                    USB_HS_Host;


__IO uint32_t TimingDelay;
__IO uint32_t UploadCondition = 0x00;


int result;  
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  USBH_USR_Init
  *         Displays the message on LCD for host lib initialization
  * @param  None
  * @retval None
///  */
void USBH_USR_Init(void)
{
  static uint8_t startup = 0;  
  
  if(startup == 0 )
  {
    startup = 1;
    /* Initialize LEDs and Push_Button on STM32F4-Discovery**************************/
    
  }
  
  /* Setup SysTick Timer for 1 msec interrupts.
     ------------------------------------------
    1. The SysTick_Config() function is a CMSIS function which configure:
       - The SysTick Reload register with value passed as function parameter.
       - Configure the SysTick IRQ priority to the lowest value (0x0F).
       - Reset the SysTick Counter register.
       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
       - Enable the SysTick Interrupt.
       - Start the SysTick Counter.
    
    2. You can change the SysTick Clock source to be HCLK_Div8 by calling the
       SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8) just after the
       SysTick_Config() function call. The SysTick_CLKSourceConfig() is defined
       inside the misc.c file.

    3. You can change the SysTick IRQ priority by calling the
       NVIC_SetPriority(SysTick_IRQn,...) just after the SysTick_Config() function 
       call. The NVIC_SetPriority() is defined inside the core_cm4.h file.

    4. To adjust the SysTick time base, use the following formula:
                            
         Reload Value = SysTick Counter Clock (Hz) x  Desired Time base (s)
    
       - Reload Value is the parameter to be passed for SysTick_Config() function
       - Reload Value should not exceed 0xFFFFFF
   */
  if (SysTick_Config(SystemCoreClock / 1000))
  { 
    /* Capture error */ 
    while (1);
  }
}

/**
  * @brief  USBH_USR_DeviceAttached
  *         Displays the message on LCD on device attached
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAttached(void)
{
  
}

/**
  * @brief  USBH_USR_UnrecoveredError
  * @param  None
  * @retval None
  */
void USBH_USR_UnrecoveredError (void)
{
  /* Toggle Red LED in infinite loop */
  Fail_Handler();
}

/**
  * @brief  USBH_DisconnectEvent
  *         Device disconnect event
  * @param  None
  * @retval Staus
  */
void USBH_USR_DeviceDisconnected (void)
{
  /* Toggle Red LED in infinite loop: USB device disconnected */
  Fail_Handler();
}

/**
  * @brief  USBH_USR_ResetUSBDevice
  * @param  None
  * @retval None
  */
void USBH_USR_ResetDevice(void)
{
  /* callback for USB-Reset */
}

/**
  * @brief  USBH_USR_DeviceSpeedDetected
  *         Displays the message on LCD for device speed
  * @param  Device speed:
  * @retval None
  */
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
  if ((DeviceSpeed != HPRT0_PRTSPD_FULL_SPEED)&&(DeviceSpeed != HPRT0_PRTSPD_LOW_SPEED))
  {
    /* Toggle Red LED in infinite loop: USB device disconnected */
    Fail_Handler();
  }
}

/**
  * @brief  USBH_USR_Device_DescAvailable
  * @param  device descriptor
  * @retval None
  */
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{
  /* callback for device descriptor */
}

/**
  * @brief  USBH_USR_DeviceAddressAssigned
  *         USB device is successfully assigned the Address
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceAddressAssigned(void)
{
  /* callback for device successfully assigned the Address */
}


/**
  * @brief  USBH_USR_Conf_Desc
  * @param  Configuration descriptor
  * @retval None
  */
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
    USBH_InterfaceDesc_TypeDef *itfDesc,
    USBH_EpDesc_TypeDef *epDesc)
{
  /* callback for configuration descriptor */
}

/**
  * @brief  USBH_USR_Manufacturer_String
  * @param  Manufacturer String
  * @retval None
  */
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
  /* callback for  Manufacturer String */
}

/**
  * @brief  USBH_USR_Product_String
  * @param  Product String
  * @retval None
  */
void USBH_USR_Product_String(void *ProductString)
{
  /* callback for Product String */
}

/**
  * @brief  USBH_USR_SerialNum_String
  * @param  SerialNum_String
  * @retval None
  */
void USBH_USR_SerialNum_String(void *SerialNumString)
{
  /* callback for SerialNum_String */
}

/**
  * @brief  EnumerationDone 
  *         User response request is displayed to ask application jump to class
  * @param  None
  * @retval None
  */
void USBH_USR_EnumerationDone(void)
{
} 

/**
  * @brief  USBH_USR_DeviceNotSupported
  *         Device is not supported
  * @param  None
  * @retval None
  */
void USBH_USR_DeviceNotSupported(void)
{
  /* Toggle Red LED in infinite loop */
  Fail_Handler();
}


/**
  * @brief  USBH_USR_UserInput
  *         User Action for application state entry
  * @param  None
  * @retval USBH_USR_Status : User response for key button
  */
USBH_USR_Status USBH_USR_UserInput(void)
{
  /* callback for Key botton: set by software in this case */
  return USBH_USR_RESP_OK;
}

/**
  * @brief  USBH_USR_OverCurrentDetected
  *         Over Current Detected on VBUS
  * @param  None
  * @retval None
  */
void USBH_USR_OverCurrentDetected (void)
{
}


/**
  * @brief  USBH_USR_OverCurrentDetected
  *         Over Current Detected on VBUS
  * @param  None
  * @retval None
  */
void USBH_USR_TerminateUI (void)
{
    OsStopTimer( &OsTimer[TIMER_PROGRESS_LED] );  
                 
    UPLOAD_STATUS_LED_OFF;
    DONE_STATUS_LED_ON;        
    
    // re init the HS (CHMM) port
    USBH_Init(&USB_OTG_HS_Core, USB_OTG_HS_CORE_ID, &USB_HS_Host, &USBH_MSC_HS_cb, &USR_Callbacks);
    USBH_USR_HS_ApplicationState =USH_USR_HS_INIT;
}       

uint8_t USBH_USR_GetAppState(void)
{
    return USBH_USR_ApplicationState;
}

/**
  * @brief  USBH_USR_HS_MSC_Application
  *         Demo application for IAP thru USB mass storage
  * @param  None
  * @retval Staus
  */
int USBH_USR_HS_MSC_Application(void)
{
  uint16_t bytesWritten;
 
  Int8U eraseValue[32];  
    
  char locoId[6];    
  char fileName[15];
  char ascData[25];

 
  static uint8_t RAM_Buf[8448] = //BUFFER_SIZE] =
  {
    0x00
  };

  static Int8U MsgFailCnt =0;
  
//static uint8_t ledTest =0;

  typedef struct
  {    
    uint32_t marker;
    uint32_t dateTime;

    uint16_t fileVersion;
    uint16_t fwVersion;

    uint32_t spare;
  }EVR_HEADER;
  
  EVR_HEADER EvrHeader;  
     
  memset( &ascData, 0x00, sizeof(ascData));
  
  if( HCD_IsDeviceConnected(&USB_OTG_Core) == 1)
  {
    USBH_USR_HS_ApplicationState =USH_USR_HS_INIT;
    return 0;
  }
  
  switch (USBH_USR_HS_ApplicationState)
  {
  case USH_USR_HS_INIT:
    
    /* Initialises the File System*/
    if (f_mount( 0, &fatfsHs ) != FR_OK ) 
    {
      /* Fatfs initialisation fails */
      /* Toggle Red LED in infinite loop */
      Fail_Handler();
      return(-1); 
    }
    
    /* Flash Disk is write protected: Set ON Blue LED and Toggle Red LED in infinite loop */
    if (USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED)
    {
      /* Set ON Blue LED */
      /* Toggle Red LED in infinite loop */
      Fail_Handler();
    }
    
    /* Go to IAP menu */
    USBH_USR_HS_ApplicationState = USH_USR_HS_IAP;
    break;
    
  case USH_USR_HS_IAP:
    
    TimingDelay = 300;
    UploadCondition = 0x01;
    MsgFailCnt =0;
    
    SendGetRtc();
        
    ReadFlashAddress( (Int16U *)&EventConfigData, 2112, sizeof(EventConfigData) );
    
    memset(eraseValue, 0xff, sizeof(eraseValue) );    
    
    if( memcmp(&EventConfigData, &eraseValue, sizeof(eraseValue)) ==0 )  
    {
        // erased, not good, maybe request info from recorder now
        // SendGetConfigInfo();      
    }
       
    USBH_USR_HS_ApplicationState =2;
    break;    
  case 2:    
    if( MsgReceived !=3 &&
        MsgFailCnt <100 // SAER may not be responding, still allow download
      )
    {
        FileRtc.year =13;
        FileRtc.month=1;
        FileRtc.day  =1;
        FileRtc.hour =1;
        FileRtc.min  =1;
        FileRtc.sec  =1;

        MsgFailCnt ++;
    
        // try again
        
      //  MsgFailCnt =0;
        
        SendGetRtc();    
         
        return 0;
    }
    
    if( HCD_IsDeviceConnected(&USB_OTG_HS_Core) == 1)
    {           
        memset( locoId, 0x00, sizeof(locoId) );
        
        locoId[0] =EventConfigData.locoId[0];
        locoId[1] =EventConfigData.locoId[1];
        locoId[2] =EventConfigData.locoId[2];
        locoId[3] =EventConfigData.locoId[3];            
        locoId[4] =EventConfigData.locoId[4];            
            
        strcpy( fileName, "0:CHM");
        strcat( fileName, locoId);
        strcat( fileName, ".bli");
   
      
        if(f_open(&ChmmFile, fileName, FA_CREATE_NEW | FA_WRITE) == FR_OK)          
        {
//            UPLOAD_STATUS_LED_ON;                  
//            OsStartPeriodicTimer( &OsTimer[TIMER_PROGRESS_LED], (Int32U)200, BlinkUploadProgressLed );  

            EvrHeader.marker =0x55aaaa55;
            EvrHeader.dateTime =0x87654321;
            EvrHeader.fileVersion =0x0001;
            EvrHeader.fwVersion =FW_DEC_VERSION;
            EvrHeader.spare =0x0000;

            f_write (&ChmmFile, &EvrHeader, sizeof(EvrHeader), (void *)&bytesWritten);                  
                       
            // write buffer to file (this is the config info from EVR, need for file processing)
            f_write (&ChmmFile, &EventConfigData, 528, (void *)&bytesWritten);                  
                 
            /* Close file and filesystem */
            f_close (&ChmmFile);  
            //USBH_USR_HS_ApplicationState =10;
        }
        else if(f_open(&ChmmFile, fileName, FA_OPEN_EXISTING | FA_WRITE) == FR_OK &&
                ChmmLogData.info.logReady
                )  
        {
             f_lseek (&ChmmFile, ChmmFile.fsize);          
             while( LogDequeEventData( &ChmmLogData, RAM_Buf ) )  
             {
#if 0               
if(ledTest==0 )
    DONE_STATUS_LED_ON;               
else
    DONE_STATUS_LED_OFF;

ledTest =~ledTest;
#endif
 //              f_lseek (&ChmmFile, ChmmFile.fsize);
	
               // write buffer to file (this is the config info from EVR, need for file processing)
               //memset(RAM_Buf, 0x56, sizeof(RAM_Buf));
               result = f_write (&ChmmFile, RAM_Buf, ChmmLogData.info.packetSize, (void *)&bytesWritten);                  

             }
             
             ChmmLogData.info.logReady =0;
             
             /* Close file and filesystem */
             f_close (&ChmmFile);  
             
             //USBH_USR_HS_ApplicationState =10;
        }  

    }
    else
    {
        // CHMM removed, what to do???
    }
    break;   
  default:
    break;
  }
  return(0);
}

/**
  * @brief  USBH_USR_MSC_Application
  *         Demo application for IAP thru USB mass storage
  * @param  None
  * @retval Staus
  */
int USBH_USR_MSC_Application(void)
{
  uint16_t bytesWritten;
  uint64_t addr;
 
  Int8U eraseValue[32];  
    
  char locoId[6];    
  char fileName[15];
  char ascData[25];
  
  static uint8_t RAM_Buf[8448] = //BUFFER_SIZE] =
  {
    0x00
  };

  static uint32_t ReadLocation =0;
  static uint32_t ReadEnd =0;
  static Int8U GetFrontEnd =0;
  static Int8U MsgFailCnt =0;
  
  uint8_t readOffset;
  uint16_t packetsRead;  
  uint32_t tempAddr;  

  typedef struct
  {    
    uint32_t marker;
    uint32_t dateTime;

    uint16_t fileVersion;
    uint16_t fwVersion;

    uint32_t spare;
  }EVR_HEADER;
  
  EVR_HEADER EvrHeader;  
     

  if( Usb_Port ==1 )
  {
    USBH_USR_HS_MSC_Application();  
    
    return 0;
  }
    
  memset( &ascData, 0x00, sizeof(ascData));
  
  switch (USBH_USR_ApplicationState)
  {
  case USH_USR_FS_INIT:
    
    /* Initialises the File System*/
    if (f_mount( 0, &fatfs ) != FR_OK ) 
    {
      /* Fatfs initialisation fails */
      /* Toggle Red LED in infinite loop */
      Fail_Handler();
      return(-1); 
    }
    
    /* Flash Disk is write protected: Set ON Blue LED and Toggle Red LED in infinite loop */
    if (USBH_MSC_Param.MSWriteProtect == DISK_WRITE_PROTECTED)
    {
      /* Set ON Blue LED */
      /* Toggle Red LED in infinite loop */
      Fail_Handler();
    }
    
    /* Go to IAP menu */
    USBH_USR_ApplicationState = USH_USR_IAP;
    break;
    
  case USH_USR_IAP:
    
    TimingDelay = 300;
    UploadCondition = 0x01;
    MsgFailCnt =0;
    
    SendGetRtc();
        
    ReadFlashAddress( (Int16U *)&EventConfigData, 2112, sizeof(EventConfigData) );
    
    memset(eraseValue, 0xff, sizeof(eraseValue) );    
    
    if( memcmp(&EventConfigData, &eraseValue, sizeof(eraseValue)) ==0 )  
    {
        // erased, not good, maybe request info from recorder now
        // SendGetConfigInfo();      
    }
       
    USBH_USR_ApplicationState =2;
    break;    
  case 2:    
    if( MsgReceived !=3 &&
        MsgFailCnt <100 // SAER may not be responding, still allow download
      )
    {
        FileRtc.year =13;
        FileRtc.month=1;
        FileRtc.day  =1;
        FileRtc.hour =1;
        FileRtc.min  =1;
        FileRtc.sec  =1;

        MsgFailCnt ++;
    
        // try again
        
      //  MsgFailCnt =0;
        
        SendGetRtc();    
         
        return 0;
    }
    
    if( HCD_IsDeviceConnected(&USB_OTG_Core) == 1)
    {           
        memset( locoId, 0x00, sizeof(locoId) );
        
        locoId[0] =EventConfigData.locoId[0];
        locoId[1] =EventConfigData.locoId[1];
        locoId[2] =EventConfigData.locoId[2];
        locoId[3] =EventConfigData.locoId[3];            
        locoId[4] =EventConfigData.locoId[4];            
            
        strcpy( fileName, "0:SA");
        strcat( fileName, locoId);
        strcat( fileName, ".bli");
                   
        if(f_open(&file, "0:820-0246.st4", FA_READ) == FR_OK)      
        {           
            NVIC_SystemReset();
            
            /* Close file and filesystem */
            f_close (&file);      
            f_mount(0, NULL);                    
        }         
        else if(f_open(&file, fileName, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)          
        {
            UPLOAD_STATUS_LED_ON;
                    
            OsStartPeriodicTimer( &OsTimer[TIMER_PROGRESS_LED], (Int32U)200, BlinkUploadProgressLed );  

            EvrHeader.marker =0x55aaaa55;
            EvrHeader.dateTime =0x87654321;            
            EvrHeader.fwVersion =FW_DEC_VERSION;
            EvrHeader.spare =0x0000;

            if( LogData.info.packetSize ==104 ) 
            {
               EvrHeader.fileVersion =0x0002;
            }
            else
               EvrHeader.fileVersion =0x0001;
            
            f_write (&file, &EvrHeader, sizeof(EvrHeader), (void *)&bytesWritten);                  
                       
            // write buffer to file (this is the config info from EVR, need for file processing)
            f_write (&file, &EventConfigData, 528, (void *)&bytesWritten);                  
                 
            if( LogData.info.wrapAddr <LogData.info.address )            
            {
                ReadLocation =LogData.info.logStart;
                ReadEnd      =LogData.info.address;
            }
            else
            {
                // memory has wrapped
                ReadLocation =LogData.info.wrapAddr; 
                ReadEnd      =LogData.info.logEnd;
                
                // need to get the front end as well (most recent data is there)
                GetFrontEnd =1;
            }

            USBH_USR_ApplicationState =3;
        }
        else
        {
            // stop timers, turn LEDS on/off
            USBH_USR_TerminateUI();
            
            OsStartPeriodicTimer( &OsTimer[TIMER_FAIL_LED], (Int32U)75, BlinkUploadCompleteLed );
            
            USBH_USR_ApplicationState =100;
                
            return 0;          
        }
            
    }
    break;
   case 3:
    if( HCD_IsDeviceConnected(&USB_OTG_Core) == 1)      
    {         
//        if( LogData.info.wrapAddr <LogData.info.address )        
        {
            packetsRead =0;          
            
            readOffset =0; 
            
//            for( addr=LogData.info.logStart; addr<LogData.info.address; addr +=LogData.info.packetSize )
            for( addr=ReadLocation; addr<ReadEnd; addr +=LogData.info.packetSize )
            {
               if( LogData.info.packetSize ==104 ) 
               {
                 if( (addr %2112) ==2080 )
                 {
                   // keep on page boundaries
                   addr =(addr/2112 +1)*2112;
                  
                   readOffset =2112-2080;
                 }                 
               }
              
                // read a packet for file write (could do more pages, we'll see)
//                ReadFlashAddress( (Int16U *)RAM_Buf, addr, LogData.info.packetSize);            
//                f_write (&file, RAM_Buf, LogData.info.packetSize, (void *)&bytesWritten);
            
                tempAddr =addr;
                
                // check if current page is within a bad block, skip to next block if it is
                HandleBadBlock( &addr );
                
                // if addr was moved due to bad block, Read location must be adjusted
                // the same amount
                ReadLocation +=(addr-tempAddr);
                  
                ReadFlashAddress( (Int16U *)&RAM_Buf[addr-readOffset-ReadLocation], addr, LogData.info.packetSize);  
              
                // Reload IWDG counter
                IWDG_ReloadCounter();                                                                            
                               
                if( ++packetsRead >=5 )
                {
                    // allow main loop to run                
                    f_write (&file, RAM_Buf, (LogData.info.packetSize*packetsRead), (void *)&bytesWritten);                  
                  
                    ReadLocation =addr +LogData.info.packetSize;
                    return 0;
                }
            }       
            
            if( GetFrontEnd )
            {              
                GetFrontEnd =0;
               
                ReadLocation =LogData.info.logStart;
                ReadEnd      =LogData.info.address;               
                  
                return 0;
            }
            
            if( packetsRead <5 )
            {
              // write remaining packets
              f_write (&file, RAM_Buf, (LogData.info.packetSize*packetsRead), (void *)&bytesWritten);                  
                  
              ReadLocation =addr +LogData.info.packetSize;                  
            }                        
        }
                
        /* Close file and filesystem */
        f_close (&file);      
            
        if(f_open(&file, "0:ErrLog.asc", FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
        {
            f_write (&file, "ErrorLog\n", 9, (void *)&bytesWritten);                              
                
            sprintf( ascData, "CRC Error Count: %ld", SerialDataCom1.errorCnt);
                
            f_write (&file, ascData, sizeof(ascData), (void *)&bytesWritten);                    
            
            /* Close file and filesystem */
            f_close (&file);      
        }
           
        // stop timers, turn LEDS on/off
        USBH_USR_TerminateUI();
        
        USBH_USR_ApplicationState =100;
            
        return 0;
    }
   
    UploadCondition = 0x00;    
    break;
    
  default:
    break;
  }
  return(0);
}

/**
  * @brief  USBH_USR_DeInit
  *         Deint User state and associated variables
  * @param  None
  * @retval None
  */
void USBH_USR_DeInit(void)
{
  USBH_USR_ApplicationState = USH_USR_FS_INIT; 
  
  OsStopTimer( &OsTimer[TIMER_FAIL_LED]);
  DONE_STATUS_LED_OFF;           
}

/**
  * @brief  This function handles the program fail.
  * @param  None
  * @retval None
  */
void Fail_Handler(void)
{
//  while(1)
  {
    /* Toggle Red LED */
//    ALL_LEDS_ON;
    
//    IWDG_ReloadCounter();  
  }
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  { 
    TimingDelay--;
  }
}

/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

#ifdef FILE_READ
    if ((HCD_IsDeviceConnected(&USB_OTG_Core) == 1) )
    {
        if( f_open(&file, "0:EVENT.bli", FA_OPEN_ALWAYS | FA_READ) == FR_OK )        
        {
            memset( RAM_Buf, 0x00, sizeof(RAM_Buf) );
            f_read(&file, RAM_Buf, LogData.packetSize, (void *)&bytesWritten);                
        }
    }
#endif






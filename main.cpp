#include "Tagr.h"

Int32U CriticalSecCntr;
__IO uint32_t LsiFreq = 0;
__IO uint32_t CaptureNumber = 0, PeriodValue = 0;

Int8U HeartbeatMachState; 
Int8U BlinkCount;
Int32U HeartbeatTimer;    
    
void BlinkStatus1Led(void);
void BlinkStatus2Led(void);

Int8U SelfTest(void);
Int8U FaultMonitor(void);

void InitWatchDog(void);
void BlinkStatusCode(void);
    
void FileTransferMachine(USBH_HOST *pdev);
void SynchLogs(void);

char TempDisplayBuf[10];
int TheLength;


/* Private variables ---------------------------------------------------------*/
USB_OTG_CORE_HANDLE          USB_OTG_Core;
USBH_HOST                    USB_Host;

USB_OTG_CORE_HANDLE          USB_OTG_HS_Core;
USBH_HOST                    USB_HS_Host;

void WatchDogKick
(
    void
)
{
    // Update WWDG counter 
    WWDG_SetCounter(127);  
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: InitTargetHardware(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
void InitTargetHardware()
{    
//GPIO_InitTypeDef  GPIO_InitStructure;


//    ENTR_CRT_SECTION();
   
    // SysTick Config
    if(SysTick_Config(SystemCoreClock/1000))
    { 
        /* Capture error */ 
        while (1);
    }
  
//    EXT_CRT_SECTION();

    // Check if the system has resumed from IWDG reset
    if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
    {
        /* IWDGRST flag set */
        /* Clear reset flags */
        RCC_ClearFlag();
    }  
   
#ifndef DEBUG
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x08008000);
#endif
    
#ifdef USB_FS_ENABLE    
    /* Init Host Library */    
    USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);     
#endif    

#ifdef USB_HS_ENABLE    
    USBH_Init(&USB_OTG_HS_Core, USB_OTG_HS_CORE_ID, &USB_HS_Host, &USBH_MSC_HS_cb, &USR_Callbacks);     
#endif    
    
    InitSerialReceiver(&SerialDataCom1, COM1, 115200);
    InitSerialReceiver(&SerialDataCom2, COM2, 115200);    
    
    InitGpio();

    InitOsTimer(); 

    InitDigitalIn();

    InitExtSensors();
#ifndef DEBUG    
    InitWatchDog();
#endif

    ALL_LEDS_OFF;           
}


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: main(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
int main()
{    
    Int8U inputNbr;
    
    InitTargetHardware();
   
//    SelfTest();
             
    BlinkCount =0;

    HeartbeatTimer =0;

    while(1)
    {
        // main application
        //    .
        //    .
        //    .      
    
        // Reload IWDG counter
        IWDG_ReloadCounter();  

        SerialRxMachine(&SerialDataCom1, 0);
        SerialRxMachine(&SerialDataCom2, 1);        
   
        for( inputNbr =0; inputNbr<MAX_NUM_DIG_IN; inputNbr++ )
        {
            DigitalInMachine( &DigitalInData[inputNbr], inputNbr ); 
        }   

#ifdef USB_FS_ENABLE            
        // Host Task handler 
        Usb_Port =0;        
        USBH_Process(&USB_OTG_Core, &USB_Host);       
#endif        

#ifdef USB_HS_ENABLE
        if( USB_Host.gState == HOST_IDLE )
        {
            Usb_Port =1;
            USBH_Process(&USB_OTG_HS_Core, &USB_HS_Host);         
        }
#endif

#ifdef USB_FS_ENABLE
        // monitor the state of the file transfer to USB device
        FileTransferMachine( &USB_Host );        
#endif        
       
    }
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: FileTransferMachine
*|  Description:
*|----------------------------------------------------------------------------
*/
void FileTransferMachine
(
    USBH_HOST *pdev
)
{
    static Int8U FileTransferMachState =0;
    
    switch( FileTransferMachState ) 
    {
        case 0:
            if( pdev->gState == HOST_DEV_ATTACHED )
            {
                // enumeration of the USB takes awhile and
                // is blocking, so ensure a led is on 
                // otherwise it looks like unit has died
                OsStopTimer(&OsTimer[TIMER_FAULT_LED]);               
                ALL_LEDS_OFF;
                UPLOAD_STATUS_LED_ON;
                HeartbeatMachState =0;                                                
        
                FileTransferMachState =1;                
            }
            break;
        case 1:
            if( pdev->gState == HOST_IDLE )
            {
                // this catches if USB device has been
                // removed
                OsStopTimer( &OsTimer[TIMER_PROGRESS_LED] );  
            
                UPLOAD_STATUS_LED_OFF;              
                         
                FileTransferMachState =0;
            }
            break;  
    }
       
}

Int8U GetHeartBeatState(void)
{
    return HeartbeatMachState;
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: BlinkStatus1Led(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
void BlinkStatus1Led(void)
{
    static Int16U pinState =0;
    
    if( pinState ==0 )    
    {
        STATUS_LED1_ON;
    }
    else 
    {
        STATUS_LED1_OFF;
    }
       
    pinState = ~pinState;    
    
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: BlinkStatus1Led(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
void BlinkStatus2Led(void)
{
    static Int16U pinState =0;
    
    if( pinState ==0 )    
    {
        STATUS_LED2_ON;        
    }
    else 
    {
        STATUS_LED2_OFF;
        
        BlinkCount ++;
    }
   
    pinState = ~pinState;    
    
}


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: BlinkStatusCode(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
void BlinkStatusCode
(
    void
)
{
    static Int32U BlinkTimer =0;
    static Int8U BlinkState =0;
        
    switch( BlinkState )
    {
        case 0:           
            BlinkTimer =TimerTicks;
            
            if( EvrFault ||
                HeartbeatMachState ==2
              )
            {
//                OsStartPeriodicTimer( &OsTimer[TIMER_FAULT_LED], (Int32U)125, BlinkStatus2Led );                  
                // no blink, just make led solid no matter what the fault
                STATUS_LED2_ON;        
                BlinkState ++;
            }
            else
            {                            
                OsStopTimer(&OsTimer[TIMER_FAULT_LED]);            
                STATUS_LED2_OFF;
            }                
            break;
        case 1:
            if( !OsTimer[TIMER_FAULT_LED].running )
            {
                BlinkState =2;
            }
            
            if( !EvrHeartBeat && 
                 HeartbeatMachState ==2
              )
            {
                if( BlinkCount >=2 )
                {
                    OsStopTimer(&OsTimer[TIMER_FAULT_LED]);
                    BlinkState ++;
                }                                                 
            }
            else if( EvrFault ==1 )
            {
                if( BlinkCount >=4 )
                {
                    OsStopTimer(&OsTimer[TIMER_FAULT_LED]);              
                    BlinkState ++;
                }                                
            }
            else 
               BlinkState =2;            
            break;
        case 2:
            if( (TimerTicks - BlinkTimer) >1000 )
            {              
                BlinkCount =0;
                BlinkState =0;
            }            
            break;                
    }
}


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: BlinkUploadProgressLed(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
void BlinkUploadProgressLed(void)
{
    static Int16U pinState =0;
    
    if( pinState ==0 )    
    {
        UPLOAD_STATUS_LED_OFF;
    }
    else 
    {
        UPLOAD_STATUS_LED_ON;
    }
       
    pinState = ~pinState;    
    
}


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: BlinkUploadCompleteLed(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
void BlinkUploadCompleteLed(void)
{
    static Int16U pinState =0;
    
    if( pinState ==0 )    
    {
        DONE_STATUS_LED_OFF;
    }
    else 
    {
        DONE_STATUS_LED_ON;
    }
       
    pinState = ~pinState;    
    
}


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: BlinkAllLed(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
void BlinkAllLed(void)
{
    static Int16U pinState =0;
    
    if( pinState ==0 )    
    {
        ALL_LEDS_ON;  
    }
    else 
    {
        ALL_LEDS_OFF;        
    }
       
    pinState = ~pinState;    
    
}


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: SelfTest(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
Int8U SelfTest
(
    void
)
{    
    static Int32U PassTimer =0;
    
    Int8U status =0;
    
    ReadFlashId(0);
    
    if( ReadFlashId(1) )       
        status |= 0x01;

//    PassTimer =TimerTicks;
    
    if( !status )
    {
        // something failed
    }
    else
    {
        OsStartPeriodicTimer( &OsTimer[TIMER_LED_TEST], (Int32U)250, BlinkAllLed ); 
        
        while( (TimerTicks-PassTimer)<2000 )
        {
            // blink all leds
            // kick WatchDog
            IWDG_ReloadCounter();            
        }
        
        OsStopTimer( &OsTimer[TIMER_LED_TEST] );
        
        // ensure all leds off
        ALL_LEDS_OFF;                
       
        // status alive LED
        OsStartPeriodicTimer( &OsTimer[TIMER_ALIVE_LED], (Int32U)500, BlinkStatus1Led ); 
    }
    
    return status;
}


/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: FaultMonitor(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
Int8U FaultMonitor
(
    void
)
{   
    return true;
}

/*
*|----------------------------------------------------------------------------
*|  Module:
*|  Routine: SynchLogs(void)
*|  Description:
*|----------------------------------------------------------------------------
*/
void SynchLogs(void)
{
}

void InitWatchDog(void)
{
  
#ifdef WWD_DOG        
    // WWDG configuration
    // Enable WWDG clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, ENABLE);

    // WWDG clock counter = (PCLK1 (42MHz)/4096)/8 = 1281 Hz (~780 us)
    WWDG_SetPrescaler(WWDG_Prescaler_8);

    // Set Window value to 80; WWDG counter should be refreshed only when the counter
    // is below 80 (and greater than 64) otherwise a reset will be generated 
    WWDG_SetWindowValue(80);

    OsStartPeriodicTimer( &OsTimer[TIMER_WATCH_DOG_KICK], (Int32U)39, WatchDogKick );    
   
    // Enable WWDG and set counter value to 127, WWDG timeout = ~780 us * 64 = 49.92 ms 
    // In this case the refresh window is: ~780 * (127-80) = 36.6ms < refresh window < ~780 * 64 = 49.9ms    
    WWDG_Enable(127);
#else  
    /* Get the LSI frequency:  TIM5 is used to measure the LSI frequency */
    LsiFreq =32000;// GetLSIFrequency();
   
    /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
        dispersion) */
    /* Enable write access to IWDG_PR and IWDG_RLR registers */
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

    /* IWDG counter clock: LSI/32 */
    IWDG_SetPrescaler(IWDG_Prescaler_256);   //IWDG_Prescaler_32

    /* Set counter reload value to obtain 250ms IWDG TimeOut.
        Counter Reload Value = 250ms/IWDG counter clock period
                            = 250ms / (LSI/32)
                            = 0.25s / (LsiFreq/32)
                            = LsiFreq/(32 * 4)
                            = LsiFreq/128
    */
        
    IWDG_SetReload(LsiFreq/64);
    // we now have a 4second watchdog timeout
    
    /* Reload IWDG counter */
    IWDG_ReloadCounter();

    /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
    IWDG_Enable();    
#endif    
}

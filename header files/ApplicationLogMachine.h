
#ifndef LOG_MACHINE_H
#define LOG_MACHINE_H

#define RDU_LOG_PACKET_SIZE  104 //132 // keep a factor of 2112
#define RDU_LOG_ADDR_START   135168
#define RDU_LOG_ADDR_END     (NAND_BYTES_PER_DEVICE-2112)
                                      
                             
typedef enum
{
    LOG_IDLE_STATE =0,
    LOG_RUN_STATE,
    LOG_CONFIRM_STATE,   
    LOG_WRITE_FAIL_STATE,
    LOG_ERASE_STATE,
         
    LOG_NEXT_STATE,
    LOG_LAST_STATE
      
}LOG_STATES;

typedef struct
{
    Int64U wrapAddr;
    Int64U address;
    
    Int8U logReady;
    
    Int8U logErase;
    
    Int32U logEnd;
    Int32U logStart;

    Int16U packetSize;   // sizeof log packet (0-2112bytes)
    
    LOG_STATES machState;
    
}INFO;

typedef struct
{
    INFO info;
        
    Int8U buffer[4224];
    
    Int16U in;   // Queue input/write pointer
    Int16U full; // Queue full flag 
    Int16U out;  // Queue output/read pointer
    
    Int32U timeoutTimer;
       
    void (*callBackPtr1)(void);

    void (*callBackPtr2)(void);
  
    void (*callBackPtr3)(void);     
    
}LOG_DATA;

extern LOG_DATA LogData;

extern LOG_DATA ChmmLogData;

extern Int8U InitBBTableCreation;

void InitLog(LOG_DATA *pLogData, Int8U logType);
void LogMachine(LOG_DATA *pLogData);

Int16U LogQueEventData( LOG_DATA *pLogData, char *sourcePtr );
Int16U LogDequeEventData( LOG_DATA *pLogData, Int8U *tempPtr);

#endif



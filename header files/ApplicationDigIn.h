
#ifndef APPLICATION_DIG_IN_H
 #define APPLICATION_DIG_IN_H


/*
*****************************************************************************
 P U B L I C   I N C L U D E   F I L E S
*****************************************************************************
*/


/*
*****************************************************************************
 P U B L I C    M A C R O S
*****************************************************************************
*/
/* the number of switches */
#define MAX_NUM_DIG_IN   ((Int8U)6)

/* the number of counts that must elapse otherwise the switch is
   is considered transient */
#define BOUNCE_COUNTS          ((Int16U)10) // msec
#define HELD_ASSERT_COUNTS     ((Int16U)5)  // held counts

#define OFFSET ((Int16U)0x0001)


/* states of the state machine */
typedef enum 
{
   SWITCH_IDLE_STATE =0,
   NORMAL_NEGATED_STATE,
   DEBOUNCE_ASSERTING_STATE,
   ASSERTED_STATE,
   DEBOUNCE_NEGATING_STATE,
   HELD_ASSERTED_STATE,
   HELD_ASSERTED_STATE2,
   HELD_ASSERTED_NEW_RATE_STATE,
   DEBOUNCE_RELEASING_HELD_STATE,
   DEBOUNCE_RELEASING_HELD_STATE2
     
}SWITCH_MACHINE_STATES;


enum
{    
    FAULT_USB1 =0,
    FAULT_USB2,
    FAULT_3V_RST,     
    FAULT_REG1_RST,
    
    DIGIN_ONE,
    DIGIN_TWO,    
     
    DIGIN_NONE
};


/*
*****************************************************************************
 P U B L I C   T Y P E    D E F I N I T I O N S
*****************************************************************************
*/
typedef struct
{
   Int8U activeState;
   Int8U assertCounts;
   Int32U bounce;
   Int32U start;   
   SWITCH_MACHINE_STATES machineState;   
   
   Int8U (*digInCallBackPtr)( Int8U );
   void (*callBackPtr1)();
   void (*callBackPtr2)();
   void (*callBackPtr3)();
   void (*callBackPtr4)();

}DIGITAL_IN_DATA;


/*
*****************************************************************************
 P U B L I C   D A T A
****************************************************************************
*/
/* context data for each switch */
extern DIGITAL_IN_DATA DigitalInData[MAX_NUM_DIG_IN];

extern Int16U RealSwitchState;

extern Int8U Usb1OverCurrent;
extern Int8U Usb2OverCurrent;
extern Int8U Fault3VRstFlag;
extern Int8U FaultReg1RstFlag;

/*
*****************************************************************************
 P U B L I C   F U N C T I O N   P R O T O T Y P E S
*****************************************************************************
*/


void InitDigitalIn(void);

void DigitalInMachine( DIGITAL_IN_DATA *pDigIn, Int8U switchId);

Int8U GetSwitchStatus( Int8U switchId );

Int8U GetDigin1Status(Int8U digIn);
Int8U GetDigin2Status(Int8U digIn);


void FaultUsb1(void);
void FaultUsb1Negate(void);

void FaultUsb2(void);
void FaultUsb2Negate(void);

void Fault3VRst(void);
void Fault3VRstNegate(void);

void FaultReg1Rst(void);
void FaultReg1RstNegate(void);

#endif

/* ApplicationDigIn.h */








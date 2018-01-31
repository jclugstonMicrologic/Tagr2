/**
******************************************************************************
* @file    
* @author  
* @version 
* @date    
* @brief   
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
#include "stm32f4x7_eth.h"
#include "stm32f4x7_eth_bsp.h"
#include "netconf.h"

#include "CommonOsTimer.h"

#include "ApplicationEthernet.h"
#include "ApplicationSocket.h"
#include "httpserver.h"
#include "tcpserverclient.h"
#include "udpserverclient.h"

#include "AppPtcMessaging.h"


/* Private typedef -----------------------------------------------------------*/

enum
{
    SOCKET_HTTP_SERVER =0,
    
    SOCKET_TCP_SERVER,
    SOCKET_TCP_CLIENT,

    SOCKET_UDP_SERVER,
    SOCKET_UDP_CLIENT,
      
    SOCKET_LAST
};

typedef enum
{
    SOCKET_INIT_STATE =0,
    SOCKET_WAIT_CONNECTION_STATE,
    SOCKET_CONNECTION_STATE,
    
    SOCKET_LAST_STATE
                
}SOCKET_STATES;

typedef struct
{
    SOCKET_STATES machState;
    Int8U type;
    Int8U tcpConnect;
    Int32U timer;
    
    void (*callBackPtr)();
  
}SOCKET_DATA;

SOCKET_DATA SocketData;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/


/* Private functions ---------------------------------------------------------*/
void UdpSocketMachine(void);
void TcpSocketMachine(void);

void UpdSocketSend(void);

/**********************************************************************
	Function: void InitSocket()
 	Description: 
**********************************************************************/
void InitSocket
(
   Int8U socketType
)
{ 
    switch(socketType)
    {
        case SOCKET_HTTP_SERVER:
            /* Initialize the webserver module */
            IAP_httpd_init();          
            break;
        case SOCKET_TCP_SERVER:
            tcpserver_init();          
            SocketData.callBackPtr =TcpSocketMachine;
            break;
        case SOCKET_TCP_CLIENT:
            tcpclient_init();
            SocketData.callBackPtr =TcpSocketMachine;
            break;
        case SOCKET_UDP_SERVER:
            udpserver_init();
            SocketData.callBackPtr =UdpSocketMachine;
            break;
        case SOCKET_UDP_CLIENT:
            udpclient_init();           
            SocketData.callBackPtr =UdpSocketMachine;
            break;                        
        default:
            break;           
    }

}

/**********************************************************************
	Function: void InitSocketMachine(void)
 	Description: 
**********************************************************************/
void InitSocketMachine(void)
{
   
    /* Initialize the LwIP stack */
    LwIP_Init();

    SocketData.type =SOCKET_UDP_CLIENT;    
    SocketData.machState =SOCKET_INIT_STATE;
    SocketData.tcpConnect =FALSE;
    SocketData.timer =0;

    InitSocket(SocketData.type);
}


/**********************************************************************
	Function: void SocketMachine(void)
 	Description: 
**********************************************************************/
void SocketMachine(void)
{
    if( SocketData.callBackPtr !=NULL )
    {
        SocketData.callBackPtr();
    }
}

/**********************************************************************
	Function: void UdpSocketMachine(void)
 	Description: 
**********************************************************************/
void UdpSocketMachine(void)
{ 
    /* check if any packet received */
    if (ETH_CheckFrameReceived())
    {   
        switch( SocketData.machState )
        {
            case SOCKET_INIT_STATE:               
                InitPtcMessage();
                   
                SocketData.timer =TimerTicks;
                
                SocketData.machState =SOCKET_WAIT_CONNECTION_STATE;
                break;
            case SOCKET_WAIT_CONNECTION_STATE:
                // socket connection established
                SocketData.timer =TimerTicks;
                
                OsStartPeriodicTimer( &OsTimer[TIMER_PTC_MSG], (Int32U)1000, UpdSocketSend );  
                
                SocketData.machState =SOCKET_CONNECTION_STATE;
                break;
            case SOCKET_CONNECTION_STATE:
                break;
        }

        /* process received ethernet packet */
        LwIP_Pkt_Handle();
    }
    
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(TimerTicks);
    
}

/**********************************************************************
	Function: void TcpSocketMachine(void)
 	Description: 
**********************************************************************/
void TcpSocketMachine(void)
{ 
    /* check if any packet received */
    if (ETH_CheckFrameReceived())
    {   
        switch( SocketData.machState )
        {
            case SOCKET_INIT_STATE:               
                SocketData.timer =TimerTicks;
                
                SocketData.machState =SOCKET_WAIT_CONNECTION_STATE;
                break;
            case SOCKET_WAIT_CONNECTION_STATE:
                if( GetTcpSocketConnect() )
                {
                    // socket connection established
                    SocketData.machState =SOCKET_CONNECTION_STATE;
                }
                else if( (TimerTicks-SocketData.timer) >5000 )
                {
                    // try again
                    SocketData.machState =SOCKET_INIT_STATE;
                }             
                break;
            case SOCKET_CONNECTION_STATE:
                if( !GetTcpSocketConnect() )
                {
                    SocketData.machState =SOCKET_INIT_STATE;
                    
                    /* Generate a software reset */
                    NVIC_SystemReset();
                    while(1);
                }
                break;
        }

        /* process received ethernet packet */
        LwIP_Pkt_Handle();
    }
    
    /* handle periodic timers for LwIP */
    LwIP_Periodic_Handle(TimerTicks);
    
}

void UpdSocketSend(void)
{
    // build the body/data portion of the message
    SendPtcMesssge(EMP_THROTTLE_POS, 4);
    
    SendPtcMesssge(EMP_REVERSER_DIR, 1);
    
    // we can send multiple mesages here
}


void InitTcpSocketConnect(void)
{
    SocketData.tcpConnect =FALSE;
}

void SetTcpSocketConnect(void)
{
    SocketData.tcpConnect =TRUE;
}

u8 GetTcpSocketConnect(void)
{
    return SocketData.tcpConnect;
}



// end ApplicationSocket.c


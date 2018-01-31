/* Copyright (c) 2001, Swedish Institute of Computer Science.
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without
  * modification, are permitted provided that the following conditions
  * are met:
  * 1. Redistributions of source code must retain the above copyright
  *    notice, this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright
  *    notice, this list of conditions and the following disclaimer in the
  *    documentation and/or other materials provided with the distribution.
  * 3. Neither the name of the Institute nor the names of its contributors
  *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * httpd.c
 *
 * Author : Adam Dunkels <adam@sics.se>
 *
 */

#include "tcpserverclient.h"
#include "lwip/tcp.h"
#include "ApplicationEthernet.h"
#include "ApplicationSocket.h"

#include "AppPtcMessaging.h"
#include "ApplicationOsTimer.h" 
#include "CommonOsTimer.h"
   
#include <string.h>
#include <stdio.h>

static vu32 DataFla=0;
static vu32 size =0;
static __IO u8 resetpage=0;
static __IO uint32_t TotalData=0, checklogin=0;


/**
  * @brief  closes tcp connection
  * @param  pcb: pointer to a tcp_pcb struct
  * @param  hs: pointer to a http_state struct
  * @retval
  */
static void close_conn(struct tcp_pcb *pcb) //, struct http_state *hs)
{
  tcp_arg(pcb, NULL);
  tcp_sent(pcb, NULL);
  tcp_recv(pcb, NULL);
  
  tcp_close(pcb);
  
  InitTcpSocketConnect();
}


///!!!!!!!!!!!!!!!!!!!!!!!!!!TCP SERVER!!!!!!!!!!!!!!!!!!!


/**
  * @brief callback function for handling TCP HTTP traffic
  * @param arg: pointer to an argument structure to be passed to callback function
  * @param pcb: pointer to a tcp_pcb structure
  * @param p: pointer to a packet buffer
  * @param err: LwIP error code
  * @retval err
  */
static err_t tcpserver_recv(void *arg, struct tcp_pcb *pcb,  struct pbuf *p, err_t err)
{
  char *data, len;
    
  if (err == ERR_OK && p != NULL)
  {
    /* Inform TCP that we have taken the data */
    tcp_recved(pcb, p->tot_len);
    
    // echo payload back to client
    data = p->payload;
    len = p->tot_len;
    
//    if (strncmp(data, "$", 1) == 0)
    {
#if 0 
      send_data(pcb, hs);
#else 
      /* We cannot send more data than space available in the send
          buffer */
      if (tcp_sndbuf(pcb) < len )
      {
          len = tcp_sndbuf(pcb);
      }
    
      len =ProcessPtcCommand(data, len);

      err = tcp_write(pcb, data, len, 0);
#endif        

      /* Tell TCP that we wish be to informed of data that has been
      successfully sent by a call to the http_sent() function. */
  //    tcp_sent(pcb, NULL);//http_sent);  
      
    }
#if 0   
    else
    {
      // bad tcp/PTC request
      pbuf_free(p);
      close_conn(pcb, hs);
    }
#endif    
  }
    
  if (err == ERR_OK && p == NULL)
  {
    close_conn(pcb);
  }
  return ERR_OK;
}

/**
  * @brief  callback function on TCP connection setup ( on port 80)
  * @param  arg: pointer to an argument structure to be passed to callback function
  * @param  pcb: pointer to a tcp_pcb structure
  * &param  err: Lwip stack error code
  * @retval err
  */
static err_t tcpserver_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{
#if 0  
  struct http_state *hs;

  /* Allocate memory for the structure that holds the state of the connection */
  hs = mem_malloc(sizeof(struct http_state));

  if (hs == NULL)
  {
    return ERR_MEM;
  }

  /* Initialize the structure. */
  hs->file = NULL;
  hs->left = 0;

  /* Tell TCP that this is the structure we wish to be passed for our
     callbacks. */
  tcp_arg(pcb, hs);
#endif

  /* Tell TCP that we wish to be informed of incoming data by a call
     to the http_recv() function. */
  tcp_recv(pcb, tcpserver_recv);

  //tcp_err(pcb, conn_err);

  SetTcpSocketConnect();
        
  //tcp_poll(pcb, http_poll, 10);
  
  return ERR_OK;
}

/**
  * @brief  intialize HTTP webserver  
  * @param  none
  * @retval none
  */
void tcpserver_init(void)
{
  struct tcp_pcb *pcb;
  /*create new pcb*/
  pcb = tcp_new();
  /* bind HTTP traffic to pcb */
  tcp_bind(pcb, IP_ADDR_ANY, 32768); //80); // Anything that comes in on port 80
  
  /* start listening */
  pcb = tcp_listen(pcb);
  /* define callback function for TCP connection setup */
  tcp_accept(pcb, tcpserver_accept);
}

#if 0
static void client_close(struct tcp_pcb *pcb)
{
   tcp_arg(pcb, NULL);
   tcp_sent(pcb, NULL);
   tcp_recv(pcb, NULL);
   tcp_close(pcb);

//   printf("\nclient_close(): Closing...\n");
}
#endif

static err_t client_sent(void *arg, struct tcp_pcb *pcb, u16_t len)
{
   LWIP_UNUSED_ARG(arg);

//   printf("\nclient_sent(): Number of bytes ACK'ed is %d", len);

//  client_close(pcb);
   
   return ERR_OK;
}

static err_t client_connected(void *arg, struct tcp_pcb *pcb, err_t err)
{
   int bufspace =0;
   struct pbuf *pb;
   char *string ="Hello TCP Server\n";
     
//   u32_t timer =0;
   
   //while(1)
   {
   //if( (TimerTicks -timer) >5000 )
   {
 //  timer =TimerTicks;
   
   pb =pbuf_alloc(PBUF_TRANSPORT, 0, PBUF_REF);
   pb->payload =string;
   pb->len =pb->tot_len= strlen(string);
   
   LWIP_UNUSED_ARG(arg);

//   if (err != ERR_OK)
//      printf("\nclient_connected(): err argument not set to ERR_OK, but is value is %d\n", err);
//   else
   if (err == ERR_OK)   
   {             
      bufspace = tcp_sndbuf(pcb); // get space which can be used to sent the data
      
      if( bufspace )
      {
         tcp_write(pcb, pb->payload, pb->len, 0);
      }
      
   //   tcp_output(pcb);
      tcp_sent(pcb, client_sent);
      
      //OsStartOneShotTimer( &OsTimer[TIMER_CLIENT], (Int32U)5000, tcpclient_init );           
      SetTcpSocketConnect();
   }

//   pbuf_free(pb);
   }
   }
   return err;
}

void tcpclient_init(void)
{
   struct tcp_pcb *pcb;
   struct ip_addr dest;
   err_t ret_val;
   
   InitTcpSocketConnect();
     
//   IP4_ADDR(&dest, 208, 118, 125, 179);
   IP4_ADDR(&dest, 192, 168, 47, 84);   

   pcb = tcp_new();
   tcp_bind(pcb, IP_ADDR_ANY, 32768); //client port for outcoming connection
//   tcp_arg(pcb, NULL);
   ret_val = tcp_connect(pcb, &dest, 32768, client_connected); //server port for incoming connection
   
#if 0   
   if (ret_val != ERR_OK)
    printf("\tcp_connect(): Errors on return value, returned value is %d\n", ret_val);
#endif   
   
   tcp_recv(pcb, tcpserver_recv);
 
}


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

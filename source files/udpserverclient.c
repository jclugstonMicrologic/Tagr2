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

#include "udpserverclient.h"
#include "AppPtcMessaging.h"
   
#include <string.h>
#include <stdio.h>

/* Private variables ---------------------------------------------------------*/
static struct udp_pcb *UDPpcb;
static __IO uint32_t total_count=0;


/* Private function prototypes -----------------------------------------------*/

static void udp_echo_recv(void *arg, struct udp_pcb *Upcb, struct pbuf *pkt_buf,
                          struct ip_addr *addr, u16_t port);

static void udp_receive(void *arg, struct udp_pcb *Upcb, struct pbuf *pkt_buf,
                        struct ip_addr *addr, u16_t port);

//static tftp_opcode IAP_tftp_Flash_Write_Addressop(char *buf);
//static u16_t IAP_tftp_extract_block(char *buf);
//static void IAP_tftp_set_opcode(char *buffer, tftp_opcode opcode);
//static void IAP_tftp_set_block(char* packet, u16_t block);
//static err_t IAP_tftp_send_ack_packet(struct udp_pcb *upcb, struct ip_addr *to, int to_port, int block);


/* Private functions ---------------------------------------------------------*/

///!!!!!!!!!!!!!!!!!!!!!!!!!!UDP SERVER!!!!!!!!!!!!!!!!!!!


/**
  * @brief  Processes traffic received on UDP port 69
  * @param  args: pointer on tftp_connection arguments
  * @param  upcb: pointer on udp_pcb structure
  * @param  pbuf: pointer on packet buffer
  * @param  addr: pointer on the receive IP address
  * @param  port: receive port number
  * @retval none
  */
static void udp_receive(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                        struct ip_addr *addr, u16_t port)
{
  if( p != NULL  )
  {
    // do something ???
    /* free the pbuf */
    pbuf_free(p);  
  }
 
}



/**
  * @brief  Processes traffic received on UDP port 69
  * @param  args: pointer on tftp_connection arguments
  * @param  upcb: pointer on udp_pcb structure
  * @param  pbuf: pointer on packet buffer
  * @param  addr: pointer on the receive IP address
  * @param  port: receive port number
  * @retval none
  */
static void udp_echo_recv(void *arg, struct udp_pcb *upcb, struct pbuf *p,
                          struct ip_addr *addr, u16_t port)
{
  if( p != NULL  )
  {
    /* send received packet back to sender */
    udp_sendto(upcb, p, addr, port);
    /* free the pbuf */
    pbuf_free(p);  
  }
  
}


/**
  * @brief  
  * @param  none
  * @retval none
  */
void udpserver_init(void)
{
  err_t err;
  unsigned port =32768;
  
  /*create new pcb*/
  UDPpcb = udp_new();
  
  if (!UDPpcb)
  {
    return;
  }

  /* Bind this PCB to port 69  */
  err = udp_bind(UDPpcb, IP_ADDR_ANY, port);
  if (err == ERR_OK)
  {
    /* Initialize receive callback function  */
    udp_recv(UDPpcb, udp_echo_recv, NULL);
  } 
  else
  {
#ifdef USE_LCD
    LCD_SetTextColor(Red);
    LCD_DisplayStringLine(Line9, (uint8_t*)"Can not bind pcb");
#endif
  }
}



/**********************************************************************
	Function: void udpclient_send()
 	Description: 
**********************************************************************/
void udpclient_send( char *pData, int length)
{
   struct ip_addr dest;

   struct pbuf *p;
   
   IP4_ADDR(&dest, 192, 168, 47, 84);   
     
//   p = pbuf_alloc(PBUF_TRANSPORT, TFTP_ACK_PKT_LEN, PBUF_POOL);   
   p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_POOL);
   
   if( p ==NULL)
     return;
          
   p->tot_len =p->len =length;
   memcpy(p->payload, pData, length);
     
   udp_sendto(UDPpcb, p, &dest, 32768);  
   
   /* free the buffer pbuf */
   pbuf_free(p);      
}

/**********************************************************************
	Function: void udpclient_init()
 	Description: 
**********************************************************************/
void udpclient_init(void)
{
   struct ip_addr dest;
   err_t ret_val;
          
   IP4_ADDR(&dest, 192, 168, 47, 84);   

   UDPpcb = udp_new();
   
   udp_bind(UDPpcb, IP_ADDR_ANY, 32768); //client port for outcoming connection
//   tcp_arg(pcb, NULL);
   ret_val = udp_connect(UDPpcb, &dest, 32768);
   
#if 0   
   if (ret_val != ERR_OK)
    printf("udp_connect(): Errors on return value, returned value is %d\n", ret_val);
#endif
   
#if 0   
   p = pbuf_alloc(PBUF_TRANSPORT, TFTP_ACK_PKT_LEN, PBUF_POOL);   
   
   if( p ==NULL)
    return;
      
   p->tot_len =p->len =BuidlPtcMessage( p->payload, 0x1004);
     
   udp_sendto(UDPpcb, p, &dest, 32768);
   
   /* free the buffer pbuf */
   pbuf_free(p);   
#endif   

   /* Initialize receive callback function  */
   udp_recv(UDPpcb, udp_receive, NULL);
}



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

/**
******************************************************************************
* @file    main.c
* @author  MCD Application Team
* @version V1.0.0
* @date    31-October-2011
* @brief   Main program body
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
#include "ApplicationEthernet.h"
#include "httpserver.h"
#include "rdu.h"

/* Private typedef -----------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

uint32_t timingdelay;
uint16_t EthRegs[32];

/* Private functions ---------------------------------------------------------*/

/**
* @brief  Inits the Ethernet Periphrial.
* @param  None
* @retval None
*/
void InitEthernet(void)
{
  /* configure ethernet (GPIOs, clocks, MAC, DMA) */ 
  ETH_BSP_Config();
   
  EthLinkStatus = (ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_MICR_INT_OE) & 0x0004) >> 2 ;
  
}

///**
//* @brief  Inserts a delay time.
//* @param  nCount: number of 10ms periods to wait for.
//* @retval None
//*/
void EthDelay(uint32_t nCount)
{
  uint32_t ctr=TimerTicks;

  /* wait until the desired delay finish */  
  while((TimerTicks-ctr) < nCount)
  {     
  }
}


#ifdef  USE_FULL_ASSERT

/**
* @brief  Res the name of the source file and the source line number
*   where the assert_param error has occurred.
* @param  file: pointer to the source file name
* @param  line: assert_param error line source number
* @retval None
*/
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
  ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  
  /* Infinite loop */
  while (1)
  {}
}
#endif


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

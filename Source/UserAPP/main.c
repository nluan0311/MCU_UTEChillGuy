/******************** (C) COPYRIGHT 2023 SONiX *******************************
* COMPANY:		    SONiX
* DATE:				2023/11
* AUTHOR:			SA1
* IC:				SN32F400
*____________________________________________________________________________
*	REVISION	Date				User		Description
*	1.0			2023/11/06	        SA1			1. First version released
*												2. Compatible to CMSIS DFP Architecture in Keil MDK v5.X (http://www.keil.com/dd2/pack/)
*												3. Run HexConvert to generate bin file and show checksum after building.
*
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL 
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN 
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/
/******************** (C) PERSON EDITER BY TRAN NAM **************************
* DATE:				18/07/2025
* VERSION           2
*gmail              trannam63623@gmail.com
*****************************************************************************/
/*_____ I N C L U D E S _______________________________________________________________*/
#include <SN32F400.h>
#include <SN32F400_Def.h>
#include "..\Driver\GPIO.h"
#include "..\Driver\CT16B0.h"
#include "..\Driver\CT16B1.h"
#include "..\Driver\CT16B5.h"
#include "..\Driver\WDT.h"
#include "..\Driver\ADC.h"
#include "..\Driver\Utility.h"

/*_____I N C L U D E S  D R I V E R ____________________________________________________*/
#include "..\Module\KeyScan.h"
#include "..\Module\Buzzer.h"
#include "..\Module\Segment.h"

/*_____ I N C L U D E S M Y C O D E ____________________________________________________*/
#include "..\main_task\main_task.h"
/*_____ D E C L A R A T I O N S ________________________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);
//uint16_t read_key;
//uint16_t key_code;
//uint8_t key_val;
//uint16_t led_timer = 0;

/*_____ D E F I N I T I O N S ___________________________________________________________*/
#ifndef	SN32F407					//Do NOT Remove or Modify!!!
	#error Please install SONiX.SN32F4_DFP.0.0.18.pack or version >= 0.0.18
#endif
#define	PKG						SN32F407				//User SHALL modify the package on demand (SN32F407)

/*_____ M A C R O S ______________________________________________________________________*/


/*****************************************************************************
* Function		: main
* Description	: 
* Input			: None
* Output		: None
* Return		: None
* Note			: 
*****************************************************************************/
int	main(void)
{
	//uint8_t i;
	//User can configure System Clock with Configuration Wizard in system_SN32F400.c
	SystemInit();
	SystemCoreClockUpdate();				//Must call for SN32F400, Please do NOT remove!!!

	//Note: User can refer to ClockSwitch sample code to switch various HCLK if needed.


	//1. User SHALL define PKG on demand.
	//2. User SHALL set the status of the GPIO which are NOT pin-out to input pull-up.
	NotPinOut_GPIO_init();

	//--------------------------------------------------------------------------
	//User Code starts HERE!!!
	//enable reset pin function
	SN_SYS0->EXRSTCTRL_b.RESETDIS = 0;

	
	GPIO_Init();								//initial gpio
	
	WDT_Init();									//Set WDT reset overflow time ~ 250ms

	CT16B0_Init();						        //driver buzzer
	//SET P3.0 as PWM pin
	SN_PFPA->CT16B0_b.PWM0 = 1;


	CT16B1_Init();	
	CT16B5_Init();				        	//timer 1ms
	
	while (1)
	{
		__WDT_FEED_VALUE;
		main_task_run();
		
	}
}
/*****************************************************************************
* Function		: NotPinOut_GPIO_init
* Description	: Set the status of the GPIO which are NOT pin-out to input pull-up. 
* Input				: None
* Output			: None
* Return			: None
* Note				: 1. User SHALL define PKG on demand.
*****************************************************************************/
void NotPinOut_GPIO_init(void)
{
#if (PKG == SN32F405)
	//set P0.4, P0.6, P0.7 to input pull-up
	SN_GPIO0->CFG = 0x00A008AA;
	//set P1.4 ~ P1.12 to input pull-up
	SN_GPIO1->CFG = 0x000000AA;
	//set P3.8 ~ P3.11 to input pull-up
	SN_GPIO3->CFG = 0x0002AAAA;
#elif (PKG == SN32F403)
	//set P0.4 ~ P0.7 to input pull-up
	SN_GPIO0->CFG = 0x00A000AA;
	//set P1.4 ~ P1.12 to input pull-up
	SN_GPIO1->CFG = 0x000000AA;
	//set P2.5 ~ P2.6, P2.10 to input pull-up
	SN_GPIO2->CFG = 0x000A82AA;
	//set P3.0, P3.8 ~ P3.13 to input pull-up
	SN_GPIO3->CFG = 0x0000AAA8;
#endif
}

/*****************************************************************************
* Function		: HardFault_Handler
* Description	: ISR of Hard fault interrupt
* Input			: None
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/
void HardFault_Handler(void)
{
	NVIC_SystemReset();
}

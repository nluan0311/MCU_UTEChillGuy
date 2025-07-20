/******************** (C) COPYRIGHT 2021 SONiX *******************************
* COMPANY:	SONiX
* DATE:	    2023/11
* AUTHOR:	SA1
* IC:				SN32F400
*____________________________________________________________________________
*	REVISION	    Date				    User		Description
*	1.0				2023/11/07	SA1			1. First version released
*																
*____________________________________________________________________________
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS TIME TO MARKET.
* SONiX SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT OR CONSEQUENTIAL 
* DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE CONTENT OF SUCH SOFTWARE
* AND/OR THE USE MADE BY CUSTOMERS OF THE CODING INFORMATION CONTAINED HEREIN 
* IN CONNECTION WITH THEIR PRODUCTS.
*****************************************************************************/

/*_____ I N C L U D E S ____________________________________________________*/
#include "Buzzer.h"
#include "..\Driver\CT16B0.h"
/*_____ D E C L A R A T I O N S ____________________________________________*/
static uint8_t blink_led0_active = 0;
static uint8_t blink_led1_active = 0;
static uint8_t beep_active       = 0;
static uint16_t beep_timer       = 0;
static uint16_t beep_duration_ms = 0;

/*_____ D E F I N I T I O N S ______________________________________________*/
#define	SET_PITCH0_FREQ		261.6f
#define	SET_PITCH1_FREQ		293.7f
#define	SET_PITCH2_FREQ		329.6f
#define	SET_PITCH3_FREQ		349.2f
#define	SET_PITCH4_FREQ		392.0f
#define	SET_PITCH5_FREQ		440.0f
#define	SET_PITCH6_FREQ		493.9f

#define	SET_PITCH7_FREQ		523.3f
#define	SET_PITCH8_FREQ		587.3f
#define	SET_PITCH9_FREQ	    659.3f

#define LOUD_BEEP_FREQ_MAIN 2730.0f 

#define	HCLK_FREQ			12000000

#define HCLK_FREQ_MAIN      12000000

/*_____ M A C R O S ________________________________________________________*/

/***********************************
  | 7  | 8  | 9  |STOP|
  | 4  | 5  | 6  |STOP|
  | 1  | 2  | 3  |STOP|
  |STOP| 0  |STOP|STOP|
***********************************/
const uint16_t musical_table[] = {
	
	(uint16_t)(HCLK_FREQ/SET_PITCH7_FREQ),
	(uint16_t)(HCLK_FREQ/SET_PITCH8_FREQ),
	(uint16_t)(HCLK_FREQ/SET_PITCH9_FREQ),
     0,
	(uint16_t)(HCLK_FREQ/SET_PITCH4_FREQ),
	(uint16_t)(HCLK_FREQ/SET_PITCH5_FREQ),
	(uint16_t)(HCLK_FREQ/SET_PITCH6_FREQ),
    0,
	(uint16_t)(HCLK_FREQ/SET_PITCH1_FREQ),
	(uint16_t)(HCLK_FREQ/SET_PITCH2_FREQ),
	(uint16_t)(HCLK_FREQ/SET_PITCH3_FREQ),
	0,
    0,
	(uint16_t)(HCLK_FREQ/SET_PITCH0_FREQ),
	0,
	0
};
/*_____ F U N C T I O N S __________________________________________________*/
/*****************************************************************************
* Function		: set_buzzer_pitch
* Description	: set buzzer frequency
* Input			: pitch: input the pitch of buzzer
* Output		: None
* Return		: None
* Note			: None
*****************************************************************************/

/*_____ F U N C T I O N S __________________________________________________*/
void set_buzzer_pitch(uint8_t pitch)
{
	uint8_t buff_len = sizeof(musical_table)>>1;		//16bit size, buffer length /2
	if(pitch < buff_len)	
	{
		SN_CT16B0->MR9 = musical_table[pitch];
		SN_CT16B0->MR0 = SN_CT16B0->MR9 >> 1;
		
		SN_CT16B0->TMRCTRL = 0;					       //START TIMER
		SN_CT16B0->TMRCTRL = 1;					       //START TIMER
	}
	else
	{
		SN_CT16B0->MR0 = 0;		                        //disable buzzer;
	}
}
/*_____ F U N C T I O N S __________________________________________________*/




/*****************************************************************************
* Function     : buzzer_bip
* Description  : Activates the buzzer for a specified duration in milliseconds
* Input        : time_delay_ms – duration the buzzer should stay active
* Output       : None
* Return       : None
* Note         : Initializes and starts Timer to generate beep signal
*****************************************************************************/

void buzzer_bip(uint16_t time_delay_ms)
{
    uint16_t timer_period = (uint16_t)(HCLK_FREQ_MAIN / LOUD_BEEP_FREQ_MAIN);

    SN_CT16B0->MR9 = timer_period;
    SN_CT16B0->MR0 = timer_period >> 1;

    SN_CT16B0->TMRCTRL = 0;
    SN_CT16B0->TMRCTRL = 1;

    beep_timer = 0;
    beep_active = 1;
    beep_duration_ms = time_delay_ms;
}

/*****************************************************************************
* Function     : buzzer_stop
* Description  : Stops the buzzer and disables the timer
* Input        : None
* Output       : None
* Return       : None
* Note         : Called when beep duration ends or manually turned off
*****************************************************************************/

void buzzer_stop(void)
{
    SN_CT16B0->TMRCTRL = 0;     // stop Timer – stop buzzer
    beep_active = 0;            // No bbip
    beep_timer = 0;             // Reset time couter
}

/*****************************************************************************
* Function     : buzzer_update
* Description  : Updates the buzzer status based on elapsed time
* Input        : None
* Output       : None
* Return       : None
* Note         : Should be called regularly (e.g. in main loop or timer ISR)
*****************************************************************************/
void buzzer_update(void)
{
    if (beep_active)
    {
        beep_timer++;
        if (beep_timer >= beep_duration_ms)
        {
            buzzer_stop();   // Stop buzzer after
        }
    }
}

void buzzer_on(void)
{
    uint16_t timer_period = (uint16_t)(HCLK_FREQ_MAIN / LOUD_BEEP_FREQ_MAIN);

    SN_CT16B0->MR9 = timer_period;
    SN_CT16B0->MR0 = timer_period >> 1;

    SN_CT16B0->TMRCTRL = 0;
    SN_CT16B0->TMRCTRL = 1;
}

void buzzer_off(void)
{
    SN_CT16B0->TMRCTRL = 0;
}



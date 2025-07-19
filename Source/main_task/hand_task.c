/******************************************************/
#include "..\Driver\GPIO.h"
#include "..\Driver\CT16B0.h"
#include "..\Driver\CT16B1.h"
#include "..\Driver\CT16B5.h"
#include "..\Driver\SYS_con_drive.h"
#include "..\Driver\Utility.h"
/******************************************************/
#include "..\Module\KeyScan.h"
#include "..\Module\Buzzer.h"
#include "..\Module\Segment.h"
/******************************************************/
#include "..\main_task\main_task.h"
#include "..\main_task\hand_task.h"
/*_____ I N C L U D E S ____________________________________________________*/
#include <SN32F400.h>

/*_____ D E F I N I T I O N S ______________________________________________*/


/*_____ M A C R O S ________________________________________________________*/

/*_____ D E C L A R A T I O N S ____________________________________________*/

// static void buzzer_beep(void)
// {

// }
static void handle_button_press(void)
{
		if (timer_1ms_flag) 
		 {
				timer_1ms_flag = 0;
				read_key = KeyScan();

				if (read_key & KEY_PUSH_FLAG)
				{
					uint8_t key_val = read_key & 0xFF;

					switch (key_val)
					{
					case KEY_1:// SETTING USER
                    // TANG / GIam thoi gian
                    // time out 30s
                    // bip bip buzzer
                    // display segment 7 
                     //if (KEY_2)
#if 0
					SET_LED0_ON;
					Digital_DisplayDEC(3210);
					for (uint16_t t = 0; t < 500; /*in-loop*/) 					
					{
						if(timer_1ms_flag) 
						{ 
						timer_1ms_flag = 0; t++; 
						}
					}
 		
					SET_LED0_OFF;
#endif
					break;

					case KEY_14: // timer user
                          // TANG / GIam thoi gian
                    // time out 30s
                    // bip bip buzzer
                    // saver in flas 
                    // display segment 7 
                    // read 

#if 0
					SET_LED1_ON;
							Digital_DisplayDEC(0000);
					for (uint16_t t = 0; t < 500; /*in-loop*/)
					{
					if(timer_1ms_flag) 
					{ 
						timer_1ms_flag = 0; t++; 
					}
						Digital_Scan();
					}
				
					SET_LED1_OFF;
#endif
					break;
					default:
					break;
					}
				}
    }
}
/*
 * Project      : MCU V2 - Digital Clock Controller
 * Author       : Tran Nam
 * Date         : 20/07/2025
 * Email        : trannam6362@gmail.com
 * Version      : 2.0
 *
 * Description  : 
 * This source file is part of the digital clock system for SN8F5708_EVK / SN32F407_EVK.
 * It handles normal timekeeping, setup modes, alarm scheduling, and hardware control
 * including LED 7-segment display, buzzer, and flash memory storage.
 *
 * Notes        :
 * - Press SW3 to switch between hour/minute settings.
 * - Press SW16 to set alarm time, saved in FLASH.
 * - SW6/SW10 increase/decrease time values, with wrap-around logic.
 * - Timeout after 30s returns to normal mode.
 * - Buzzer and LED provide feedback on user interaction.
 */

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

#include "..\main_task\hand_task.h"
///****************************MACRO***************************************/
static uint16_t key_timeout_counter = 0;
#define KEY_TIMEOUT_MS 30000

/**************************Clock***********************/
static uint8_t   hours   = 23;
static uint8_t   minutes = 59;
static uint8_t   seconds = 55;

static uint8_t   arm_hours   = 0;
static uint8_t   arm_minutes = 0;
static uint8_t   arm_seconds = 0;

/**************************Clock***********************/
static uint16_t read_key;
static uint16_t key_code;
static uint8_t  key_val;
/**************************Clock***********************/
static uint8_t  key1_press_count = 0;
static uint8_t  key1_count = 0;
static uint8_t  key14_press_count = 0;
static uint8_t  key14_count = 0;

static uint8_t key1_pressed = 0;
static uint8_t key14_pressed = 0;

static uint16_t ms_count[2] = {0, 0};
static uint8_t led_state[2] = {0, 0}; // 0: TẮT, 1: BẬT


static uint8_t blink_led1_active = 0;
/**************************Clock***********************/

/*______________________________________________ D E F I N I T I O N S ______________________________________________*/

/*______________________________________________ D E C L A R A T I O N S ____________________________________________*/
//static void save_timer_flash(void);
//static void use_timer_flash(void);
static void normal_clock();
static void scan_moude();
static void clock_settings ();
static void clock_timer();
static void clock_update();
static void key_SW(void);

static void time_out();

static void led_blink_update(uint8_t led_num, uint16_t time_delay_ms);
static uint8_t blink_led0_active = 1;
/*-----------------------------------FUNSION----------------------------*/
static void key_SW(void)
{
    if (read_key & KEY_PUSH_FLAG)
    {
    uint8_t key_val = read_key & 0xFF;
     uint8_t mode = (key1_pressed) ? key1_count : key14_count;
        switch (key_val)
        {
            case KEY_1:
            if (!key14_pressed) 
            {
                key1_pressed = 1;
                key14_pressed = 0;

                key1_count++;
                if (key1_count > 2)
                {
                    key1_count = 0;
                    key1_pressed = 0; 
                }
                buzzer_bip(200);
                blink_led0_active = 1;
                //key_timeout_counter = 0
            }
            
            break;

            case KEY_4://Up
            if (key1_pressed) 
            {
               

                if (mode == 1)
                        hours = (hours + 1) % 24;
                else if (mode == 2)
                        minutes = (minutes + 1) % 60;

                SET_LED0_OFF;
			}
            if (key14_pressed) 
            {
               
                if (mode == 1)
                        arm_hours = (arm_hours + 1) % 24;
                else if (mode == 2)
                        arm_minutes = (arm_minutes + 1) % 60;

                SET_LED0_OFF;
			}
            buzzer_bip(200);
            break;
         
            case KEY_8://DOWN TIME
            if (key1_pressed) 
            {
                if (mode == 1)
                    hours = (hours == 0) ? 23 : hours - 1;
                else if (mode == 2)
                    minutes = (minutes == 0) ? 59 : minutes - 1;

                SET_LED0_ON;
            }
            if (key14_pressed) 
            {
               
                if (mode == 1)
                        arm_hours = (arm_hours + 1) % 24;
                else if (mode == 2)
                        arm_minutes = (arm_minutes + 1) % 60;

                SET_LED0_OFF;
                
			}
            buzzer_bip(200);
            break;

            case KEY_14:
            if (!key1_pressed)
            {
                key14_pressed = 1;
                key1_pressed = 0;

                key14_count++;
                if (key14_count > 2)
                {
                    key14_count = 0;
                    key14_pressed = 0; 
                }

                blink_led0_active = 1;
                 buzzer_bip(200);
            }
           
            break;


            default:
                break;
        }
				read_key = 0;
    }
}
static void led_blink_update(uint8_t led_num, uint16_t time_delay_ms)
{
    ms_count[led_num]++;
    if (ms_count[led_num] >= time_delay_ms)
    {
        ms_count[led_num] = 0;
        if (led_num == 0)
        {
            if (led_state[0])
            {
                SET_LED0_OFF;
                led_state[0] = 0;
            }
            else
            {
                SET_LED0_ON;
                led_state[0] = 1;
            }
        }
        else if (led_num == 1)
        {
            if (led_state[1])
            {
                SET_LED1_OFF;
                led_state[1] = 0;
            }
            else
            {
                SET_LED1_ON;
                led_state[1] = 1;
            }
        }
    }
}
static void clock_update(void)
{
    if (timer_1s_flag) 
    {
        timer_1s_flag = 0;
        if (++seconds >= 60)
         {
            seconds = 0;
            if (++minutes >= 60)
             {
                minutes = 0;
                if (++hours >= 24) 
                {
                    hours = 0;
                }
            }
        }
				 	
    }
}
static void normal_clock()
{
    if (key14_pressed && !key1_pressed)
        Digital_DisplayDEC(arm_hours * 100 + arm_minutes); // Hiển thị hẹn giờ khi đang nhấn SW14
    else
        Digital_DisplayDEC(hours * 100 + minutes);         // Hiển thị đồng hồ bình thường
}

static void scan_moude()
{
    if(timer_1ms_flag)
    {
        timer_1ms_flag = 0;
        Digital_Scan();
        key_code = KeyScan();
        buzzer_update();
        // key_val  = read_key & 0xFF;
        read_key = key_code;
        time_out();
        if(key1_count==2)
        {
        if(blink_led0_active)
            {
                led_blink_update(0, 500); // Nháy 0.5s
            }
        }

    }
}

static void time_out()
{
    if (key1_pressed || key14_pressed)
    {
        key_timeout_counter++;
        if (key_timeout_counter >= KEY_TIMEOUT_MS)
        {
            key1_pressed = 0;
            key14_pressed = 0;
            key1_count = 0;
            key14_count = 0;
            //blink_led0_active = 0;
            key_timeout_counter = 0;
        }
    }
    else
    {
        key_timeout_counter = 0; // reset khi không ở trong chế độ
    }
}

/*---------------------------INIT/ MAIN------------------------------------------*/

void main_task_init(void) { 
//    currentState = NORMAL_USER;
//    timeoutCounter = 0;

}
#if 1
void main_task_run(void)
{
clock_update();
normal_clock();
scan_moude();
key_SW();
	
   // handle_button_press();
}
#endif

#if 0
#endif
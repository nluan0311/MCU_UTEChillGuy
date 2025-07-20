/*
 * Project      : MCU V2 - Digital Clock Controller
 * Author       : Tran Nam
 * Date         : 18/07/2025
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
#include "..\Driver\SPI.h"
/******************************************************/
#include "..\Module\KeyScan.h"
#include "..\Module\Buzzer.h"
#include "..\Module\Segment.h"
#include "..\Module\sst_flash.h"
/******************************************************/

//#include "..\main_task\hand_task.h"
///****************************MACRO***************************************/
#define FLASH_ALARM_ADDR   0x7C00    

#define KEY_TIMEOUT_MS 30000

#define IS_VALID_HOUR(h)    ((h) < 24)
#define IS_VALID_MINUTE(m)  ((m) < 60)

/*_____ C L O C K   T I M E _________________________________________________*/
static uint8_t   hours   = 23;
static uint8_t   minutes = 59;
static uint16_t   seconds = 55;

/*_____ A L A R M   T I M E _________________________________________________*/

static uint8_t   arm_hours   = 0;
static uint8_t   arm_minutes = 0;
static uint8_t   arm_seconds = 0;

/*_____ K E Y   R E A D I N G _______________________________________________*/
static uint16_t read_key;
static uint16_t key_code;
static uint8_t  key_val;

/*_____ K E Y   P R E S S   C O U N T E R S _________________________________*/
static uint8_t  key1_press_count = 0;
static uint8_t  key1_count = 0;
static uint8_t  key14_press_count = 0;
static uint8_t  key14_count = 0;

static uint16_t key_timeout_counter = 0;

/*_____ K E Y   S T A T U S _________________________________________________*/
static uint8_t key1_pressed = 0;
static uint8_t key14_pressed = 0;

/*_____ L E D   S T A T E ____________________________________________________*/
static uint16_t ms_count[2] = {0, 0};
static uint8_t led_state[2] = {0, 0}; 

static uint8_t blink_led1_active = 0;

static uint16_t alarm_tick_ms = 0;
static uint8_t alarm_buzzer_state = 0;
static uint8_t alarm_active = 0;
static uint16_t alarm_blink_counter = 0;

static uint8_t alarm_time_changed = 0;
static uint8_t prev_arm_hours = 0;
static uint8_t prev_arm_minutes = 0;


static uint8_t alarm_enable_flag = 1; 

/**************************CHEK********************/

void check_and_activate_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds);


/*_____ F U N C T I O N   P R O T O T Y P E S   &   L O C A L   S T A T E _____*/
/*=============== INTERNAL FUNCTION DECLARATIONS ===============*/
/* These functions control clock logic, key scanning, and LED update */
/*===== Clock System Core Functions =====*/
static void normal_clock();
static void clock_settings ();
static void clock_timer();
static void clock_update();
static void  led_blink();
/*===== Input Scanning & Handling =====*/
static void key_SW(void);
static void time_out();
static void scan_moude();
void bip_5s();
/*===== LED Management =====*/
static void led_blink_update(uint8_t led_num, uint16_t time_delay_ms);
static uint8_t blink_led0_active = 1;
void init_alarm_time_if_needed(void);
void load_alarm_time_from_flash(void);
/*===== Save Flash =====*/
/*-----------------------------------FUNSION----------------------------*/

void save_alarm_time_to_flash(void)
{
    uint8_t data[8] = {
        0xA5,             
        arm_hours,       
        arm_minutes,   
        arm_seconds,     
        alarm_enable_flag,
        0x00,             
        0x00,             
        0x5A             
    };

    flash_erase_sector(FLASH_ALARM_ADDR);
    flash_write(FLASH_ALARM_ADDR, 8, data);
}



#if 1
void load_alarm_time_from_flash(void)
{
    uint8_t data[8];  
    flash_read(FLASH_ALARM_ADDR, 8, data);

    if (data[0] == 0xA5 && IS_VALID_HOUR(data[1]) && IS_VALID_MINUTE(data[2]))
    {
        arm_hours         = data[1];
        arm_minutes       = data[2];
        arm_seconds       = data[3];
        alarm_enable_flag = data[4];
      
    }
    else
    {
        arm_hours = 0;
        arm_minutes = 0;
        arm_seconds = 0;
        alarm_enable_flag = 1;
    }
}


void init_alarm_time_if_needed(void)
{
    uint8_t data[8];
    flash_read(FLASH_ALARM_ADDR, 8, data);

    if (data[0] != 0xA5)
    {
        data[0] = 0xA5;
        data[1] = 0;
        data[2] = 0;
        data[3] = 0;
        data[4] = 1;     
        data[5] = 0;
        data[6] = 0;
        data[7] = 0x5A;  

        flash_erase_sector(FLASH_ALARM_ADDR);
        flash_write(FLASH_ALARM_ADDR, 8, data);
    }
}




#endif

static void alarm_buzzer_handler(void)
{
    alarm_blink_counter++;
    if (alarm_blink_counter >= 500)  
    {
        alarm_blink_counter = 0;
        if (alarm_buzzer_state)
        {
            buzzer_off();
            alarm_buzzer_state = 0;
        }
        else
        {
            buzzer_on();
            alarm_buzzer_state = 1;
        }
    }
}
void bip_5s()
{
if (alarm_active)
{
    alarm_tick_ms++;

    alarm_blink_counter++;
    if (alarm_blink_counter >= 500) // 500ms = 0.5s
    {
        alarm_blink_counter = 0;

        if (alarm_buzzer_state)
        {
            buzzer_off();
            alarm_buzzer_state = 0;
        }
        else
        {
            buzzer_on();
            alarm_buzzer_state = 1;
        }
    }

    if (alarm_tick_ms >= 5000)
    {
        alarm_active = 0;
        alarm_tick_ms = 0;
        alarm_blink_counter = 0;
        alarm_buzzer_state = 0;
        buzzer_off(); 
    }
}
}


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
									  blink_led0_active = 0; 
									  SET_LED0_OFF;
                    led_state[0] = 0;
                }
								else{blink_led0_active = 1;}
                buzzer_bip(200);
                
            }

            break;

            case KEY_4://Up
            if (key1_pressed) 
            {    
                if (mode == 1)
                hours = (hours + 1) % 24;
                else if (mode == 2)
                minutes = (minutes + 1) % 60;

                buzzer_bip(200);
            }
            if (key14_pressed) 
            {

            if (mode == 1)
                arm_hours = (arm_hours + 1) % 24;
                else if (mode == 2)
                arm_minutes = (arm_minutes + 1) % 60;
								
								//save_alarm_time_to_flash();
								alarm_time_changed = 1;
                buzzer_bip(200);
            }
            
            break;

            case KEY_8://DOWN TIME
            if (key1_pressed) 
            {
                if (mode == 1)
                hours = (hours == 0) ? 23 : hours - 1;
                else if (mode == 2)
                minutes = (minutes == 0) ? 59 : minutes - 1;
								buzzer_bip(200);
            }
            if (key14_pressed) 
            {
                if (mode == 1)
                arm_hours = (arm_hours == 0) ? 23 : arm_hours - 1;
                else if (mode == 2)
                arm_minutes = (arm_minutes == 0) ? 59 : arm_minutes - 1;			
				alarm_time_changed = 1;
				buzzer_bip(200);
            }
            
            break;

            case KEY_14:
            if (!key1_pressed)
            {
                key14_pressed = 1;
                key1_pressed = 0;
                key14_count++;
                if (key14_count == 1)
                {
                
                    prev_arm_hours = arm_hours;
                    prev_arm_minutes = arm_minutes;
                    blink_led0_active = 1;
                }
                else if (key14_count > 2)
                {
                    
                    if (alarm_time_changed)
                    {
                        save_alarm_time_to_flash();
                        alarm_time_changed = 0;
                    }
                    key14_count = 0;
                    key14_pressed = 0;
                    blink_led0_active = 0;
                    SET_LED0_OFF;
                    led_state[0] = 0;
                }
                else
                {
                    blink_led0_active = 1;
                }
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
    if (key14_pressed && key14_count > 0)
        Digital_DisplayDEC(arm_hours * 100 + arm_minutes); 
    else
        Digital_DisplayDEC(hours * 100 + minutes);         
}

static void scan_moude()
{
    if(timer_1ms_flag)
    {
        timer_1ms_flag = 0;
        Digital_Scan();
        key_code = KeyScan();
        buzzer_update();
        
        read_key = key_code;
        time_out();
        led_blink();
			  bip_5s();


    }
}
static void led_blink()  
{
    if (blink_led0_active)
    {
    
        if ((key1_pressed && key1_count > 0 && key1_count <= 2) || 
            (key14_pressed && key14_count > 0 && key14_count <= 2))
        {
            led_blink_update(0, 500); 
    }
}

static void time_out()
{
    if (key1_pressed || key14_pressed)
    {
        key_timeout_counter++;
        if (key_timeout_counter >= KEY_TIMEOUT_MS)
        {
            if (key14_pressed && alarm_time_changed)
            {
               
                save_alarm_time_to_flash();
                alarm_time_changed = 0;
            }
            key1_pressed = 0;
            key14_pressed = 0;
            key1_count = 0;
            key14_count = 0;
            blink_led0_active = 0;
            key_timeout_counter = 0;
            SET_LED0_OFF;
            led_state[0] = 0;
        }
    }
    else
    {
        key_timeout_counter = 0; 
    }
}
void check_and_activate_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (alarm_enable_flag &&
        hours == arm_hours &&
        minutes == arm_minutes &&
        seconds <= 1 &&
        !alarm_active)
    {
        alarm_active = 1;
        alarm_tick_ms = 0;
        alarm_blink_counter = 0;
        alarm_buzzer_state = 0;
    }
}

/*---------------------------INIT/ MAIN------------------------------------------*/

void main_task_init(void) { 

	    init_alarm_time_if_needed();     
        load_alarm_time_from_flash();
        alarm_active = 0;


}
#if 1
void main_task_run(void)
{
    clock_update();
    normal_clock();
    scan_moude();
    key_SW();
    check_and_activate_alarm(hours, minutes, seconds);

}

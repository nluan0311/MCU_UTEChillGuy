/*
 * Project      : MCU V2 - Digital Clock Controller
 * Author       : Tran Nam
 * Date         : 22/07/2025
 * Email        : trannam6362@gmail.com
 * Version      : 2.2
 *
 * Description  : 
 * This source file is part of the digital clock system for SN8F5708_EVK / SN32F407_EVK.
 * It handles normal timekeeping, setup modes, alarm scheduling, and hardware control
 * including LED 7-segment display, buzzer, and flash memory storage.
 *
 * Update 2.1   : Added segment_blink function to make hour/minute digits blink
 * during time/alarm setting modes for better user feedback.
 * Update 2.2   : Refactored all key handling logic into hand_task.c.
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
#include "main_task.h"
#include "hand_task.h"
/******************************************************/

/****************************MACRO***************************************/
#define FLASH_ALARM_ADDR   0x7C00    
#define KEY_TIMEOUT_MS 30000

#define IS_VALID_HOUR(h)    ((h) < 24)
#define IS_VALID_MINUTE(m)  ((m) < 60)

/*_____ C L O C K   T I M E _________________________________________________*/
uint8_t   hours   = 23;
uint8_t   minutes = 59;
uint16_t  seconds = 59;

/*_____ A L A R M   T I M E _________________________________________________*/
uint8_t   arm_hours   = 0;
uint8_t   arm_minutes = 0;
uint8_t   arm_seconds = 0;

/*_____ K E Y   R E A D I N G _______________________________________________*/
uint16_t read_key;
static uint16_t key_code;

/*_____ K E Y   P R E S S   C O U N T E R S _________________________________*/
uint8_t  key1_count = 0;
uint8_t  key14_count = 0;
uint16_t key_timeout_counter = 0;

/*_____ K E Y   S T A T U S _________________________________________________*/
uint8_t key1_pressed = 0;
uint8_t key14_pressed = 0;

/*_____ L E D   &   B L I N K   S T A T E ___________________________________*/
static uint16_t ms_count[2] = {0, 0};
uint8_t led_state[2] = {0, 0}; 
uint8_t blink_led0_active = 0;

uint16_t segment_blink_timer = 0;
uint8_t segment_display_on = 1;

/*_____ A L A R M   S T A T E _______________________________________________*/
static uint16_t alarm_tick_ms = 0;
static uint8_t alarm_buzzer_state = 0;
static uint8_t alarm_active = 0;
static uint16_t alarm_blink_counter = 0;
uint8_t alarm_time_changed = 0;
uint8_t alarm_enable_flag = 1; 

/**************************FUNCTION PROTOTYPES*******************************/
void check_and_activate_alarm(uint8_t hours, uint8_t minutes, uint8_t seconds);
void load_alarm_time_from_flash(void);
void init_alarm_time_if_needed(void);
static void update_display_logic(void);
static void segment_blink(uint16_t time_blink);
static void clock_update(void);
static void key_SW(void);
static void handle_timeout(void);
static void handle_led_blinking(void);
void handle_alarm_buzzer(void);


/*-----------------------------------FUNCTIONS------------------------------*/

/**
 * @brief Saves the current alarm time and enable status to flash memory.
 */
void save_alarm_time_to_flash(void)
{
    uint8_t data[8] = {
        0xA5,             // Magic number to validate data
        arm_hours,       
        arm_minutes,   
        arm_seconds,     
        alarm_enable_flag,
        0x00,             // Reserved
        0x00,             // Reserved
        0x5A              // Complementary magic number
    };

    flash_erase_sector(FLASH_ALARM_ADDR);
    flash_write(FLASH_ALARM_ADDR, 8, data);
}

/**
 * @brief Loads the alarm time from flash memory. If data is invalid, loads defaults.
 */
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
        alarm_enable_flag = 1; // Default to enabled
    }
}

/**
 * @brief Checks if flash has valid alarm data. If not, initializes it.
 */
void init_alarm_time_if_needed(void)
{
    uint8_t data[8];
    flash_read(FLASH_ALARM_ADDR, 8, data);

    if (data[0] != 0xA5) // If magic number is not present, initialize flash
    {
        data[0] = 0xA5;
        data[1] = 0;    // arm_hours
        data[2] = 0;    // arm_minutes
        data[3] = 0;    // arm_seconds
        data[4] = 1;    // alarm_enable_flag
        data[5] = 0;
        data[6] = 0;
        data[7] = 0x5A;

        flash_erase_sector(FLASH_ALARM_ADDR);
        flash_write(FLASH_ALARM_ADDR, 8, data);
    }
}

/**
 * @brief Handles the 5-second alarm buzzer and LED blinking sequence.
 */
void handle_alarm_buzzer()
{
    if (alarm_active)
    {
        alarm_tick_ms++;

        alarm_blink_counter++;
        if (alarm_blink_counter >= 500) // Toggle every 500ms
        {
            alarm_blink_counter = 0;
            alarm_buzzer_state = !alarm_buzzer_state;
            if (alarm_buzzer_state) buzzer_on();
            else buzzer_off();
        }

        if (alarm_tick_ms >= 5000) // Stop after 5 seconds
        {
            alarm_active = 0;
            alarm_tick_ms = 0;
            alarm_blink_counter = 0;
            alarm_buzzer_state = 0;
            buzzer_off(); 
        }
    }
}

/**
 * @brief Manages the blinking of the 7-segment display during setting modes.
 * @param time_blink The interval in milliseconds for one blink cycle (on/off).
 */
static void segment_blink(uint16_t time_blink)
{
    if (key1_pressed || key14_pressed)
    {
        segment_blink_timer++;
        if (segment_blink_timer >= time_blink)
        {
            segment_blink_timer = 0;
            segment_display_on = !segment_display_on; // Toggle display state
        }
    }
    else
    {
        // If not in a setting mode, ensure the display is always on
        segment_display_on = 1;
        segment_blink_timer = 0;
    }
}

/**
 * @brief Processes key presses to change modes and adjust time/alarm values.
 */
static void key_SW(void)
{
    if (read_key & KEY_PUSH_FLAG)
    {
        uint8_t key_val = read_key & 0xFF;

        // Reset timeout counter on any key press
        key_timeout_counter = 0;

        switch (key_val)
        {
            case KEY_1:
                handle_key1_press();
                break;

            case KEY_4:
                handle_key4_press();
                break;

            case KEY_8:
                handle_key8_press();
                break;

            case KEY_14:
                handle_key14_press();
                break;
        }
        read_key = 0; // Consume the key press
    }
}

/**
 * @brief Updates the 7-segment display buffer based on current mode and blink state.
 */
static void update_display_logic(void)
{
    // Determine which time to display
    if (key14_pressed && key14_count > 0)
    {
        Digital_DisplayDEC(arm_hours * 100 + arm_minutes); 
    }
    else
    {
        Digital_DisplayDEC(hours * 100 + minutes);         
    }

    // Apply blinking effect if a setting mode is active and display is in "off" state
    if (!segment_display_on)
    {
        uint8_t mode = (key1_pressed) ? key1_count : key14_count;
        if (mode == 1) // Blinking hours
        {
            segment_buff[0] = 0;
            segment_buff[1] = 0;
        }
        else if (mode == 2) // Blinking minutes
        {
            segment_buff[2] = 0;
            segment_buff[3] = 0;
        }
    }
}

/**
 * @brief Manages blinking of the mode indicator LED.
 */
static void handle_led_blinking(void)
{
    if (blink_led0_active)
    {
        ms_count[0]++;
        if (ms_count[0] >= 500) // 500ms interval
        {
            ms_count[0] = 0;
            led_state[0] = !led_state[0];
            if (led_state[0]) SET_LED0_ON;
            else SET_LED0_OFF;
        }
    }
}

/**
 * @brief Updates the main clock time every second.
 */
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

/**
 * @brief Handles timeout to exit setting modes after a period of inactivity.
 */
static void handle_timeout(void)
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

/**
 * @brief Checks if the current time matches the alarm time to trigger the alarm.
 */
void check_and_activate_alarm(uint8_t h, uint8_t m, uint8_t s)
{
    if (alarm_enable_flag && h == arm_hours && m == arm_minutes && s == arm_seconds && !alarm_active)
    {
        alarm_active = 1;
        alarm_tick_ms = 0;
        alarm_blink_counter = 0;
        alarm_buzzer_state = 0;
    }
}

/*---------------------------INIT/MAIN TASK-------------------------------------*/

/**
 * @brief Initializes the main task, loading data from flash.
 */
void main_task_init(void) 
{ 
    init_alarm_time_if_needed();     
    load_alarm_time_from_flash();
    alarm_active = 0;
}

/**
 * @brief The main execution loop for the clock application.
 */
void main_task_run(void)
{
    clock_update();
    check_and_activate_alarm(hours, minutes, seconds);

    if (timer_1ms_flag)
    {
        timer_1ms_flag = 0;

        segment_blink(500);
        update_display_logic();
        Digital_Scan();
        
        buzzer_update();
        handle_timeout();
        handle_led_blinking();
        handle_alarm_buzzer();
    }

    key_code = KeyScan();
    if(key_code) 
    {
        read_key = key_code;
        key_SW();
    }
}

/*
 * Project      : MCU V2 - Digital Clock Controller
 * Author       : Tran Nam
 * Date         : 22/07/2025
 * Email        : trannam6362@gmail.com
 * Version      : 2.2 9:08
 *
 * Description  : 
 * This source file handles specific key press events for the main task.
 */

/******************************************************/
#include "..\Driver\GPIO.h"
#include "..\Module\Buzzer.h"
#include "..\main_task\main_task.h"
#include "..\main_task\hand_task.h"
/******************************************************/

/**
 * @brief Handles the logic for pressing KEY_1 (Enter/Cycle through Time Setting Mode).
 */
void handle_key1_press(void)
{
    if (!key14_pressed) 
    {
        key1_pressed = 1;
        key1_count++;
        if (key1_count > 2)
        {
            key1_count = 0;
            key1_pressed = 0; 
            blink_led0_active = 0; 
            SET_LED0_OFF;
            led_state[0] = 0;
        }
        else
        {
            blink_led0_active = 1;
        }
        buzzer_bip(200);
        segment_blink_timer = 0;
        segment_display_on = 1;
    }
}

/**
 * @brief Handles the logic for pressing KEY_4 (Increment value - Up).
 */
void handle_key4_press(void)
{
    uint8_t mode = (key1_pressed) ? key1_count : key14_count;
    if (key1_pressed) 
    {    
        if (mode == 1) hours = (hours + 1) % 24;
        else if (mode == 2) minutes = (minutes + 1) % 60;
    }
    else if (key14_pressed) 
    {
        if (mode == 1) arm_hours = (arm_hours + 1) % 24;
        else if (mode == 2) arm_minutes = (arm_minutes + 1) % 60;
        alarm_time_changed = 1;
    }
    buzzer_bip(200);
}

/**
 * @brief Handles the logic for pressing KEY_8 (Decrement value - Down).
 */
void handle_key8_press(void)
{
    uint8_t mode = (key1_pressed) ? key1_count : key14_count;
    if (key1_pressed) 
    {
        if (mode == 1) hours = (hours == 0) ? 23 : hours - 1;
        else if (mode == 2) minutes = (minutes == 0) ? 59 : minutes - 1;
    }
    else if (key14_pressed) 
    {
        if (mode == 1) arm_hours = (arm_hours == 0) ? 23 : arm_hours - 1;
        else if (mode == 2) arm_minutes = (arm_minutes == 0) ? 59 : arm_minutes - 1;
        alarm_time_changed = 1;
    }
    buzzer_bip(200);
}

/**
 * @brief Handles the logic for pressing KEY_14 (Enter/Cycle through Alarm Setting Mode).
 */
void handle_key14_press(void)
{
    if (!key1_pressed)
    {
        key14_pressed = 1;
        key14_count++;
        if (key14_count > 2)
        {
            if (alarm_time_changed)
            {
                // This function is defined in main_task.c and declared in main_task.h
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
        segment_blink_timer = 0;
        segment_display_on = 1;
    }
}

/*****************************************************************************
* File        : main_task.h
* Description : Header file for main task control in application logic.
* Includes state definitions and initialization interfaces.
* Author      : namne1204
* Date        : 2025/07/22 9:07
* GitHub      : https://github.com/namne1204
* Revision    : 2.3 - Added prototype for shared function.
*****************************************************************************/

#ifndef __MAIN_TASK_H
#define __MAIN_TASK_H

#include <stdint.h>

typedef enum
{
    NORMAL_USER,
    IN_SET,
    ARM_CLOCK
} StateUser;

// ================== SHARED VARIABLES (EXTERN) ==================
// Khai báo các biến trạng thái dùng chung để các module khác có thể truy cập
extern uint8_t   hours;
extern uint8_t   minutes;
extern uint16_t  seconds;
extern uint8_t   arm_hours;
extern uint8_t   arm_minutes;
extern uint8_t   arm_seconds;
extern uint16_t  read_key;
extern uint8_t   key1_count;
extern uint8_t   key14_count;
extern uint16_t  key_timeout_counter;
extern uint8_t   key1_pressed;
extern uint8_t   key14_pressed;
extern uint8_t   blink_led0_active;
extern uint8_t   led_state[2];
extern uint16_t  segment_blink_timer;
extern uint8_t   segment_display_on;
extern uint8_t   alarm_time_changed;
extern uint8_t   alarm_enable_flag;
extern uint8_t   timer_1s_flag;
extern uint8_t   timer_1ms_flag;

// ================== FUNCTION PROTOTYPES ==================
void main_task_init(void);
void main_task_run(void);

// Shared function prototype for hand_task.c
void save_alarm_time_to_flash(void);

#endif

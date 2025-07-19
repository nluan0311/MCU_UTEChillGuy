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
///****************************MACRO***************************************/
//static uint16_t read_key;
//static uint8_t key_val;
//static uint16_t led0_timer = 0;
//static uint16_t led1_timer = 0;
//static uint16_t timeoutCounter = 30;
//static uint16_t timeoutbuzzer = 30;

static uint32_t  led_timer     = 0;    // couter LED
static uint32_t  beep_timer    = 0;    // couter beep
static uint8_t   beep_active   = 0;
static uint8_t   led_active    = 0;
static uint32_t  led_duration_ms = 3000; //
static uint8_t   up = 0;
static uint8_t   down = 0;
/********************************Clock****************************************/
static uint8_t   hours   = 23;
static uint8_t   minutes = 59;
static uint8_t   seconds = 55;

uint16_t read_key;
uint16_t key_code;
uint8_t  key_val;

static uint8_t  key1_press_count = 0;
static uint8_t  key14_press_count = 0;

static uint8_t key1_pressed = 0;

typedef enum {
    KEY1_MODE_A = 0,
    KEY1_MODE_B,
    KEY1_MODE_C
} key1_mode_t;
static key1_mode_t key1_mode = KEY1_MODE_A;

// Định nghĩa 3 chế độ cho KEY_14
typedef enum {
    KEY14_MODE_A = 0,
    KEY14_MODE_B,
    KEY14_MODE_C
} key14_mode_t;
///*_____ D E F I N I T I O N S ______________________________________________*/
//volatile uint8_t key_tick = 0;
//volatile uint8_t buzzer_tick = 0;
//volatile uint8_t seg_tick = 0;
//static StateUser currentState = NORMAL_USER;

#if 0
static uint16_t timeoutCounter = 0;
uint16_t key_code;
//const uint8_t key_table[] = {KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_10,KEY_11,KEY_12,KEY_13,KEY_14,KEY_15,KEY_16};	
//const uint8_t key_table[] = {KEY_1,KEY_4,KEY_8,KEY_14};
uint8_t last_buzzer_index = 255;
uint8_t last_key_index = 255;   
uint8_t buzzer_hold_counter = 0;
#define NUM_KEYS   (sizeof(key_table)/sizeof(key_table[0]))
#endif 
/*_____ D E C L A R A T I O N S ____________________________________________*/
//static void save_timer_flash(void);
//static void use_timer_flash(void);
static void normal_clock();
static void scan_moude();
static void clock_settings ();
static void clock_timer();
static void clock_update();
static void key_SW(void)
{
   
    if (read_key & KEY_PUSH_FLAG)
{
   uint8_t key_val = read_key & 0xFF;

   switch (key_val)
   {
       case KEY_1:
           key1_pressed = 1;
           SET_LED0_ON;
           break;
       case KEY_2:
            
           SET_LED0_OFF;
           key1_pressed = 0;
           break;

       case KEY_4:
           SET_LED1_ON;
				    	set_buzzer_pitch(5);
           UT_DelayNx10us(5000);
							set_buzzer_pitch(255);
           SET_LED1_OFF;
           break;

       default:
           break;
   }
}
}
/*-----------------------------------FUNSION----------------------------*/
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
    Digital_DisplayDEC(hours * 100 + minutes);
}
static void scan_moude()
{
if(timer_1ms_flag)
{
    timer_1ms_flag = 0;
    Digital_Scan();
    key_code = KeyScan();
   // key_val  = read_key & 0xFF;
	read_key = key_code;
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
// Hàm con xử lý sự kiện nút nhấn
#if 0
static void HandleKeyEvent(uint16_t keyEvt, uint32_t T_sec)
{
    if (keyEvt & KEY_PUSH_FLAG)
    {
        // 1) bật beep, reset bộ đếm
        beep_active     = true;
        beep_timer      = 0;
        BeepOn();

        // 2) bật LED sequence, reset bộ đếm LED
        led_active        = true;
        led_timer         = 0;
        led_duration_ms   = T_sec * 1000U;
    }
}

// Hàm con cập nhật trạng thái còi (beep)
static void UpdateBeep(void)
{
    if (!beep_active) return;

    if (++beep_timer >= 300U)  // giữ còi 300 ms
    {
        BeepOff();
        beep_active = false;
    }
}

// Hàm con cập nhật trạng thái LED
static void UpdateLed(void)
{
    if (!led_active) return;

    if (++led_timer >= led_duration_ms)
    {
        // kết thúc sequence
        LED_Off();
        led_active = false;
        return;
    }

    // blink mỗi 500 ms: 250 ms on, 250 ms off
    if ((led_timer % 500U) < 250U)
        LED_On();
    else
        LED_Off();
}
#endif
#if 0
#endif
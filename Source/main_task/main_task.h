/*****************************************************************************
* File        : main_task.h
* Description : Header file for main task control in application logic.
*               Includes state definitions and initialization interfaces.
* Author      : namne1204
* Date        : 2025/07/17
* GitHub      : https://github.com/namne1204
* Revision    : 1.0 - Initial version
*****************************************************************************/

#ifndef __MAIN_TASK_H
#define __MAIN_TASK_H

typedef enum 
{
    NORMAL_USER,
    IN_SET,
    ARM_CLOCK
} StateUser;

void main_task_init(void);
void main_task_run(); 
#endif

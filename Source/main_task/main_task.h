#ifndef __MAIN_TASK_H
#define __MAIN_TASK_H

typedef enum {
    NORMAL_USER,
    IN_SET,
} StateUser;

void main_task_init(void);
void main_task_run(); 
#endif

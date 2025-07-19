#ifndef __BUZZER_H
#define __BUZZER_H

/*_____ I N C L U D E S ____________________________________________________*/
#include <SN32F400.h>

/*_____ D E F I N I T I O N S ______________________________________________*/


/*_____ M A C R O S ________________________________________________________*/

/*_____ D E C L A R A T I O N S ____________________________________________*/
void set_buzzer_pitch(uint8_t pitch);

void buzzer_bip(uint16_t time_delay_ms);

void buzzer_stop(void);
void buzzer_update(void);
#endif	/*__BUZZER_H*/

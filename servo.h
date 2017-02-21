#ifndef SERVO_H
#define SERVO_H

#include <stdbool.h>
#include <stdint.h>

/*-----Period, in TACLK cycles-----*/
#define PWM_PERIOD (CLOCK_SPEED / (PWM_FREQUENCY * TIMER_A_DIVIDER))

#define DEFAULT_BASEBAND_TIME_US 		(1000ul)
#define DEFAULT_MAXBAND_TIME_US 		(2000ul)

#define DEFAULT_BASEBAND_CLK_TIME 		(((CLOCK_SPEED/1000000ul) * DEFAULT_BASEBAND_TIME_US) / \
                                         (TIMER_A_DIVIDER))
#define DEFAULT_MAXBAND_CLK_TIME 		(((CLOCK_SPEED/1000000ul) * DEFAULT_MAXBAND_TIME_US) / \
                                 (TIMER_A_DIVIDER))

#define DEFAULT_MAXBAND_CLK_TIME_DIFF (DEFAULT_MAXBAND_CLK_TIME - DEFAULT_BASEBAND_CLK_TIME)
#define NUM_SERVOS (2)

typedef struct
{
    uint16_t pos[NUM_SERVOS];
    uint16_t baseband, maxband;
} servo_ctl_t;

void servo_init(servo_ctl_t* control, bool(*ctl_ready)());

#endif // SERVO_H

/*
 * software_timer.h
 *
 *  Created on: Sep 24, 2023
 *      Author: HaHuyen
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include "tim.h"
#define TIMER_NUM 5

extern uint16_t flag_timer[TIMER_NUM]; // 0: blinky value; 1: system timer; 2: 200ms changes; 3: 10s for send request

void timer_init();
void setTimer(uint16_t duration, int index);

#endif /* INC_SOFTWARE_TIMER_H_ */

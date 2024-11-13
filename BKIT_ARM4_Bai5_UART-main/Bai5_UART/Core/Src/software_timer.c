/*
 * software_timer.c
 *
 *  Created on: Sep 24, 2023
 *      Author: HaHuyen
 */

#include "software_timer.h"

#define TIMER_CYCLE 1

uint16_t flag_timer[TIMER_NUM] = {0,0,0,0,0};
uint16_t timer_counter[TIMER_NUM] = {0,0,0,0,0};
uint16_t timer_MUL[TIMER_NUM] = {0,0,0,0,0};

void timer_init(){
	HAL_TIM_Base_Start_IT(&htim2);
}

void setTimer(uint16_t duration, int index){
	timer_MUL[index] = duration/TIMER_CYCLE;
	timer_counter[index] = timer_MUL[index];
	flag_timer[index] = 0;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM2){
		for (int i = 0; i < TIMER_NUM; i++) {
			if(timer_counter[i] > 0){
				timer_counter[i]--;
				if(timer_counter[i] == 0) {
					flag_timer[i] = 1;
					timer_counter[i] = timer_MUL[i];
				}
			}
		}
		led7_Scan();
	}
}


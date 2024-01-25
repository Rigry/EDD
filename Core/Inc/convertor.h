#pragma once

#include "adc.h"
#include "service.h"
#include "interrupt.h"
#include "pin.h"

class Convertor {

	ADC_& adc;
	Interrupt& period_callback;
	Interrupt& adc_comparator_callback;
	Interrupt& ext_holla_1_callback;

	Pin &led_red;
	Pin& en_holla;
	Pin& error_holla;
	Pin& phase_a_low;
	Pin& phase_b_low;
	Pin& phase_c_low;

	uint8_t hallpos{1};
	int16_t step{0};
	int16_t max_steps{0};

	uint16_t duty_cycle{0};

	bool enable{false};
	bool reverse{false};

	using Parent = Convertor;

	struct TIM3_interrupt: Interrupting {
		Parent &parent;
		TIM3_interrupt(Parent &parent) :
				parent(parent) {
			parent.period_callback.subscribe(this);
		}
		void interrupt() override {
			parent.period_interrupt();
		}
	} tim3_interrupt { *this };

	struct adc_comparator_interrupt: Interrupting {
		Parent &parent;
		adc_comparator_interrupt(Parent &parent) :
				parent(parent) {
			parent.adc_comparator_callback.subscribe(this);
		}
		void interrupt() override {
			parent.comparator_interrupt();
		}
	} adc_comparator_ { *this };

	struct Holla_1_interrupt: Interrupting {
		Parent &parent;
		Holla_1_interrupt(Parent &parent) :
				parent(parent) {
			parent.ext_holla_1_callback.subscribe(this);
		}
		void interrupt() override {
			parent.holla_1_callback();
		}
	} holla_1_interrupt_ { *this };

	void period_interrupt(){

		HAL_ADCEx_InjectedStart_IT(&hadc2);
	}

	void comparator_interrupt() {

	}

	void holla_1_callback(){
		hallpos = ((HAL_GPIO_ReadPin(GPIOC, holla_1_Pin) << 2) | (HAL_GPIO_ReadPin(GPIOB, holla_2_Pin) << 1) | HAL_GPIO_ReadPin(GPIOB, holla_3_Pin));

		switch (hallpos) {
		case 6:
			TIM1->CCR1 = 0;
			if (not reverse) {
				TIM1->CCR2 = duty_cycle;
				phase_c_low = true;
			} else {
				phase_b_low = true;
				TIM1->CCR3 = duty_cycle;
			}
			break;
		case 2:
			phase_c_low = false;
			if (not reverse) {
				phase_a_low = true;
				TIM1->CCR2 = duty_cycle;
			} else {
				TIM1->CCR1 = duty_cycle;
				phase_b_low = true;
			}
			break;
		case 3:
			TIM1->CCR2 = 0;
			if (not reverse) {
				phase_a_low = true;
				TIM1->CCR3 = duty_cycle;
			} else {
				TIM1->CCR1 = duty_cycle;
				phase_c_low = true;
			}
			break;
		case 1:
			phase_a_low = false;
			if (not reverse) {
				phase_b_low = true;
				TIM1->CCR3 = duty_cycle;
			} else {
				TIM1->CCR2 = duty_cycle;
				phase_c_low = true;
			}
			break;
		case 5:
			TIM1->CCR3 = 0;
			if (not reverse) {
				TIM1->CCR1 = duty_cycle;
				phase_b_low = true;
			} else {
				phase_a_low = true;
				TIM1->CCR2 = duty_cycle;

			}
			break;
		case 4:
			phase_b_low = false;
			if (not reverse) {
				TIM1->CCR1 = duty_cycle;
				phase_c_low = true;
			} else {
				phase_a_low = true;
				TIM1->CCR3 = duty_cycle;
			}
			break;
		} // end of phase switch

		if(reverse)
			step--;
		else
			step++;

		if(enable) {
			timer.stop();
			timer.start(1000);
		}
	}

public:

	Convertor(ADC_& adc, Interrupt& period_callback, Interrupt& adc_comparator_callback
			,  Interrupt& ext_holla_1_callback
			, Pin& led_red
			, Pin& en_holla, Pin& error_holla
			, Pin& phase_a_low, Pin& phase_b_low, Pin& phase_c_low)
	: adc{adc}, period_callback{period_callback}, adc_comparator_callback{adc_comparator_callback}
	, ext_holla_1_callback{ext_holla_1_callback}
	, led_red{led_red}
	, en_holla{en_holla}, error_holla{error_holla}
	, phase_a_low{phase_a_low}, phase_b_low{phase_b_low}, phase_c_low{phase_c_low}
	{
		hallpos = ((HAL_GPIO_ReadPin(GPIOC, holla_1_Pin) << 2) | (HAL_GPIO_ReadPin(GPIOB, holla_2_Pin) << 1) | HAL_GPIO_ReadPin(GPIOB, holla_3_Pin));
		en_holla = true;
		stop();
	}

	Timer timer;

	void forward() {
//		en_holla = true;
		pusk();
		holla_1_callback();
		reverse = false;

	}

	void forward_step(){

	}

	void back() {
//		en_holla = true;
		pusk();
		holla_1_callback();
		reverse = true;
	}

	void back_step(){

	}

	void power(uint16_t percent) {
		duty_cycle = 7200/100 * percent - 1;
	}

	int16_t steps(){
		return step;
	}

	void reset_steps(){
		step = 0;
	}

	void set_max_steps(int16_t s) {
		max_steps = s;
	}

	void fix(){
		phase_a_low = true;
		phase_b_low = true;
		phase_c_low = true;
	}

	bool check_holla(){
		return bool(error_holla);
	}


	void pusk() {

		phase_a_low = false;
		phase_b_low = false;
		phase_c_low = false;

		TIM3->ARR = 99;

		HAL_TIM_Base_Start_IT(&htim3);

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

		adc.measure_value();

		timer.start(1000);

		enable = true;

//		service.outData.error.current = false;

	}

	void stop() {

		TIM1->CCR1 = 0;
		TIM1->CCR2 = 0;
		TIM1->CCR3 = 0;
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		phase_a_low = false;
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		phase_b_low = false;
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
		phase_c_low = false;

		HAL_TIM_Base_Stop_IT(&htim3);

//		en_holla = false;

		timer.stop();

		adc.measure_offset();

		enable = false;

	}

	void alarm() {

	}

};

Interrupt period_callback;
Interrupt adc_comparator_callback;
Interrupt ext_holla_1_callback;

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim){
	if(htim->Instance == TIM3) //check if the interrupt comes from ACD2
	{
		period_callback.interrupt();
	}
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc){
	if(hadc->Instance == ADC2) //check if the interrupt comes from ACD2
	{
		adc_comparator_callback.interrupt();
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t Holla){
	if(Holla == holla_1_Pin or Holla == holla_2_Pin or Holla == holla_3_Pin) {
		ext_holla_1_callback.interrupt();
	}
}

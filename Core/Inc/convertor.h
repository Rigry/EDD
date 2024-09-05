#pragma once

#include "adc.h"
#include "service.h"
#include "interrupt.h"
#include "pin.h"

class Convertor {

	ADC_& adc;
	Service& service;
	Interrupt& period_callback;
	Interrupt& adc_comparator_callback;
	Interrupt& ext_holla_1_callback;

	Pin &led_red;
	Pin& en_holla;
	Pin& error_holla;
//	Pin& phase_a_low;
//	Pin& phase_b_low;
//	Pin& phase_c_low;

	uint8_t hallpos{1};
	int32_t step{0};
	int16_t max_steps{0};

	uint16_t duty_cycle{0};
	uint16_t km{0};
	uint16_t new_km{0};

	uint16_t new_ARR{0};

	uint16_t sin_table[qty_point]{3600, 4627, 5514, 6234, 6764, 7089
								, 7199, 7089, 6764, 6234, 6764, 7089
								, 7199, 7089, 6764, 6234, 5514, 4627
								, 3600, 2462, 1250,    0,    0,    0
								,    0,    0,    0,    0,    0,    0
								,    0,    0,    0,    0, 1250, 2462
								};

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

		km += (new_km - km) * 10 / 40;

		TIM1->CCR1 = Km * sin_table[m++] / 1000;
		TIM1->CCR2 = Km * sin_table[k++] / 1000;
		TIM1->CCR3 = Km * sin_table[n++] / 1000;

		if (k >= qty_point) {k = 0;}
		if (m >= qty_point) {m = 0;}
		if (n >= qty_point) {n = 0;}
		HAL_ADCEx_InjectedStart_IT(&hadc2);
	}

//	void comparator_interrupt() {
//
//	}

//	void holla_1_callback(){
//		hallpos = ((HAL_GPIO_ReadPin(GPIOC, holla_1_Pin) << 2) | (HAL_GPIO_ReadPin(GPIOB, holla_2_Pin) << 1) | HAL_GPIO_ReadPin(GPIOB, holla_3_Pin));
//
//		switch (hallpos) {
//		case 6:
//			TIM1->CCR1 = 0;
//			phase_a_low = false;
//			if (not reverse) {
//				TIM1->CCR3 = 0;
//				phase_b_low = false;
//				phase_c_low = true;
//				TIM1->CCR2 = duty_cycle;
//			} else {
//				phase_b_low = true;
//				TIM1->CCR3 = duty_cycle;
//			}
//			break;
//		case 2:
//			TIM1->CCR3 = 0;
//			phase_c_low = false;
//			if (not reverse) {
//				phase_a_low = true;
//				phase_b_low = false;
//				TIM1->CCR2 = duty_cycle;
//				TIM1->CCR1 = 0;
//			} else {
//				TIM1->CCR1 = duty_cycle;
//				phase_b_low = true;
//			}
//			break;
//		case 3:
//			TIM1->CCR2 = 0;
//			phase_b_low = false;
//			if (not reverse) {
//				phase_a_low = true;
//				phase_c_low = false;
//				TIM1->CCR3 = duty_cycle;
//				TIM1->CCR1 = 0;
//			} else {
//				TIM1->CCR1 = duty_cycle;
//				phase_c_low = true;
//			}
//			break;
//		case 1:
//			TIM1->CCR1 = 0;
//			phase_a_low = false;
//			if (not reverse) {
//				phase_b_low = true;
//				phase_c_low = false;
//				TIM1->CCR2 = 0;
//				TIM1->CCR3 = duty_cycle;
//			} else {
//				TIM1->CCR2 = duty_cycle;
//				phase_c_low = true;
//			}
//			break;
//		case 5:
//			TIM1->CCR3 = 0;
//			phase_c_low = false;
//			if (not reverse) {
//				TIM1->CCR2 = 0;
//				phase_a_low = false;
//				phase_b_low = true;
//				TIM1->CCR1 = duty_cycle;
//				TIM1->CCR2 = 0;
//				phase_a_low = false;
//				phase_b_low = true;
//			} else {
//				phase_a_low = true;
//				TIM1->CCR2 = duty_cycle;
//
//			}
//			break;
//		case 4:
//			TIM1->CCR3 = 0;
//			phase_b_low = false;
//			if (not reverse) {
//				TIM1->CCR2 = 0;
//				phase_a_low = false;
//				phase_c_low = true;
//				TIM1->CCR1 = duty_cycle;
//			} else {
//				phase_a_low = true;
//				TIM1->CCR3 = duty_cycle;
//			}
//			break;
//		} // end of phase switch
//
//		if(reverse)
//			step--;
//		else
//			step++;

//		if(enable) {
//			timer.stop();
//			timer.start(1000);
//		}
//	}

public:

	Convertor(ADC_& adc, Service& service, Interrupt& period_callback, Interrupt& adc_comparator_callback
			,  Interrupt& ext_holla_1_callback
			, Pin& led_red
			, Pin& en_holla, Pin& error_holla
//			, Pin& phase_a_low, Pin& phase_b_low, Pin& phase_c_low
			)
	: adc{adc}, service{service}, period_callback{period_callback}, adc_comparator_callback{adc_comparator_callback}
	, ext_holla_1_callback{ext_holla_1_callback}
	, led_red{led_red}
	, en_holla{en_holla}, error_holla{error_holla}
//	, phase_a_low{phase_a_low}, phase_b_low{phase_b_low}, phase_c_low{phase_c_low}
	{
		hallpos = ((HAL_GPIO_ReadPin(GPIOC, holla_1_Pin) << 2) | (HAL_GPIO_ReadPin(GPIOB, holla_2_Pin) << 1) | HAL_GPIO_ReadPin(GPIOB, holla_3_Pin));
		en_holla = true;
		stop();
	}

	Timer timer;

	void forward() {
		new_km = service.outData.voltage_logic / 4095 * 1000;

		new_ARR = service.outData.voltage_drive * 10;

		TIM3->ARR += (new_ARR - TIM3->ARR) * 10 / 40;
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
		if (duty_cycle > 7199) duty_cycle = 7199;
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

		TIM1->CCR1 = 0;
		TIM1->CCR2 = 0;
		TIM1->CCR3 = 0;

		TIM3->ARR = 99;

		Km = 5;

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

		HAL_TIM_Base_Start_IT (&htim3);

		adc.measure_value();

//		service.outData.error.current = false;

	}

	void stop() {

		TIM1->CCR1 = 0;
		TIM1->CCR2 = 0;
		TIM1->CCR3 = 0;
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

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

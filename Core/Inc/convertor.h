#pragma once

#include "adc.h"
#include "service.h"
#include "interrupt.h"
#include "pin.h"

class Convertor {

	enum State {wait, starting} state{wait};

	ADC_& adc;
	Service<In_data, Out_data>& service;
	Interrupt& period_callback;
	Interrupt& adc_comparator_callback;
	Pin& led_red;
	Pin& led_green;
	Pin& open_in;
	Pin& close_in;
	Pin& open_out;
	Pin& close_out;
	Pin& open_fb;
	Pin& close_fb;
	Pin& end;
	Pin& en_holla;
	Pin& error_holla;

	Timer timer;

	bool enable{true};

//	float radian = 10 * 3.14 / 180;
	uint32_t div_f = 1'800'000 / (qty_point);

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

	void period_interrupt(){

		TIM1->CCR1 = 0;
		TIM1->CCR2 = 0;
		TIM1->CCR3 = 0;

		HAL_ADCEx_InjectedStart_IT(&hadc2);
	}

	void comparator_interrupt() {

	}

public:

	Convertor(ADC_& adc, Service<In_data, Out_data>& service,Interrupt& period_callback, Interrupt& adc_comparator_callback
			, Pin& led_red, Pin& led_green, Pin& open_in, Pin& close_in, Pin& open_out
			, Pin& close_out, Pin& open_fb, Pin& close_fb, Pin& end, Pin& en_holla, Pin& error_holla)
	: adc{adc}, service{service}, period_callback{period_callback}, adc_comparator_callback{adc_comparator_callback}
	, led_red{led_red}, led_green{led_green}, open_in{open_in}, close_in{close_in}, open_out{open_out}
	, close_out{close_out}, open_fb{open_fb}, close_fb{close_fb}, end{end}, en_holla{en_holla}, error_holla{error_holla}
	{}

	void operator() (){

		service();

		switch(state) {
		case wait:

			break;
		case starting:
			break;
		}
	}

	void pusk() {


		TIM3->ARR = 0;

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_3);

		HAL_TIM_Base_Start_IT(&htim3);

		timer.start(4);
		adc.measure_value();

		service.outData.error.current = false;

		led_red = false;
	}

	void stop() {

		TIM1->CCR1 = 0;
		TIM1->CCR1 = 0;
		TIM1->CCR1 = 0;
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_3);
		HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_3);

		HAL_TIM_Base_Stop_IT(&htim3);
		timer.stop();

		state = State::wait;

		adc.measure_offset();

	}

	void alarm() {

	}

};

Interrupt period_callback;
Interrupt adc_comparator_callback;

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


#pragma once


#include "interrupt.h"
#include <cmath>

const uint16_t qty_point{18};

enum {CURRENT};
enum {VB = 0, VL = 1, VD = 2};

class ADC_ : TickSubscriber
{
	Interrupt& adc_callback;
	Interrupt& adc_injected_callback;

	uint8_t qty_channel{0};
	uint16_t time_refresh{1000};

	uint16_t time{0};
	uint16_t buffer[3];

	int16_t arr_current[9]{0};
	uint8_t j{0};

	bool work{false};
	bool measure{false};
	bool error{false};
	bool over_cur{false};

	int16_t arr[9]{0};

	uint16_t max_current{16};
	uint8_t over_current{0};

	void adc_interrupt() {
		HAL_ADC_Stop_DMA (&hadc1);
	}

	void adc_injected_interrupt() {

		HAL_ADCEx_InjectedStop_IT (&hadc2);
		arr_current[j] = HAL_ADCEx_InjectedGetValue(&hadc2, CURRENT);

		if(not work) {
			offset_I = 0;
			for (auto i = 0; i < 9; i++) {
				offset_I += arr_current[i];
			}
			offset_I /= (9);

			over_current = 0;

		} else if (work) {

			arr[j] = abs(arr_current[j] - offset_I);

			if (arr[j] / 21 >= max_current) {
				over_current++;
				if (over_current >= 4)
					over_cur = true;
			}
		}

		if (j < 8) j++;
		else {
			j = 0;
			over_current = 0;
		}
	}

	using Parent = ADC_;

	struct ADC_interrupt : Interrupting {
		Parent &parent;
		ADC_interrupt(Parent &parent) :
				parent(parent) {
			parent.adc_callback.subscribe(this);
		}
		void interrupt() override {
			parent.adc_interrupt();
		}
	} adc_ { *this };

	struct ADC_INJ_interrupt : Interrupting {
		Parent &parent;
		ADC_INJ_interrupt(Parent &parent) :
				parent(parent) {
			parent.adc_injected_callback.subscribe(this);
		}
		void interrupt() override {
			parent.adc_injected_interrupt();
		}
	} adc_injected_ { *this };

public:

	ADC_(Interrupt& adc_callback, Interrupt& adc_injected_callback, uint8_t qty_channel, uint16_t time_refresh)
    : adc_callback {adc_callback}
    , adc_injected_callback {adc_injected_callback}
    , qty_channel  {qty_channel}
    , time_refresh {time_refresh}
	{
		subscribed = false;
		if (time_refresh > 0)
		  subscribe();
	}

	int16_t offset_I{0};

	void measure_offset() {
		work = false;
	}

	void measure_value() {
		work = true;
	}

	uint16_t operator[](uint8_t i) {
		return buffer[i];
	}

	bool is_error(){return error;}
	void reset_error(){error = false;}
	bool is_over_s(){return over_cur;}
	void reset_over_s(){over_cur = false;}
//	void what_Km(uint16_t k) {Km_check = k > 50 ? true : false;}

	void notify(){
		if (time++ >= time_refresh) {
		   time = 0;
		   HAL_ADC_Start_DMA(&hadc1, (uint32_t*)buffer, qty_channel);
		}
		if( not time % 100 and not work)
			HAL_ADCEx_InjectedStart_IT(&hadc2);
	}


};

Interrupt adc_callback;
Interrupt adc_injected_callback;

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef * hadc){
	if(hadc->Instance == ADC1) //check if the interrupt comes from ACD1
	{
		adc_callback.interrupt();
	}
}

void HAL_ADCEx_InjectedConvCpltCallback(ADC_HandleTypeDef* hadc){
	if(hadc->Instance == ADC2) //check if the interrupt comes from ACD2
	{
		adc_injected_callback.interrupt();
	}
}


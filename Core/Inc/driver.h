#pragma once

#include "pin.h"
#include "service.h"
#include "convertor.h"

class Driver {

	enum State {wait, opening, closing, alarm} state{wait};

	Service<In_data, Out_data> &service;
	Convertor& convertor;
	Pin &led_red;
	Pin &led_green;
	Pin &open_in;
	Pin &close_in;
	Pin &open_out;
	Pin &close_out;
	Pin &open_fb;
	Pin &close_fb;
	Pin &end;
	bool begin{false};

	uint16_t prev_step{0};

	bool enable{false};
	bool clamp{false};

	Timer delay;

public:

	Driver( Service<In_data, Out_data>& service, Convertor& convertor
		  , Pin& led_red, Pin& led_green, Pin& open_in, Pin& close_in
		  , Pin& open_out, Pin& close_out, Pin& open_fb, Pin& close_fb, Pin& end)
		  : service{service}, convertor{convertor}
	      , led_red{led_red}, led_green{led_green}, open_in{open_in}, close_in{close_in}
	      , open_out{open_out}, close_out{close_out}, open_fb{open_fb}, close_fb{close_fb}, end{end}
	{
		convertor.pusk();
	}

	void operator() () {

		service();
		service.outData.error.open     = open_out  = bool (not end);
		service.outData.error.close    = close_out = bool (end);
		service.outData.error.open_in  = open_in;
		service.outData.error.close_in = close_in;

		if(service.outData.error.current
		or service.outData.error.voltage_board_low
		or service.outData.error.voltage_drive_low
		or service.outData.error.voltage_logic_low
		/*or not convertor.check_holla()*/) {
//			enable = false;
//			state = wait;
//			convertor.stop();
		} else {
			enable = true;
		}

		led_red = not enable;
		led_green = not led_red;
//		convertor.forward();

//		if(close_in) {
			convertor.forward();
//	}
//			convertor.pusk();
//		} else if(not close_in) {
//			convertor.stop();
//		}
//		if(convertor.timer.timePassed() >= 100) {
//			clamp = true;
//			convertor.timer.stop();
//		}

//		if (convertor.timer.event()) {
//			clamp = bool (convertor.steps() == prev_step);
//			prev_step = convertor.steps();
//		}
//
//		if(clamp) {
//
//			if(state == closing) {
//				convertor.stop();
//				state = wait;
//				convertor.stop();
//				delay.start(100);
//				clamp = false;
//			} else if(state == opening) {
//				state = wait;
//			}
//		}
//
//		switch(state) {
//			case wait:
//				if(enable) {
//					if(open_in and not begin and not clamp) {
//						convertor.power(95); convertor.forward(); state = opening;
//					} else if(close_in and not end and not clamp) {
//						convertor.power(95); convertor.back(); state = closing;
//					} else if(not open_in and not close_in) {
//						clamp = false; begin = false; convertor.stop();
//					} else {
//						 convertor.stop();
//					}
//				}
//				if(end) convertor.reset_steps();
//			break;
//			case opening:
//
//				if(convertor.steps() >= 300) {
//					convertor.power(30);
//				}
//
//				if( (not open_in or convertor.steps() >= 370)) {
//					state = wait;
//					convertor.stop();
//					if(convertor.steps() >= 370) {
//						begin = true;
//						convertor.fix();
//					}
//
//				}
//
//			break;
//			case closing:
//				if (convertor.steps() <= 70) {
//					convertor.power(30);
//				}
//
//				if(not close_in or end) {
//					state = wait;
//					convertor.stop();
//					if(end) convertor.reset_steps();
//				}
//
//			break;
//			case alarm:
//				if (delay.done()) {
//					clamp = false;
//					delay.stop();
////					convertor.power(99); convertor.forward();
//				} else if (delay.isCount()) {
//					convertor.stop();
//				}
//			break;

//		} //switch(state)
	} //void operator()
};

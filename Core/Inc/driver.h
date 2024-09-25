#pragma once

#include "pin.h"
#include "service.h"
#include "convertor.h"
#include "can.h"

class Driver {

	enum State {wait, opening, closing, clamp_opening, clamp_closing, alarm} state{wait};

	CAN<In_id, Out_id>& can;
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
	bool clamp_open{false};
	bool clamp_close{false};
	bool fix{false};

	uint16_t time_clamp{150}; // was 120

	Timer going;
	Timer delay;
	uint16_t power{0};

public:

	Driver( CAN<In_id, Out_id>& can, Service<In_data, Out_data>& service, Convertor& convertor
		  , Pin& led_red, Pin& led_green, Pin& open_in, Pin& close_in
		  , Pin& open_out, Pin& close_out, Pin& open_fb, Pin& close_fb, Pin& end)
		  : can{can}, service{service}, convertor{convertor}
	      , led_red{led_red}, led_green{led_green}, open_in{open_in}, close_in{close_in}
	      , open_out{open_out}, close_out{close_out}, open_fb{open_fb}, close_fb{close_fb}, end{end}
	{

	}

	void operator() () {

		service();
		service.outData.error.open     = open_out  = bool (not end);
		service.outData.error.close    = close_out = bool (end);
		service.outData.error.open_in  = open_in;
		service.outData.error.close_in = close_in;

//		service.outData.voltage_logic = convertor.speed;
//		service.outData.voltage_drive = power;

		if(service.outData.error.current
		or service.outData.error.voltage_board_low
		or service.outData.error.voltage_drive_low
		or service.outData.error.voltage_logic_low
		/*or not convertor.check_holla()*/) {
			enable = false;
			state = wait;
			convertor.stop();
		} else {
			enable = true;
		}

		led_red = not enable;
//		led_green = false;

//		if(convertor.time_1 >= time_clamp or convertor.time_2 >= time_clamp  or convertor.time_3 >= time_clamp) {
//			if(abs(convertor.steps() < 250) and (state == opening or state == closing))
//				clamp = true;
//			if(state == clamp_opening) {
//				convertor.stop();
//				convertor.power(20);
//				state = wait;
//				clamp = true;
//			}
//		}

//		if(convertor.speed > 3) {
//			clamp_open = false;
//		}

		if( service.outData.current > 500 and convertor.is_work()
		or ( /*abs(convertor.steps()) > 30 and abs(convertor.steps()) < 180*/convertor.is_start()  and convertor.speed < 3 and not clamp_open and convertor.is_work())
		 ) {
			clamp = true;
//			convertor.equal_step();
//			convertor.stop();
		}



		if(clamp) {

//			going.stop();

			if(state == closing) {
				convertor.stop();
				state = alarm;
				clamp_close = true;
//				clamp = false;
				delay.start(300);
			} else if(state == opening) {
				convertor.stop();
				state = alarm;
				clamp_open = true;
//				clamp = false;
				delay.start(300);
			}

//			else if( (state == clamp_opening and clamp_close) or ( state == clamp_closing and clamp_open) ) {
//				convertor.stop();
//				state = wait;
//			}
		}

		can.outID.state.open = begin;
		can.outID.state.close = bool(end);
		can.outID.state.clamp = clamp;


		switch(state) {
			case wait:
				if(end) {convertor.reset_steps(); convertor.fix();}
				if((abs(convertor.steps()) >= (200) or fix)) {
					convertor.current_fix();
				} else {
					convertor.current_stop();
				}
				convertor.equal_step();
				if(enable) {
					if(( (open_in or can.inID.control.open) and not begin and not clamp and not clamp_open)/* or clamp_open or not clamp*/) {
						convertor.power(95); convertor.forward(); state = opening; fix = false;/*going.start(5);*/ // back для водителя forward для пассажира // 60 passenger 90 driver
					} else if( (close_in or can.inID.control.close) and not end and not clamp and not clamp_close) {
						fix = false;
						convertor.stop();
						power = 70; //was 50
						convertor.power(power); convertor.back(); state = closing; // // forward для водителя  back для пассажира
//						going.start(5);
					} else if ( (not open_in and not close_in) and (not can.inID.control.open and not can.inID.control.close) ){
						clamp = false;  clamp_open = false; clamp_close = false;
					} else if (clamp_open and clamp_close){
						 convertor.stop();
					}
				}

			break;
			case opening:



//				if (going.done()) {
//					going.stop();
//					going.start(5);
////					if(power++ >= 60) power = 60; // 95 passenger 70 driver
//					if (convertor.speed >= 15) {
//						power -= (convertor.speed - 15) * 10 / 30;
//					} else {
//						power += (15 - convertor.speed) * 10 / 30;
//					}
//					power = power >= 95 ? 95 : power;
//					convertor.power(power);
//				}
//				convertor.current_fix();

				if(abs(convertor.steps()) >= (120)) { // for passenger
					convertor.power(50); // 35 passenger 65 driver // was60
				}

//				if (abs(convertor.steps()) >= (160)) { // for passenger
//					convertor.power(10); // 35 passenger 65 driver // was60
//				}
//
				if( not open_in or abs(convertor.steps()) >= (215)) {
					state = wait;
//					clamp = false; clamp_open = false;
//					convertor.stop();
					convertor.current_fix();
//					convertor.power(20);
					if(abs(convertor.steps()) >= (210)) {
						begin = true;
					}

				}

			break;
			case closing:

//				if (going.done()) {
//					going.stop();
//					going.start(5);
////					if(power++ >= 60) power = 60; // 95 passenger 70 driver
//					if(convertor.speed >= 10) {
//						power -= (convertor.speed - 10)*10/30;
//					} else {
//						power += (10 - convertor.speed) * 10 / 30;
//					}
//					power = power >= 95 ? 95 : power;
//					convertor.power(power);
//				}

				if (abs(convertor.steps()) <= (150)) {
					convertor.power(70); // 95 passenger // 70 driver // 60
				}

				if(not close_in or end) {
					state = wait;
					convertor.stop();
					if(end) convertor.reset_steps();
					begin = false;
				}

			break;
			case clamp_opening:
				if( abs(convertor.steps()) >= (215) or service.outData.current > 150) {
									state = wait;
				//					clamp = false; clamp_open = false;
				//					convertor.stop();
									fix = true;
									convertor.current_fix();
				//					convertor.power(20);
									if(abs(convertor.steps()) >= (210)) {
										begin = true;
									}
				}
			break;
			case clamp_closing:
//				clamp = false;
//				if((open_in and not begin and not clamp and not clamp_open)/* or clamp_open or not clamp*/) {
//						convertor.power(95); convertor.forward(); state = opening; fix = false;/*going.start(5);*/ // back для водителя forward для пассажира // 60 passenger 90 driver
//				} else if(close_in and not end and not clamp and not clamp_close) {
//						fix = false;
//						convertor.stop();
//						power = 70; //was 50
//						convertor.power(power); convertor.back(); state = closing; // // forward для водителя  back для пассажира
//				//						going.start(5);
//				   if(close_in) {
//						clamp = false;  clamp_open = false; clamp_close = false; state = wait; convertor.reset_steps();
//				   }
//
//				   if (open_in and not clamp_open) {
//					   state = clamp_opening; convertor.reset_steps();
//					   convertor.power(50); convertor.forward();
//				   } else if (not open_in) {
//					   clamp = false;  clamp_open = false; clamp_close = false;
//				   }

				if(end) {
					state = wait;
					convertor.stop();
					convertor.reset_steps();
					begin = false;
					clamp_close = true;
				}
			break;
			case alarm:

				if (clamp_close and delay.done()) {
					delay.stop();
					convertor.power(50); convertor.forward(); state = clamp_opening;  clamp = false; // back for driver
				}

				if (clamp_open and delay.done()) {
					convertor.stop();
					delay.stop();
					convertor.power(50); convertor.back();
					state = wait;
					clamp = false; // forward for driver
//					clamp_open = false;
				}

			break;

		} //switch(state)
	} //void operator()
};

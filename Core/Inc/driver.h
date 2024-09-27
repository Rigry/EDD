#pragma once

#include "pin.h"
#include "service.h"
#include "convertor.h"
#include "can.h"

class Driver {

	enum State    {wait, opening, closing, clamp_opening, clamp_closing, alarm} state{wait};
	enum Door_of  {driver, single_pass, double_pass, not_door} door{not_door};

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
	bool init_door{false};

	uint16_t prev_step{0};

	bool enable{false};
	bool clamp{false};
	bool clamp_open{false};
	bool clamp_close{false};
	bool fix{false};

	bool first_time{true};

	uint16_t time_clamp{150}; // was 120
	uint8_t open_power{60};

	Timer init;
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
		init.start(2'000);
		open_out = true;
		close_out = true;
	}

	bool is_initial() {

		if (init.isCount()) {
			if(open_in and close_in and can.inID.initial == 0xFF) {
				door = driver;
				can.change_ID(0xAA);
			} else if (not open_in and not close_in and can.inID.initial == 0xFF) {
				door = single_pass;
			} else if (not open_in and not close_in and can.inID.initial != 0xFF) {
				door = double_pass;
			}
		}

		if(init.done()) {
			init.stop();
			open_out = false;
			close_out = false;
			init_door = door != not_door;
		}

		return init_door;

	}

	void operator() () {

		if(end) first_time = false;

		service();
//		service.outData.error.open     = open_out  = bool (not end);
//		service.outData.error.close    = close_out = bool (end);
//		service.outData.error.open_in  = open_in;
//		service.outData.error.close_in = close_in;

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
		if (is_initial()) {

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
//				convertor.stop();
//				state = alarm;
//				clamp_close = true;
				clamp = false;
//				delay.start(300);
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

		switch(door) {
		case driver:
			can.outID.state.open_driver = not bool(end);
			break;
		case single_pass:
			can.outID.state.open_passenger = not bool(end);
			break;
		case double_pass:
			can.outID.state.open_driver = can.outID.state.open_passenger = not bool(end);
			break;
		case not_door:
			can.outID.state.open_driver =
			can.outID.state.open_passenger = true;
			break;
		}

		switch(state) {
			case wait:
				if(end) {convertor.reset_steps(); convertor.fix();}
				if((abs(convertor.steps()) >= (185) or fix)) {
					convertor.current_fix();
				} else {
					convertor.current_stop();
				}
				convertor.equal_step();
				if(enable) {
					if(( (open_in or (can.inID.control.open_passenger and (door == single_pass or door == double_pass))
					              or (can.inID.control.open_driver and door == driver)) and not begin and not clamp and not clamp_open and not first_time)/* or clamp_open or not clamp*/) {
						clamp_close = false;
						convertor.stop();
						convertor.power(99);
						switch (door) {
						case driver:
							if(open_in or can.inID.control.open_driver)
								convertor.back();
							break;
						case single_pass:
							if(open_in or can.inID.control.open_passenger)
								convertor.forward();
							break;
						case double_pass:
							if(open_in or can.inID.control.open_passenger)
								convertor.forward();
							break;
						case not_door:
							break;
						}
						state = opening; fix = false;/*going.start(5);*/ // back для водителя forward для пассажира // 60 passenger 90 driver
					} else if( ( close_in or (can.inID.control.close_passenger  and (door == single_pass or door == double_pass))
							              or (can.inID.control.close_driver and door == driver) ) and not end and not clamp and not clamp_close ) {
						fix = false;
						clamp_open = false;
						convertor.stop();
						power = 70; //was 50
						convertor.power(power);
						switch (door) {
						case driver:
							if(close_in or can.inID.control.close_driver)
								convertor.forward();
							break;
						case single_pass:
							if(close_in or can.inID.control.close_passenger)
								convertor.back();
							break;
						case double_pass:
							if(close_in or can.inID.control.close_passenger)
								convertor.back();
							break;
						case not_door:
							break;
						}
						state = closing; // // forward для водителя  back для пассажира
//						going.start(5);
					} else if ( (not open_in and not close_in and not can.inID.control.close_passenger and not can.inID.control.open_passenger) /*and (not can.inID.control.open and not can.inID.control.close)*/ ){
						clamp = false;  clamp_open = false; clamp_close = false;
					} else if (clamp_open and clamp_close){
						 convertor.stop();
					}
					if (end) {clamp_open = false; begin = false; }
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
					convertor.power(60); // 35 passenger 65 driver // was60
				}

//				if (abs(convertor.steps()) >= (160)) { // for passenger
//					convertor.power(10); // 35 passenger 65 driver // was60
//				}
//
				if(door == driver) {
					if ((not open_in and not can.inID.control.open_driver) or abs(convertor.steps()) >= (185)) {
						state = wait;
//					clamp = false; clamp_open = false;
//					convertor.stop();
						convertor.current_fix();
//					convertor.power(20);
						if (abs(convertor.steps()) >= (180)) {
							begin = true;
						}

					}
				} else if (door == single_pass or door == double_pass) {
					if ((not open_in and not can.inID.control.open_passenger) or abs(convertor.steps()) >= (185)) {
						state = wait;
//					clamp = false; clamp_open = false;
//					convertor.stop();
						convertor.current_fix();
//					convertor.power(20);
						if (abs(convertor.steps()) >= (180)) {
							begin = true;
						}

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

				if(door == driver) {
					if( (not close_in and not can.inID.control.close_driver) or end) {
						state = wait;
						convertor.stop();
						if(end) convertor.reset_steps();
						begin = false;
					}
				} else if (door == single_pass or door == double_pass) {
					if( (not close_in and not can.inID.control.close_passenger) or end) {
						state = wait;
						convertor.stop();
						if(end) convertor.reset_steps();
						begin = false;
					}
				}

			break;
			case clamp_opening:
				if( abs(convertor.steps()) >= (190) or service.outData.current > 150) {
									state = wait;
				//					clamp = false; clamp_open = false;
				//					convertor.stop();
									fix = true;
									convertor.current_fix();
				//					convertor.power(20);
									if(abs(convertor.steps()) >= (180)) {
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
					convertor.power(50);
					switch (door) {
					case driver:
						convertor.back();
						break;
					case single_pass:
						convertor.forward();
						break;
					case double_pass:
						convertor.forward();
						break;
					case not_door:
						break;
					}
					state = clamp_opening;
					clamp = false; // back for driver
				}

				if (clamp_open and delay.done()) {
					convertor.stop();
					delay.stop();
//					convertor.power(50);
//					switch (door) {
//					case driver:
//						convertor.forward();
//						break;
//					case single_pass:
//						convertor.back();
//						break;
//					case double_pass:
//						convertor.back();
//						break;
//					case not_door:
//						break;
//					}
					state = wait;
					clamp = false; // forward for driver
//					clamp_open = false;
				}

			break;

		} //switch(state)
	} // if (is_initial())
	} //void operator()
};

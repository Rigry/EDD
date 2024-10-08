#pragma once

#include "pin.h"
#include "interrupt.h"

// по инициализации проема 8 байт 0xFF на первом проеме на двух независимых дверях. Водительская со жгутом выходов на входа.

struct Control {
	bool res1            : 1; // 0
	bool close_passenger : 1; // 1
	bool open_passenger  : 1; // 2
    uint16_t             : 5;
	bool close_driver    : 1; // 0
	bool open_driver     : 1; // 1
	uint16_t             : 6;
};

struct State {
	bool res1            : 1; // 0
	bool res2            : 1; // 1
	bool res3            : 1; // 2
	bool open_passenger  : 1; // 3 1 - open door, 0 - close
	bool open_driver     : 1; // 4 1 - open door, 0 - close
	bool clamp           : 1; // 5
	uint16_t             : 9;
};

struct In_id{
	Control control;
	uint16_t res1;
	uint16_t res2;
	uint8_t res3;
	uint8_t initial; // 0xFF
};

struct Out_id{
	State state;
	uint16_t res1;
	uint16_t res2;
	uint16_t res3;
};

template <class InID_t, class OutID_t>
class CAN : TickSubscriber
{

  Pin& rts;
//  Interrupt& tx_interrupt;
  Interrupt& rx_interrupt;

  CAN_TxHeaderTypeDef TxHeader;
  CAN_RxHeaderTypeDef RxHeader;

  uint8_t TxData[8];
  uint8_t RxData[8];

  uint32_t TxMailBox;

  uint8_t Data[31];
  uint8_t DataRx[31];

  uint32_t ID{0x0DD};

  uint16_t time{0};
  uint16_t time_refresh{0};

  bool work{false};

  uint8_t ToChar(uint8_t c) {
  	if  (0 <= c && c <= 9) {
  		return c + '0';
  	}else if ('a' <= c && c <= 'f') {
  		return c + 'a' - 10;
  	}else if (10 <= c && c <= 15) {
  		return c + 'A' - 10;
  	}
  	//return c + '0';
  }

  uint8_t FromChar(uint8_t c) {

  	if ('0' <= c && c <= '9') {
  		return c - '0';
  	}else if('a' <= c && c <= 'f'){
  		return c - 'a' + 10;
  	} else if('A' <= c && c <= 'F') {
  		return c - 'A' + 10;
  	}

  }

public:

  CAN(Pin& rts, Interrupt& rx_interrupt, uint16_t time_refresh)
  	  : rts{rts}
  	  , rx_interrupt{rx_interrupt}
  	  , time_refresh{time_refresh}
  {
	  arInID[0] = arInID[1] = arInID[2] = arInID[3] = arInID[4] = arInID[5] = arInID[6] = arInID[7]= 0;
	  arOutID[0] = arOutID[1] = arOutID[2] = arOutID[3] = arOutID[4] = arOutID[5] = arOutID[6] = arOutID[7] = 0;
	  subscribed = false;
//	  if (time_refresh > 0)
//		  subscribe();
  }

  static const uint8_t InIDQty  = sizeof(InID_t);
  static const uint8_t OutIDQty = sizeof(OutID_t);

  union {
	InID_t inID;
    uint8_t arInID[InIDQty];
  };

  union {
    OutID_t outID;
    uint8_t arOutID[OutIDQty];
  };

  using Parent = CAN;

  struct can_rx_interrupt : Interrupting
  {
	  Parent& parent;
      can_rx_interrupt (Parent& parent) : parent(parent) {
          parent.rx_interrupt.subscribe (this);
      }
      void interrupt() {parent.receive();}
  }can_rx_{ *this };

  void change_ID(uint32_t v){
	  ID = v;
  }

  void transmit(){
	  	rts = true;
		TxHeader.DLC = 8;
		TxHeader.ExtId = 0;
		TxHeader.IDE = CAN_ID_STD;
		TxHeader.RTR = CAN_RTR_DATA;
		TxHeader.StdId = ID;
		TxHeader.TransmitGlobalTime = DISABLE;
		for (int i = 0; i < TxHeader.DLC; i++) {
			TxData[i] = arOutID[i];
		}
		HAL_CAN_AddTxMessage(&hcan, &TxHeader, TxData, &TxMailBox);
		rts = false;
  }

  void receive(){
		HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &RxHeader, RxData);

		switch(RxHeader.StdId) {
			case 0xDC:
				arInID[0] = RxData[0];
				arInID[1] = RxData[1];
				inID.initial = RxData[7];
				start_transmit();
				break;
			case 0xAA:
				outID.state.open_driver = RxData[0] & (1 << 4);
				break;
		}
	}

  bool is_work(){ return work; }

  void start_transmit() {
		if (not work) {
			work = true;
			if (time_refresh > 0)
				subscribe();
		}
  }

  void stop_transmit() { unsubscribe(); work = false; }

  void notify() {
	  if (time++ >= time_refresh) {
		  time = 0;
		  transmit();
	  }
//	  if(inID.control.on_off) stop_transmit();
  }

};


//Interrupt interrupt_can_tx;
Interrupt interrupt_can_rx;

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  interrupt_can_rx.interrupt();
}


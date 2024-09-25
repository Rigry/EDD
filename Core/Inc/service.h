#pragma once

#include "adc.h"
#include "interrupt.h"
#include "uart.h"

enum Function {
      read_03  = (uint8_t)0x03
    , read_04  = (uint8_t)0x04
    , write_16 = (uint8_t)0x10
}function;

enum Error_code {
      wrong_func   = (uint8_t)0x01
    , wrong_reg    = (uint8_t)0x02
    , wrong_value  = (uint8_t)0x03
}error_code;

struct Error {
	bool current             : 1;
	bool voltage_board_low   : 1;
	bool voltage_logic_low   : 1;
	bool voltage_drive_low   : 1;
	bool open                : 1;
	bool close               : 1;
	bool open_in             : 1;
	bool close_in            : 1;
	bool holla               : 1;
	uint16_t res             : 7;
};

struct In_data{

};

struct Out_data{
	uint16_t current;        // 0
	uint16_t voltage_board;  // 1
	uint16_t voltage_logic;  // 2
	uint16_t voltage_drive;  // 3
	Error error;             // 4
};

constexpr float k_adc   = 3.35 / 4025;
constexpr float k_adc_i = 3 * k_adc / 2 / 0.0167; // 3 и 2 потому что делитель 10 и 20 кОм, 0,025 В/А

template<class In_data_t, class Out_data_t>
class Service
{
	ADC_& adc;
	UART_<>& uart;
	Interrupt& interrupt_DMA;
	Interrupt& interrupt_usart;

	Timer timer;

	uint8_t reg{0};
	bool event{false};
	bool kolhoz{false};
	bool done{true};

	void uartInterrupt(){
		event = true;
		timer.stop();
	}

	void dmaInterrupt(){
		uart.receive();
	}

	using Parent = Service;

	struct uart_interrupt: Interrupting {
		Parent &parent;
		uart_interrupt(Parent &parent) :
				parent(parent) {
			parent.interrupt_usart.subscribe(this);
		}
		void interrupt() override {
			parent.uartInterrupt();
		}
	} uart_ { *this };

	struct dma_interrupt: Interrupting {
		Parent &parent;
		dma_interrupt(Parent &parent) :
				parent(parent) {
			parent.interrupt_DMA.subscribe(this);
		}
		void interrupt() override {
			parent.dmaInterrupt();
		}
	} dma_ {*this};

public:

	static constexpr uint16_t InDataQty = sizeof(In_data_t) / 2;
	static constexpr uint16_t OutDataQty = sizeof(Out_data_t) / 2;

	union {
		In_data_t inData;
		uint16_t arInData[InDataQty];
	};
	union {
		Out_data_t outData;
		uint16_t arOutData[OutDataQty];
	};
	union {
		In_data_t inDataMin;
		uint16_t arInDataMin[InDataQty];
	};
	union {
		In_data_t inDataMax;
		uint16_t arInDataMax[InDataQty];
	};

	Service (
		  ADC_& adc
		, UART_<>& uart
		, Interrupt& interrupt_DMA
		, Interrupt& interrupt_usart
	) : adc              {adc}
	  ,	uart             {uart}
	  , interrupt_DMA    {interrupt_DMA}
      , interrupt_usart  {interrupt_usart}
      , arInData { }, arOutData { }, arInDataMin { }, arInDataMax {}
	{
		uart.receive();
		timer.start(2000);
	}

	void operator()(){

		outData.current        = k_adc * (adc.current_value() * 30 / 20) * 1000;
		outData.voltage_board  = k_adc * adc[VB] * 100;
		outData.voltage_logic  = k_adc * adc[VL] * 100;
		outData.voltage_drive  = k_adc * adc[VD] * 100;

		outData.error.current           = (outData.current >= 850);
		outData.error.voltage_board_low = (outData.voltage_board <= 180);
		outData.error.voltage_logic_low = (outData.voltage_logic <= 180);
		outData.error.voltage_drive_low = (outData.voltage_drive <= 180);

		kolhoz ^= timer.event();

		if (event or kolhoz) {
			if(uart.buffer[0] == 4 or kolhoz) {
				uart.buffer.clear();
				uart.buffer << outData.current
							<< outData.voltage_board
						    << outData.voltage_logic
							<< outData.voltage_drive
							<< arOutData[4];

			} else if(uart.buffer[0] == '+') {
				uart.buffer.clear();
				uart.buffer << 'O';
				uart.buffer << 'K';
			}
			event = false;
			kolhoz = false;
			if(uart.buffer.size())
				uart.transmit();
			else
				uart.receive();
		}
	}

};

Interrupt interrupt_dma;
Interrupt interrupt_uart;


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART3) {
		interrupt_dma.interrupt();
	}
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
	if (huart->Instance == USART3) {
		interrupt_uart.interrupt();
	}
}

#define b0 1
#define b1 2
#define b2 4
#define b3 8
#define b4 16
#define b5 32
#define b6 64
#define b7 128
#define b8 256
#define b9 512
#define b10 1024
#define b11 2048
#define b12 4096
#define b13 8192
#define b14 16384
#define b15 32768
#define b16 (1 << 16)
#define b17 (1 << 17)
#define b18 (1 << 18)
#define b19 (1 << 19)
#define b20 (1 << 20)
#define b21 (1 << 21)
#define b22 (1 << 22)
#define b23 (1 << 23)
#define b24 (1 << 24)
#define b25 (1 << 25)
#define b26 (1 << 26)
#define b27 (1 << 27)
#define b28 (1 << 28)
#define b29 (1 << 29)
#define b30 (1 << 30)
#define b31 (unsigned int)(1 << 31)

#include "thuvien.h"

void uart_init()
{
	const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};
	uart_param_config(UART_NUM_0, &uart_config);
    uart_driver_install(UART_NUM_0, 256, 0, 0, NULL, 0);
    uart_set_pin(UART_NUM_0, GPIO_NUM_1, GPIO_NUM_3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void uart_send_hex(uint64_t n)
{
	uint8_t hex_characters[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	uint8_t i;
	uint8_t tmp_n[16];
	for(i=0; i<16; i++)
	{
		tmp_n[i] = hex_characters[n % 16];
		n /= 16;
		if(!n) break;
	}
	i++;
	for(; i>0; i--) uart_write_bytes(UART_NUM_0, &tmp_n[i-1], 1);
}

void uart_send_dec(uint64_t n)
{
	uint8_t i;
	uint8_t tmp_n[20];
	for(i=0; i<20; i++)
	{
		tmp_n[i] = (n % 10) + 48;
		n /= 10;
		if(!n) break;
	}
	i++;
	for(; i>0; i--) uart_write_bytes(UART_NUM_0, &tmp_n[i-1], 1);
}

void uart_send_bin(uint64_t n)
{
	uint8_t i;
	uint8_t tmp_n[64];
	for(i=0; i<64; i++)
	{
		tmp_n[i] = (n % 2) + 48;
		n /= 2;
		if(!n) break;
	}
	i++;
	for(; i>0; i--) uart_write_bytes(UART_NUM_0, &tmp_n[i-1], 1);
}

void uart_send_raw_ir(rmt_item32_t* items, uint8_t number)
{
	uint8_t i;
	uart_send_dec(number);
	uart_write_bytes(UART_NUM_0, "\r\n", 2);
	for(i=0; i<number; i++)
	{
		uart_send_dec(items[i].duration0);
		uart_write_bytes(UART_NUM_0, "\t", 1);
		uart_send_dec(items[i].duration1);
		uart_write_bytes(UART_NUM_0, "\r\n", 2);
	}
	uart_write_bytes(UART_NUM_0, "!", 1);
}

void uart_send_custom_ir(struct CUSTOM_IR custom_ir)
{
	uint8_t i;
	uart_send_dec(custom_ir.number_section_ir);
	uart_write_bytes(UART_NUM_0, "\r\n", 2);
	
	for(i=0; i<4; i++)
	{
		uart_send_dec(custom_ir.logic_ir[i]);
		uart_write_bytes(UART_NUM_0, "\t", 1);
	}
	uart_write_bytes(UART_NUM_0, "\r\n", 2);
	
	for(i=0; i<custom_ir.number_section_ir; i++)
	{
		uart_send_dec(custom_ir.size_section_ir[i]);
		uart_write_bytes(UART_NUM_0, "\t", 1);
	}
	uart_write_bytes(UART_NUM_0, "\r\n", 2);
	
	for(i=0; i<4*custom_ir.number_section_ir; i++)
	{
		uart_send_dec(custom_ir.section_ir[i]);
		uart_write_bytes(UART_NUM_0, "\t", 1);
	}
	uart_write_bytes(UART_NUM_0, "\r\n", 2);
	
	for(i=4; i>0; i--)
	{
		uart_send_hex(custom_ir.result_section_ir[i-1]);
		uart_write_bytes(UART_NUM_0, "\t", 1);
	}
	uart_write_bytes(UART_NUM_0, "!", 1);
}

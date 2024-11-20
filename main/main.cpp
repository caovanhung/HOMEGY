#include "thuvien_ac.h"
#include "thuvien.h"

extern "C" void app_main()
{
	rmt_item32_t* items_rx_ir;
	size_t number_rx_ir;
	size_t length_buffer_uart;
	uint8_t isreceive = 1;
	uint8_t isdisplay = 1;
	uint8_t isdecode = 1;
	uint8_t isteaching = 1;
	uint8_t iswaittingvoice = 1;
	uint8_t timewaittingteaching = 0;
	uint8_t voicerequest[3] = {RECV_VOICE_CMD, 0, 0};
	uint8_t tmprequest[3] = {0, 0, 0};
	uint8_t tmpqueue[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t isok = 0;
	uint8_t statusdecode = 0;
	uint32_t timeresetvoice = 0;
	
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, 1);
    gpio_set_direction(PIN_BOOT, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_BOOT, 1);
	gpio_set_direction(PIN_RES, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_RES, 1);
	partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "storage");
	config_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "config");
    load_memory_system();

    if(memory[0] != 1){
		clear_memory_system();
		save_memory_system();
	}
	uart_init();

    if((isota > 0) && (isota < 5)){
		voicerequest[1] = 246;
		uart_write_bytes(UART_NUM_0, voicerequest, 3);
    	vTaskDelay(5000);
		if(wifi_init())
    	{
			voicerequest[1] = 247;
			uart_write_bytes(UART_NUM_0, voicerequest, 3);
			vTaskDelay(5000);
			gpio_set_level(LED, 0);
			ota_partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "file");
			if(isota == 1)
			{
				if(get_wifi_data(1)) isok = ota_firmware();
			}
			else
			{
				if(isota == 2)
				{
					if(get_wifi_data(2)) isok = ota_data();
				}
				else
				{
					if(isota == 3)
					{
						if(get_wifi_data(3)) isok = ota_voice();
					}
					else
					{
						if(isota == 4)
						{
							if(get_wifi_data(4)) isok = ota_voice();
							if(get_wifi_data(1)) isok &= ota_firmware();
						}
					}
				}
			}
		}
		else
		{
			voicerequest[1] = 248;
			uart_write_bytes(UART_NUM_0, voicerequest, 3);
			vTaskDelay(5000);
			isok = 0;
		}
		if(isok) voicerequest[1] = 249;
		else voicerequest[1] = 250;
		uart_write_bytes(UART_NUM_0, voicerequest, 3);
		vTaskDelay(5000);
		isota = 0;
		save_memory_system();
		esp_restart();
	}

	gpio_reset_pin(GPIO_NUM_16);
	gpio_reset_pin(GPIO_NUM_17);
	gpio_set_direction(BUTTON, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON, GPIO_PULLUP_ONLY);
    nvs_flash_init(); //khởi tạo bộ nhớ nvs,
	ble_mesh_init();
    ir_init();
	irrecv.enableIRIn();
	isreceive = check_params_remote();
    if(isreceive != 1) rmt_rx_start(RMT_CHANNEL_0, true);
    ble_mesh_send(gateway, 0x8204, 3, 0xF1, 0, 1, 0, 0, 0, 0, 0);
    voicerequest[1] = 245;
	uart_write_bytes(UART_NUM_0, voicerequest, 3);
	gpio_set_level(LED, 0);
    
	while(1)
    {
		if(isreceive != 1){ 
			items_rx_ir = (rmt_item32_t*)xRingbufferReceive(ring_buffer_ir, &number_rx_ir, 100);
	    	if (items_rx_ir){
				if(number_rx_ir > 100){
					if((isteaching == 3) || (isteaching == 5)){
						if(isteaching == 3){
							isok = add_raw_key(items_rx_ir, number_rx_ir/4, tmprequest[0], tmprequest[1], tmprequest[2]);
							ble_mesh_send(gateway, 0xE50211, 5, SEND_TEACHING_CMD, tmprequest[0], tmprequest[1], tmprequest[2], isok, 0, 0, 0);
							voicerequest[1] = 253;
							uart_write_bytes(UART_NUM_0, voicerequest, 3);
							gpio_set_level(LED, 0);
							isreceive = check_params_remote();
		    				if(isreceive != 1) rmt_rx_start(RMT_CHANNEL_0, true);
		    				else rmt_rx_stop(RMT_CHANNEL_0);
							isteaching = 1;
						}
						else{
							fill_recv_ir(items_rx_ir, number_rx_ir/4);
							if(irrecv.decode(&results)){									
								if(find_ac_teaching()){
									voicerequest[1] = 253;
									uart_write_bytes(UART_NUM_0, voicerequest, 3);
									gpio_set_level(LED, 0);
									isreceive = check_params_remote();
				    				if(isreceive != 1) rmt_rx_start(RMT_CHANNEL_0, true);
				    				else rmt_rx_stop(RMT_CHANNEL_0);
									isteaching = 1;
								}
							}
							delete data_ir_recv;
						}
					}
					else{
						if(isdisplay == 2){
							if(isdecode == 2){
								if(decode_custom_ir(items_rx_ir, number_rx_ir/4)) uart_send_custom_ir(custom_ir);
							}
							else {
								uart_send_raw_ir(items_rx_ir, number_rx_ir/4);
							}
							voicerequest[1] = 255;
							uart_write_bytes(UART_NUM_0, voicerequest, 3);
							gpio_set_level(LED, 1);
							vTaskDelay(100);
							gpio_set_level(LED, 0);
						}
						else{
							statusdecode = decode_custom_ir(items_rx_ir, number_rx_ir/4);
							if(isreceive & 0x02){
								if(find_memory_remote(items_rx_ir, number_rx_ir/4, statusdecode)){
									voicerequest[1] = 255;
									uart_write_bytes(UART_NUM_0, voicerequest, 3);
									gpio_set_level(LED, 1);
									vTaskDelay(100);
									gpio_set_level(LED, 0);
								}
							}
							if(isreceive & 0x04){
								fill_recv_ir(items_rx_ir, number_rx_ir/4);
								if(irrecv.decode(&results)){
									if(find_ac_remote()){
										voicerequest[1] = 255;
										uart_write_bytes(UART_NUM_0, voicerequest, 3);
										gpio_set_level(LED, 1);
										vTaskDelay(100);
										gpio_set_level(LED, 0);
									}
								}
								delete data_ir_recv;
								if(statusdecode){
									if(find_ac_custom_remote()){
										voicerequest[1] = 255;
										uart_write_bytes(UART_NUM_0, voicerequest, 3);
										gpio_set_level(LED, 1);
										vTaskDelay(100);
										gpio_set_level(LED, 0);
									}
								}
							}
						}
					}
				}
				irrecv.resume();
				vRingbufferReturnItem(ring_buffer_ir, items_rx_ir);
			}
		}
		else {
			vTaskDelay(100);
		}
		
		uart_get_buffered_data_len(UART_NUM_0, &length_buffer_uart);
		
		if(length_buffer_uart >= 8){
			uart_read_bytes(UART_NUM_0, (void*)tmpqueue, 8, 100);
			if(lockdevice)
			{
				if(tmpqueue[0] == CONFIG_MEMORY_CMD) set_queue(tmpqueue);
			}
			else set_queue(tmpqueue);
		}
		
		get_queue();

		switch(request[0])
		{
			case CONFIG_MEMORY_CMD:
				gpio_set_level(LED, 1);
				gpio_set_level(PIN_RES, 0);
				if(request[1] == 1) {
					clear_flash(request[2], request[3]);
				}
				else{
					if(request[1] == 2) {
						save_flash();
					}
					else{
						if(request[1] == 3) {
							load_flash(request[2], request[3], request[4]);
						}
					}
				}
				uart_write_bytes(UART_NUM_0, "!", 1);
				vTaskDelay(100);
				gpio_set_level(PIN_RES, 1);
				gpio_set_level(LED, 0);
				request[0] = NONE_CMD;
				break;
			case RECV_KEY_CMD:
				if(isreceive != 1) {
					rmt_rx_stop(RMT_CHANNEL_0);
				}
				gpio_set_level(LED, 1);
				isok = send_memory_ir(request[1], request[2], request[3], request[4]);
				if(isok){
					if(check_list_remote(request[1], request[2], request[3])) {
						ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 8, request[1], request[2], request[3], request[4], 2, 17);
					}
					else {
						ble_mesh_send(gateway, 0xE50211, 8, RECV_KEY_CMD, 1, 0, 0, 0, 0, 0, 0);
					}
					send_buffer_ir();
				}
				else {
					ble_mesh_send(gateway, 0xE50211, 8, RECV_KEY_CMD, 0, 0, 0, 0, 0, 0, 0);
				}
				voicerequest[1] = 255;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				if(isreceive != 1){
					rmt_rx_start(RMT_CHANNEL_0, true);
				}
				irrecv.resume();
				request[0] = NONE_CMD;
				break;
			case RECV_AC_CMD:
				gpio_set_level(LED, 1);
				if(isreceive != 1) rmt_rx_stop(RMT_CHANNEL_0);
				isok = send_ac_ir(request[1], request[2], request[3], request[4], request[5], request[6], request[7]);
				if(isok)
				{
					if(check_list_remote(request[1], request[2], request[3])) ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 8, request[1], request[2], request[3], request[4], request[5], ((request[6]<<4)&0xF0)|(request[7]&0x0F));
					else ble_mesh_send(gateway, 0xE50211, 8, RECV_AC_CMD, 1, 0, 0, 0, 0, 0, 0);
					send_buffer_ir();
				}
				else ble_mesh_send(gateway, 0xE50211, 8, RECV_AC_CMD, 0, 0, 0, 0, 0, 0, 0);
				voicerequest[1] = 255;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				if(isreceive != 1) rmt_rx_start(RMT_CHANNEL_0, true);
				irrecv.resume();
				request[0] = NONE_CMD;
				break;
			case RECV_PARAM_CMD:
				gpio_set_level(LED, 1);
				isok = 0;
				if(request[1] == 1) isok = delete_params_remote(request[3], request[4], request[5]);
				else
				{
					if(request[1] == 2) isok = add_params_remote(request[2], request[3], request[4], request[5]);
					else
					{
						if(request[1] == 3) isok = delete_all_params_remote();
					}
				}
				ble_mesh_send(gateway, 0xE50211, 2, RECV_PARAM_CMD, isok, 0, 0, 0, 0, 0, 0);
				save_memory_system();
				isreceive = check_params_remote();
				if(isreceive != 1) rmt_rx_start(RMT_CHANNEL_0, true);
				else rmt_rx_stop(RMT_CHANNEL_0);
				voicerequest[1] = 255;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				request[0] = NONE_CMD;
				break;
			case RECV_MODE_CMD:
				gpio_set_level(LED, 1);
				isdecode = request[1]+1;
				isdisplay = request[2]+1;
				isteaching = request[3]+1;
				isok = 0;
				if(isteaching == 2)
				{
					isok = delete_raw_key(request[4], request[5], request[6]);
					isteaching = 1;
				}
				else
				{
					if(isteaching == 4)
					{
						clear_flash(request[4], request[5]);
						isok = 1;
						isteaching = 1;
					}
					else
					{
						if(isteaching == 3)
						{
							if((request[4]+256*request[5] > 1) && (request[4]+256*request[5] < 21) && request[6])
							{
								voicerequest[1] = 252;
								uart_write_bytes(UART_NUM_0, voicerequest, 3);
								tmprequest[0] = request[4];
								tmprequest[1] = request[5];
								tmprequest[2] = request[6];
								timewaittingteaching = 0;
								isok = 1;
							}
							else isteaching = 1;
						}
						else
						{
							if(isteaching == 5)
							{
								voicerequest[1] = 252;
								uart_write_bytes(UART_NUM_0, voicerequest, 3);
								tmprequest[0] = request[4];
								tmprequest[1] = request[5];
								tmprequest[2] = request[6];
								timewaittingteaching = 0;
								isok = 1;
							}
							else isteaching = 1;
						}
					}
				}
				ble_mesh_send(gateway, 0xE50211, 2, RECV_MODE_CMD, isok, 0, 0, 0, 0, 0, 0);
				if((isdisplay == 2) || (isteaching == 3) || (isteaching == 5))
				{
					rmt_rx_start(RMT_CHANNEL_0, true);
					isreceive = 2;
				}
				if(isteaching == 1)
				{
					voicerequest[1] = 255;
					uart_write_bytes(UART_NUM_0, voicerequest, 3);
					vTaskDelay(100);
					gpio_set_level(LED, 0);
				}
				request[0] = NONE_CMD;
				break;
			case RECV_SCRIPT_CMD:
				gpio_set_level(LED, 1);
				isok = 0;
				if((request[3] & 0xC0) == 0x40) isok = delete_script(request[1], request[2], request[3], request[4], request[5], request[6]);
				else
				{
					if((request[3] & 0xC0) == 0xC0) isok = delete_all_script(request[1], request[2]);
					else
					{
						if((request[3] & 0xC0) == 0x80) isok = add_script(request[1], request[2],request[3], request[4], request[5], request[6],request[7]);
					}
				}
				ble_mesh_send(gateway, 0x8245, 8, 0, request[1], request[2], 0, 0, 0, 0, 0);
				voicerequest[1] = 255;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				request[0] = NONE_CMD;
				break;
			case RECV_OTA_CMD:
				gpio_set_level(LED, 1);
				isok = save_ota(request[1], 65536*request[2]+256*request[3]+request[4], 65536*request[5]+256*request[6]+request[7]);
				ble_mesh_send(gateway, 0xE50211, 2, RECV_OTA_CMD, isok, 0, 0, 0, 0, 0, 0);
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				if(isok) esp_restart();
				request[0] = NONE_CMD;
				break;
			case RECV_VOICE_CMD:
				if(request[1]+256*request[2] == wakeupvoice)
				{
					iswaittingvoice = 2;
					gpio_set_level(LED, 1);
					voicerequest[1] = 100;
					uart_write_bytes(UART_NUM_0, voicerequest, 3);
				}
				else
				{
					if(iswaittingvoice == 2)
					{
						if((request[1]+256*request[2] > 0) && (request[1]+256*request[2] < 255))
						{
							if(isreceive != 1) rmt_rx_stop(RMT_CHANNEL_0);
							voicerequest[1] = request[1];
							uart_write_bytes(UART_NUM_0, voicerequest, 3);
							if((request[1]+256*request[2] > 0) && (request[1]+256*request[2] < 20))
							{
								if(find_script(request[1], request[2])) vTaskDelay(1000);
								if(request[1]+256*request[2] == 19) find_voice_remote(2, 1);
								else find_voice_remote(2, request[1]+256*request[2]);
								ble_mesh_send(gateway, 0xE50211, 8, RECV_VOICE_CMD, request[1], request[2], 0, 0, 0, 0, 0);
							}
							else
							{
								if((request[1]+256*request[2] > 19) && (request[1]+256*request[2] < 60))
								{
									if(find_script(request[1], request[2])) vTaskDelay(1000);
									if(request[1]+256*request[2] == 45) find_voice_remote(3, 31);
									else find_voice_remote(3, request[1]+256*request[2]);
									ble_mesh_send(gateway, 0xE50211, 8, RECV_VOICE_CMD, request[1], request[2], 0, 0, 0, 0, 0);
								}
								else
								{
									if((request[1]+256*request[2] > 59) && (request[1]+256*request[2] < 71))
									{
										if(find_script(request[1], request[2])) vTaskDelay(1000);
										if(request[1]+256*request[2] == 70) find_voice_remote(4, 60);
										else
										{
											if(request[1]+256*request[2] == 69) find_voice_remote(4, 61);
											else find_voice_remote(4, request[1]+256*request[2]);
										}
										ble_mesh_send(gateway, 0xE50211, 8, RECV_VOICE_CMD, request[1], request[2], 0, 0, 0, 0, 0);
									}
									else
									{
										if(request[1]+256*request[2] == 244)
										{
											iswaittingvoice = 1;
											gpio_set_level(LED, 0);
										}
										else
										{
											if(request[1]+256*request[2] > 109)
											{
												find_script(request[1], request[2]);
												ble_mesh_send(gateway, 0xE50211, 8, RECV_VOICE_CMD, request[1], request[2], 0, 0, 0, 0, 0);
											}
										}
									}
								}
							}
							if(isreceive != 1) rmt_rx_start(RMT_CHANNEL_0, true);
							irrecv.resume();
						}
						else
						{
							if(request[1]+256*request[2] == 0)
							{
								voicerequest[1] = 244;
								uart_write_bytes(UART_NUM_0, voicerequest, 3);
							}
						}
						iswaittingvoice = 1;
						gpio_set_level(LED, 0);
					}
				}
				request[0] = NONE_CMD;
				break;
			case RECV_LOCAL_CMD:
				if(isreceive != 1) rmt_rx_stop(RMT_CHANNEL_0);
				isok = run_script(request[1]+256*request[2]);
				if(isreceive != 1) rmt_rx_start(RMT_CHANNEL_0, true);
				irrecv.resume();
				if(count_relay_list() > 3 * CONFIG_BLE_MESH_CRPL / 5)
				{
					if(!isok) remove_relay_list(request[3]+256*request[4]);
				}
				request[0] = NONE_CMD;
				break;
			case RECV_WAKEUP_CMD:
				gpio_set_level(LED, 1);
				if((request[1] + 256*request[2] > 99) && (request[1] + 256*request[2] < 110))
				{
					wakeupvoice = request[1] + 256*request[2];
					save_memory_system();
					isok = 1;
				}
				else isok = 0;
				ble_mesh_send(gateway, 0xE50211, 2, RECV_WAKEUP_CMD, isok, 0, 0, 0, 0, 0, 0);
				uart_write_bytes(UART_NUM_0, request, 3);
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				request[0] = NONE_CMD;
				break;
			case RECV_RESET_CMD:
				kick_out();
				gpio_set_level(LED, 1);
				voicerequest[0] = RECV_WAKEUP_CMD;
				voicerequest[1] = 100;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(100);
				voicerequest[0] = RECV_VOLUME_CMD;
				voicerequest[1] = 100;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(100);
				voicerequest[0] = RECV_VOICE_CMD;
				voicerequest[1] = 251;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(5000);
				esp_restart();
				request[0] = NONE_CMD;
				break;
			case RECV_VOLUME_CMD:
				gpio_set_level(LED, 1);
				ble_mesh_send(gateway, 0xE50211, 2, RECV_VOLUME_CMD, 1, 0, 0, 0, 0, 0, 0);
				uart_write_bytes(UART_NUM_0, request, 3);
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				request[0] = NONE_CMD;
				break;
			case SEND_SCRIPT_CMD:
				gpio_set_level(LED, 1);
				send_list_script();
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				request[0] = NONE_CMD;
				break;
			case SEND_REMOTE_CMD:
				gpio_set_level(LED, 1);
				send_list_remote();
				vTaskDelay(100);
				gpio_set_level(LED, 0);
				request[0] = NONE_CMD;
				break;
		}
		
		if((isteaching == 3) || (isteaching == 5)){
			timewaittingteaching++;
			if(timewaittingteaching >= 150)
			{
				isteaching = 1;
				ble_mesh_send(gateway, 0xE50211, 5, SEND_TEACHING_CMD, tmprequest[0], tmprequest[1], tmprequest[2], 2, 0, 0, 0);
				voicerequest[1] = 254;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				gpio_set_level(LED, 0);
				isreceive = check_params_remote();
    			if(isreceive != 1) rmt_rx_start(RMT_CHANNEL_0, true);
    			else rmt_rx_stop(RMT_CHANNEL_0);
			}
		}
		
		if(timewaittingresponse){
			timewaittingresponse--;
			if(!timewaittingresponse) ble_mesh_send(gateway, 0x8204, 3, 0xF1, 0, 1, 0, 0, 0, 0, 0);
		}
		
		if(timeresetvoice > 36000){
			gpio_set_level(PIN_RES, 0);
    		vTaskDelay(500);
    		gpio_set_level(PIN_RES, 1);
    		gpio_set_level(LED, 0);
			iswaittingvoice = 1;
			timeresetvoice = 0;
		}
		else {
			timeresetvoice++;
		}

		//Đọc giá trị nút nhấn để reset
		if(!gpio_get_level(BUTTON))
		{
			vTaskDelay(2000);
			if(!gpio_get_level(BUTTON))
			{
				kick_out();
				gpio_set_level(LED, 1);
				voicerequest[0] = RECV_WAKEUP_CMD;
				voicerequest[1] = 100;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(100);
				voicerequest[0] = RECV_VOLUME_CMD;
				voicerequest[1] = 100;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(100);
				voicerequest[0] = RECV_VOICE_CMD;
				voicerequest[1] = 251;
				uart_write_bytes(UART_NUM_0, voicerequest, 3);
				vTaskDelay(5000);
				while(!gpio_get_level(BUTTON));
				esp_restart();
			}
		}
	}
}

uint8_t send_ac_ir(uint8_t idl, uint8_t idh, uint8_t index, uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	uint16_t id = idl+256*idh;
	if((id < 21) || (idl%2) || (!index)) return 0;
	if((temp != 1) && ((temp < 16) || (temp > 30))) return 0;
	if((!mode) || (mode > 5) || (!fan) || (fan > 4) || (!swing) || (swing > 7)) return 0;
	data_ac = new uint16_t[1024];
	data_ac[0] = 1;
	setup_send_ac(idl, idh, index, temp, mode, fan, swing);
	if(data_ac[0] == 1)
	{
		delete data_ac;
		data_ac = NULL;
		return 0;
	}
	uint16_t i = 0;
	if(items_tx_ir != NULL)
	{
		free(items_tx_ir);
		items_tx_ir = NULL;
	}
	items_tx_ir = (rmt_item32_t*)malloc(2*(data_ac[0]-1));
	for(i=0; i<(data_ac[0]-1)/2; i++)
	{
		items_tx_ir[i].level0 = LOGIC0;
		items_tx_ir[i].duration0 = data_ac[2*i+1];
		items_tx_ir[i].level1 = LOGIC1;
		items_tx_ir[i].duration1 = data_ac[2*i+2];
	}
	number_tx_ir = i;
	delete data_ac;
	data_ac = NULL;
	save_status_remote(idl, idh, index, temp, mode, fan, swing);
	return 1;
}

void fill_recv_ir(rmt_item32_t* items, uint8_t number)
{
	uint16_t i;
	data_ir_recv = new uint16_t[2*number+1];
	data_ir_recv[0] = 2*number;
	data_ir_recv[1] = 1;
	for(i=0; i<number; i++)
	{
		data_ir_recv[2*i+2] = (uint16_t)(items[i].duration0 / 3.125);
		if(i < number-1) data_ir_recv[2*i+3] = (uint16_t)(items[i].duration1 / 3.125);
	}
}

uint8_t find_ac_remote()
{
	uint8_t i;
	uint8_t response[4] = {0, 0, 0, 0};
	for(i=0; i<100; i+=4)
	{
		if((params[i] == 5) || (params[i] == 8))
		{
			setup_receive_ac(params[i+1], params[i+2], params[i+3]);
			if(decode_ac != NULL)
			{
				if(decode_ac(response))
				{
					if((response[0] != 1) && ((response[0] < 16) || (response[0] > 30))) response[0] = 30;
					if((response[1] < 1) || (response[1] > 5)) response[1] = 2;
					if((response[2] < 1) || (response[1] > 4)) response[2] = 1;
					if((response[2] < 1) || (response[1] > 7)) response[3] = 1;
					ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 5, params[i+1], params[i+2], params[i+3], response[0], response[1], ((response[2]<<4)&0xF0)|(response[3]&0x0F));
					save_status_remote(params[i+1], params[i+2], params[i+3], response[0], response[1], response[2], response[3]);
					return 1;
				}
			}
		}
	}
	return 0;
}

uint8_t find_ac_custom_remote()
{
	uint8_t i;
	uint8_t response[4] = {0, 0, 0, 0};
	for(i=0; i<100; i+=4)
	{
		if((params[i] == 5) || (params[i] == 8))
		{
			setup_receive_custom_ac(params[i+1], params[i+2], params[i+3]);
			if(decode_ac != NULL)
			{
				if(decode_ac(response))
				{
					if((response[0] != 1) && ((response[0] < 16) || (response[0] > 30))) response[0] = 30;
					if((response[1] < 1) || (response[1] > 5)) response[1] = 2;
					if((response[2] < 1) || (response[1] > 4)) response[2] = 1;
					if((response[2] < 1) || (response[1] > 7)) response[3] = 1;
					ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 5, params[i+1], params[i+2], params[i+3], response[0], response[1], ((response[2]<<4)&0xF0)|(response[3]&0x0F));
					save_status_remote(params[i+1], params[i+2], params[i+3], response[0], response[1], response[2], response[3]);
					return 1;
				}
			}
		}
	}
	return 0;
}

uint8_t find_ac_teaching()
{
	uint8_t i;
	uint8_t j;
	uint8_t response[4];
	for(i=22; i<116; i+=2)
	{
		for(j=1; j<9; j++)
		{
			setup_receive_ac(i%255, i/255, j);
			if(decode_ac != NULL)
			{
				if(decode_ac(response))
				{
					ble_mesh_send(gateway, 0xE50211, 5, SEND_TEACHING_CMD, i%255, i/255, j, 1, 0, 0, 0);
					return 1;
				}
			}
		}
	}
	return 0;
}

void find_voice_remote(uint8_t type, uint16_t command)
{
	uint8_t i;
	uint8_t info[4];
	uint8_t isok;
	for(i=0; i<100; i+=4)
	{
		isok = 0;
		if(type == 2)
		{
			if((params[i] == 2) || (params[i] == 6)) isok = 1;
		}
		else
		{
			if(type == 3)
			{
				if((params[i] == 3) || (params[i] == 7) || (params[i] == 8) || (params[i] == 10)) isok = 1;
			}
			else
			{
				if(type == 4)
				{
					if((params[i] == 4) || (params[i] == 9)) isok = 1;
				}
			}
		}
		if(isok == 1)
		{
			isok = 0;
			if((params[i] == 2) || (params[i] == 4) || (params[i] == 6) || (params[i] == 9) || (params[i] == 3) || (params[i] == 7))
			{
				if(type == 2)
				{
					info[0] = command;
					info[1] = 0;
					info[2] = 0;
					info[3] = 0;
				}
				else
				{
					if(type == 4)
					{
						info[0] = command - 59;
						info[1] = 0;
						info[2] = 0;
						info[3] = 0;
					}
					else
					{
						if(type == 3)
						{
							if(!load_status_remote(params[i+1], params[i+2], params[i+3], info)) info[0] = 26;
							info[1] = 2;
							info[2] = 1;
							info[3] = 1;
							if(command == 20) info[0] = 1;
							else
							{
								if((command > 20) && (command < 36)) info[0] = command - 5;
								else
								{
									if(command == 36) info[0]++;
									else
									{
										if(command == 37) info[0]--;
									}
								}
							}
						}
					}
				}
				isok = send_memory_ir(params[i+1], params[i+2], params[i+3], info[0]);
				if(isok)
				{
					if(type == 3) ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 3, params[i+1], params[i+2], params[i+3], info[0], info[1], ((info[2]<<4)&0xF0)|(info[3]&0x0F));
					send_buffer_ir();
				}
			}
			else
			{
				if((params[i] == 8) || (params[i] == 10))
				{
					if(!load_status_remote(params[i+1], params[i+2], params[i+3], info))
					{
						info[0] = 26;
						info[1] = 2;
						info[2] = 1;
						info[3] = 1;
					}
					if(command == 20) info[0] = 1;
					else
					{
						if((command > 20) && (command < 36)) info[0] = command - 5;
						else
						{
							if(command == 36) info[0]++;
							else
							{
								if(command == 37) info[0]--;
								else
								{
									if((command > 37) && (command < 43)) info[1] = command - 37;
								}
							}
						}
					}
					isok = send_ac_ir(params[i+1], params[i+2], params[i+3], info[0], info[1], info[2], info[3]);
					if(isok)
					{
						ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 6, params[i+1], params[i+2], params[i+3], info[0], info[1], ((info[2]<<4)&0xF0)|(info[3]&0x0F));
						send_buffer_ir();
					}
				}
			}
			vTaskDelay(1000);
		}
	}
}

uint8_t run_script(uint16_t id)
{
	if(!id) return 0;
	uint16_t i;
	uint8_t voicerequest[3] = {RECV_VOICE_CMD, 0, 0};
	uint8_t cache[11];
	uint8_t isok = 0;
	uint8_t isuse = 0;
	for(i=0; i<4096; i+=11)
	{
		esp_partition_read_raw(partition, 4096+i, cache, 11);
		if(cache[0] == '!') break;
		if(cache[1]+256*cache[2] == id)
		{
			isuse = 1;
			if(cache[3] == RECV_KEY_CMD)
			{
				if(find_list_remote(cache[4], cache[5], cache[6]))
				{
					isok = send_memory_ir(cache[4], cache[5], cache[6], cache[7]);
					if(isok)
					{
						if((cache[4]+256*cache[5] > 20) && (cache[4]%2==0)) ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 7, cache[4], cache[5], cache[6], cache[7], 2, 17);
						else ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 7, cache[4], cache[5], cache[6], cache[7], 0, 0);
						send_buffer_ir();
						voicerequest[1] = 255;
						uart_write_bytes(UART_NUM_0, voicerequest, 3);
						vTaskDelay(1000);
					}
				}
			}
			else
			{
				if(cache[3] == RECV_AC_CMD)
				{
					if(find_list_remote(cache[4], cache[5], cache[6]))
					{
						isok = send_ac_ir(cache[4], cache[5], cache[6], cache[7], cache[8], cache[9], cache[10]);
						if(isok)
						{
							ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 7, cache[4], cache[5], cache[6], cache[7], cache[8], ((cache[9]<<4)&0xF0)|(cache[10]&0x0F));
							send_buffer_ir();
							voicerequest[1] = 255;
							uart_write_bytes(UART_NUM_0, voicerequest, 3);
							vTaskDelay(1000);
						}
					}
				}
			}
		}
	}
	return isuse;
}

uint8_t decode_daikin_custom1_ac(uint8_t* response)
{
	if(custom_ir.number_section_ir != 2) return 0;
	if(custom_ir.size_section_ir[0] != 56) return 0;
	if(custom_ir.size_section_ir[1] != 200) return 0;
	if((custom_ir.logic_ir[0] < 70) || (custom_ir.logic_ir[0] > 170)) return 0;
	if((custom_ir.logic_ir[1] < 170) || (custom_ir.logic_ir[1] > 270)) return 0;
	if((custom_ir.logic_ir[2] < 70) || (custom_ir.logic_ir[2] > 170)) return 0;
	if((custom_ir.logic_ir[3] < 510) || (custom_ir.logic_ir[3] > 610)) return 0;
	if((custom_ir.section_ir[0] < 1400) || (custom_ir.section_ir[0] > 1800)) return 0;
	if((custom_ir.section_ir[1] < 630) || (custom_ir.section_ir[1] > 730)) return 0;
	if((custom_ir.section_ir[2] < 70) || (custom_ir.section_ir[2] > 170)) return 0;
	if((custom_ir.section_ir[3] < 9200) || (custom_ir.section_ir[3] > 9600)) return 0;
	if((custom_ir.section_ir[4] < 1400) || (custom_ir.section_ir[4] > 1800)) return 0;
	if((custom_ir.section_ir[5] < 630) || (custom_ir.section_ir[5] > 730)) return 0;
	if((custom_ir.section_ir[6] < 70) || (custom_ir.section_ir[6] > 170)) return 0;
	if(custom_ir.section_ir[7] != 0) return 0;
	if(custom_ir.result_section_ir[2] & 0x8000) response[0] = 9 + (reserval_byte((custom_ir.result_section_ir[1] >> 48) & 0xFF) >> 1);
	else response[0] = 1;
	response[1] = 2;
	switch((custom_ir.result_section_ir[2] >> 24) & 0xFF)
	{
		case 0xCE:
			response[1] = 2;
			break;
		case 0xC4:
			response[1] = 3;
			break;
		case 0xC6:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch((custom_ir.result_section_ir[1] >> 40) & 0x0F)
	{
		case 0x00:
			response[2] = 1;
			break;
		case 0x08:
			response[2] = 2;
			break;
		case 0x0C:
			response[2] = 5;
			break;
		case 0x0A:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

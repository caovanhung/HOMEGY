#include "thuvien.h"

void ir_init()
{
	rmt_config_t rmt_rx_config = {
        .rmt_mode = RMT_MODE_RX,
        .channel = RMT_CHANNEL_0,
        .gpio_num = GPIO_NUM_15,
        .clk_div = 250,
        .mem_block_num = 4,
        .flags = 0,
        .rx_config = {
            .idle_threshold = 65000,
            .filter_ticks_thresh = 100,
            .filter_en = true,
        }
	};
	rmt_config_t rmt_tx_config = {
	    .rmt_mode = RMT_MODE_TX,
	    .channel = RMT_CHANNEL_4,
	    .gpio_num = GPIO_NUM_13,
	    .clk_div = 250,
	    .mem_block_num = 4,
	    .flags = 0,
	    .tx_config = {
	        .carrier_freq_hz = 38000,
	        .carrier_level = RMT_CARRIER_LEVEL_HIGH,
	        .idle_level = RMT_IDLE_LEVEL_LOW,
	        .carrier_duty_percent = 50,
	        .carrier_en = true,
	        .loop_en = false,
	        .idle_output_en = true,
	    }
	};
	rmt_config(&rmt_rx_config);
    rmt_driver_install(RMT_CHANNEL_0, 2048, 0);
    rmt_get_ringbuf_handle(RMT_CHANNEL_0, &ring_buffer_ir);
    rmt_config(&rmt_tx_config);
    rmt_driver_install(RMT_CHANNEL_4, 0, 0);
}

void send_buffer_ir()
{
	if((items_tx_ir == NULL) || (!number_tx_ir)) return;
	rmt_write_items(RMT_CHANNEL_4, items_tx_ir, number_tx_ir, true);
	free(items_tx_ir);
	items_tx_ir = NULL;
	number_tx_ir = 0;
}

uint8_t send_raw_ir(uint8_t idl, uint8_t idh, uint8_t key)
{
	uint16_t id = idl+256*idh;
	if((memory[id] != 4) || (id < 2) || (id > 20) || (!key)) return 0;
	uint8_t matrix[4] = {0, 0, 0, 0};
	if(!find_raw_key(matrix, idl, idh, key)) return 0;
	uint32_t position;
	uint16_t i;
	uint8_t j;
	uint8_t data[4];
	if(items_tx_ir != NULL)
	{
		free(items_tx_ir);
		items_tx_ir = NULL;
		number_tx_ir = 0;
	}
	items_tx_ir = (rmt_item32_t*)malloc(1024);
	j = 0;
	position = 4096*matrix[0]+512*matrix[1];
	for(i=0; i<512; i+=4)
	{
		esp_partition_read_raw(partition, position+i, &data, 4);
		items_tx_ir[j].level0 = LOGIC0;
		items_tx_ir[j].level1 = LOGIC1;
		items_tx_ir[j].duration0 = 256 * data[0] + data[1];
		items_tx_ir[j].duration1 = 256 * data[2] + data[3];
		j++;
		if((!data[2]) && (!data[3])) break;
	}
	if((matrix[2]) || (matrix[3]))
	{
		position = 4096*matrix[2]+512*matrix[3];
		for(i=0; i<512; i+=4)
		{
			esp_partition_read_raw(partition, position+i, &data, 4);
			items_tx_ir[j].level0 = LOGIC0;
			items_tx_ir[j].level1 = LOGIC1;
			items_tx_ir[j].duration0 = 256 * data[0] + data[1];
			items_tx_ir[j].duration1 = 256 * data[2] + data[3];
			j++;
			if((!data[2]) && (!data[3])) break;
		}
	}
	number_tx_ir = j;
	return 1;
}

uint8_t send_custom_ir(uint8_t idl, uint8_t idh, uint8_t index, uint8_t key)
{
	uint16_t id = idl+256*idh;
	if((memory[id] != 2) || (id < 2) || (!index) || (!key)) return 0;
	uint32_t position = 4096*(idl + 256*idh);
	uint16_t i = 0;
	uint16_t start;
	uint8_t tmpindex;
	uint8_t data;
	while(1)
	{
		tmpindex = 0;
		start = i;
		while(1)
		{
			esp_partition_read_raw(partition, position+i, &data, 1);
			if(data == ',') break;
			tmpindex = 10 * tmpindex + data - 48;
			i++;
		}
		if(tmpindex == index) break;
		while(1)
		{
			esp_partition_read_raw(partition, position+i, &data, 1);
			i++;
			if(data == ';') break;
		}
		esp_partition_read_raw(partition, position+i, &data, 1);
		if(data == '!') break;
	}
	if(tmpindex != index) return 0;
	uint8_t j;
	i = start;
	while(1)
	{
		esp_partition_read_raw(partition, position+i, &data, 1);
		i++;
		if(data == ',') break;
	}
	for(j=0; j<4; j++)
	{
		custom_ir.logic_ir[j] = 0;
		while(1)
		{
			esp_partition_read_raw(partition, position+i, &data, 1);
			i++;
			if(data == ',') break;
			custom_ir.logic_ir[j] = 10 * custom_ir.logic_ir[j] + data - 48;
		}
	}
	esp_partition_read_raw(partition, position+i, &data, 1);
	custom_ir.number_section_ir = data - 48;
	i += 2;
	for(j=0; j<custom_ir.number_section_ir; j++)
	{
		custom_ir.size_section_ir[j] = 0;
		while(1)
		{
			esp_partition_read_raw(partition, position+i, &data, 1);
			i++;
			if(data == ',') break;
			custom_ir.size_section_ir[j] = 10 * custom_ir.size_section_ir[j] + data - 48;
		}
	}
	for(j=0; j<4*custom_ir.number_section_ir; j++)
	{
		custom_ir.section_ir[j] = 0;
		while(1)
		{
			esp_partition_read_raw(partition, position+i, &data, 1);
			i++;
			if(data == ',') break;
			custom_ir.section_ir[j] = 10 * custom_ir.section_ir[j] + data - 48;
		}
	}
	uint8_t tmpkey = 1;
	while(tmpkey != key)
	{
		esp_partition_read_raw(partition, position+i, &data, 1);
		if(data == ';') return 0;
		i++;
		if(data == ',') tmpkey++;
	}
	custom_ir.result_section_ir[0] = 0;
	custom_ir.result_section_ir[1] = 0;
	custom_ir.result_section_ir[2] = 0;
	custom_ir.result_section_ir[3] = 0;
	j = 0;
	while(1)
	{
		esp_partition_read_raw(partition, position+i, &data, 1);
		i++;
		if((data == ',') || (data == ';')) break;
		if((data >= '0') && (data <= '9')) data -= 48;
		else
		{
			if((data >= 'A') && (data <= 'F')) data -= 55;
		}
		if(j < 16) custom_ir.result_section_ir[3] = 16 * custom_ir.result_section_ir[3] + data;
		else
		{
			if(j < 32) custom_ir.result_section_ir[2] = 16 * custom_ir.result_section_ir[2] + data;
			else
			{
				if(j < 48) custom_ir.result_section_ir[1] = 16 * custom_ir.result_section_ir[1] + data;
				else custom_ir.result_section_ir[0] = 16 * custom_ir.result_section_ir[0] + data;
			}
		}
		j++;
	}
	if(custom_ir.result_section_ir[3] == 0) return 0;
	if(items_tx_ir != NULL)
	{
		free(items_tx_ir);
		items_tx_ir = NULL;
		number_tx_ir = 0;
	}
	items_tx_ir = (rmt_item32_t*)malloc(4*custom_ir.size_section_ir[custom_ir.number_section_ir-1] + 8*custom_ir.number_section_ir);
	uint8_t k = 0;
	i = 0;
	for(j=0; j<custom_ir.number_section_ir; j++)
	{
		if(custom_ir.section_ir[4*j] != 0)
		{
			items_tx_ir[i].level0 = LOGIC0;
			items_tx_ir[i].duration0 = custom_ir.section_ir[4*j];
			items_tx_ir[i].level1 = LOGIC1;
			items_tx_ir[i].duration1 = custom_ir.section_ir[4*j+1];
			i++;
		}
		while(k < custom_ir.size_section_ir[j])
		{
			items_tx_ir[i].level0 = LOGIC0;
			items_tx_ir[i].level1 = LOGIC1;
			if(k<64)
			{
				if(custom_ir.result_section_ir[3] & (((uint64_t)1) << (63-k)))
				{
					items_tx_ir[i].duration0 = custom_ir.logic_ir[2];
					items_tx_ir[i].duration1 = custom_ir.logic_ir[3];
				}
				else
				{
					items_tx_ir[i].duration0 = custom_ir.logic_ir[0];
					items_tx_ir[i].duration1 = custom_ir.logic_ir[1];
				}
			}
			else
			{
				if(k < 128)
				{
					if(custom_ir.result_section_ir[2] & (((uint64_t)1) << (127-k)))
					{
						items_tx_ir[i].duration0 = custom_ir.logic_ir[2];
						items_tx_ir[i].duration1 = custom_ir.logic_ir[3];
					}
					else
					{
						items_tx_ir[i].duration0 = custom_ir.logic_ir[0];
						items_tx_ir[i].duration1 = custom_ir.logic_ir[1];
					}
				}
				else
				{
					if(k < 192)
					{
						if(custom_ir.result_section_ir[1] & (((uint64_t)1) << (191-k)))
						{
							items_tx_ir[i].duration0 = custom_ir.logic_ir[2];
							items_tx_ir[i].duration1 = custom_ir.logic_ir[3];
						}
						else
						{
							items_tx_ir[i].duration0 = custom_ir.logic_ir[0];
							items_tx_ir[i].duration1 = custom_ir.logic_ir[1];
						}
					}
					else
					{
						if(custom_ir.result_section_ir[0] & (((uint64_t)1) << (255-k)))
						{
							items_tx_ir[i].duration0 = custom_ir.logic_ir[2];
							items_tx_ir[i].duration1 = custom_ir.logic_ir[3];
						}
						else
						{
							items_tx_ir[i].duration0 = custom_ir.logic_ir[0];
							items_tx_ir[i].duration1 = custom_ir.logic_ir[1];
						}
					}
				}
			}
			i++;
			k++;
		}
		items_tx_ir[i].level0 = LOGIC0;
		items_tx_ir[i].duration0 = custom_ir.section_ir[4*j+2];
		items_tx_ir[i].level1 = LOGIC1;
		items_tx_ir[i].duration1 = custom_ir.section_ir[4*j+3];
		i++;
	}
	number_tx_ir = i;
	return 1;
}

uint8_t send_memory_ir(uint8_t idl, uint8_t idh, uint8_t index, uint8_t key)
{
	uint8_t isok = 0;
	uint16_t id = idl + 256*idh;
	uint8_t tmpkey = key;
	if((id >= 2) && (id <= 20))
	{
		if(memory[id] == 4)
		{
			if(key > 1)
			{
				if(find_type_raw(idl, idh) == 2) key = choose_raw_key(idl, idh, key, 0, 0);
			}
			isok = send_raw_ir(idl, idh, key);
			if(isok)
			{
				if(key != tmpkey) save_status_remote(idl, idh, index, tmpkey, 2, 1, 1);
			}
		}
	}
	else
	{
		if(id > 20)
		{
			if(idl%2) isok = send_custom_ir(idl, idh, index, key);
			else
			{
				if(key == 1) key = 2;
				else
				{
					if((key > 15) && (key < 31)) key -= 13;
				}
				isok = send_custom_ir(idl, idh, index, key);
				if(isok) save_status_remote(idl, idh, index, tmpkey, 2, 1, 1);
			}
		}
	}
	return isok;
}

uint8_t decode_custom_ir(rmt_item32_t* items, uint16_t number)
{
	uint16_t i;
	uint8_t j;
	uint8_t k = 0;
	uint16_t tmp;
	uint16_t duration0[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t duration1[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t number_duration[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	for(i=0; i<number-1; i++)
	{
		for(j=0; j<k; j++)
		{
			if((items[i].duration0 >= duration0[j] - 50) && (items[i].duration0 <= duration0[j] + 50) && (items[i].duration1 >= duration1[j] - 50) && (items[i].duration1 <= duration1[j] + 50))
			{
				number_duration[j]++;
				break;
			}
		}
		if(j == k)
		{
			duration0[k] = items[i].duration0;
			duration1[k] = items[i].duration1;
			number_duration[k] = 1;
			k++;
		}
	}
	j = 1;
	while(j)
	{
		j = 0;
		for(i=0; i<k-1; i++)
		{
			if(number_duration[i] < number_duration[i+1])
			{
				tmp = duration0[i];
				duration0[i] = duration0[i+1];
				duration0[i+1] = tmp;
				tmp = duration1[i];
				duration1[i] = duration1[i+1];
				duration1[i+1] = tmp;
				tmp = number_duration[i];
				number_duration[i] = number_duration[i+1];
				number_duration[i+1] = tmp;
				j = 1;
			}
		}
	}
	if((duration0[1] > duration0[0]-50) && (duration0[1] < duration0[0]+50))
	{
		custom_ir.logic_ir[0] = duration0[0];
		custom_ir.logic_ir[2] = duration0[0];
		if(duration1[0] > duration1[1])
		{
			custom_ir.logic_ir[1] = duration1[1];
			custom_ir.logic_ir[3] = duration1[0];
		}
		else
		{
			custom_ir.logic_ir[1] = duration1[0];
			custom_ir.logic_ir[3] = duration1[1];
		}
	}
	else
	{
		if((duration1[1] > duration1[0]-50) && (duration1[1] < duration1[0]+50))
		{
			if(duration0[0] > duration0[1])
			{
				custom_ir.logic_ir[0] = duration0[1];
				custom_ir.logic_ir[2] = duration0[0];
			}
			else
			{
				custom_ir.logic_ir[0] = duration0[0];
				custom_ir.logic_ir[2] = duration0[1];
			}
			custom_ir.logic_ir[1] = duration1[0];
			custom_ir.logic_ir[3] = duration1[0];
		}
		else
		{
			if((duration1[0] > duration0[0]-50) && (duration1[0] < duration0[0]+50) && (duration1[1] > duration0[1]-50) && (duration1[1] < duration0[1]+50))
			{
				if(duration0[0] > duration0[1])
				{
					custom_ir.logic_ir[0] = duration0[1];
					custom_ir.logic_ir[2] = duration0[0];
					custom_ir.logic_ir[1] = duration1[1];
					custom_ir.logic_ir[3] = duration1[0];
				}
				else
				{
					custom_ir.logic_ir[0] = duration0[0];
					custom_ir.logic_ir[2] = duration0[1];
					custom_ir.logic_ir[1] = duration1[0];
					custom_ir.logic_ir[3] = duration1[1];
				}
			}
			else return 0;
		}
	}
	custom_ir.result_section_ir[0] = 0;
	custom_ir.result_section_ir[1] = 0;
	custom_ir.result_section_ir[2] = 0;
	custom_ir.result_section_ir[3] = 0;
	j = 0;
	k = 0;
	for(i=0; i<10; i++)
	{
		if((items[j].duration0 < custom_ir.logic_ir[0] - 50)
		|| ((items[j].duration0 > custom_ir.logic_ir[0] + 50) && (items[j].duration0 < custom_ir.logic_ir[2] - 50))
		|| (items[j].duration0 > custom_ir.logic_ir[2] + 50)
		|| (items[j].duration1 < custom_ir.logic_ir[1] - 50)
		|| ((items[j].duration1 > custom_ir.logic_ir[1] + 50) && (items[j].duration1 < custom_ir.logic_ir[3] - 50))
		|| (items[j].duration1 > custom_ir.logic_ir[3] + 50))
		{
			custom_ir.section_ir[4*i] = items[j].duration0;
			custom_ir.section_ir[4*i+1] = items[j].duration1;
			j++;
		}
		else
		{
			custom_ir.section_ir[4*i] = 0;
			custom_ir.section_ir[4*i+1] = 0;
		}
		for(;;j++)
		{
			if(j >= number)
			{
				custom_ir.section_ir[4*i+2] = 0;
				custom_ir.section_ir[4*i+3] = 0;
				custom_ir.size_section_ir[i] = k;
				j++;
				break;
			}
			if((items[j].duration0 > custom_ir.logic_ir[0] - 50)
			&& (items[j].duration0 < custom_ir.logic_ir[0] + 50)
			&& (items[j].duration1 > custom_ir.logic_ir[1] - 50)
			&& (items[j].duration1 < custom_ir.logic_ir[1] + 50));
			else
			{
				if((items[j].duration0 > custom_ir.logic_ir[2] - 50)
				&& (items[j].duration0 < custom_ir.logic_ir[2] + 50)
				&& (items[j].duration1 > custom_ir.logic_ir[3] - 50)
				&& (items[j].duration1 < custom_ir.logic_ir[3] + 50))
				{
					if(k < 64) custom_ir.result_section_ir[3] |= ((uint64_t)1) << (63-k);
					else
					{
						if(k < 128) custom_ir.result_section_ir[2] |= ((uint64_t)1) << (127-k);
						else
						{
							if(k < 192) custom_ir.result_section_ir[1] |= ((uint64_t)1) << (191-k);
							else custom_ir.result_section_ir[0] |= ((uint64_t)1) << (255-k);
						}
					}
				}
				else
				{
					custom_ir.section_ir[4*i+2] = items[j].duration0;
					custom_ir.section_ir[4*i+3] = items[j].duration1;
					custom_ir.size_section_ir[i] = k;
					j++;
					break;
				}
			}
			k++;
		}
		if(j >= number) break;
	}
	custom_ir.number_section_ir = i+1;
	return 1;
}

uint8_t find_memory_remote(rmt_item32_t* items, uint16_t number, uint8_t statusdecode)
{
	uint8_t i;
	uint8_t key;
	for(i=0; i<100; i+=4)
	{
		if((params[i] == 1) || (params[i] == 7))
		{
			if((params[i+1]+256*params[i+2] > 20) && (params[i+1] % 2 == 0))
			{
				if(statusdecode)
				{
					load_custom_remote(i);
					key = find_custom_remote(i);
					delete_custom_remote();
				}
			}
			else
			{
				if((params[i+1]+256*params[i+2] > 1) && (find_type_raw(params[i+1], params[i+2]) == 2)) key = find_raw_remote(items, number, i);
				else key = 0;
			}
			if(key)
			{
				if((params[i+1]+256*params[i+2] > 20) && (params[i+1] % 2 == 0))
				{
					if(key == 2) key = 1;
					else key += 13;
					ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 1, params[i+1], params[i+2], params[i+3], key, 2, 17);
					save_status_remote(params[i+1], params[i+2], params[i+3], key, 2, 1, 1);
				}
				else
				{
					if((params[i+1]+256*params[i+2] < 21) && (params[i+1]+256*params[i+2] > 1) && (find_type_raw(params[i+1], params[i+2]) == 2))
					{
						if(key != 1) key = ((key >> 2)&0x0F)+16;
						ble_mesh_send(gateway, 0xE50211, 8, SEND_KEY_CMD, 1, params[i+1], params[i+2], params[i+3], key, 2, 17);
						save_status_remote(params[i+1], params[i+2], params[i+3], key, 2, 1, 1);
					}
				}
				return 1;
			}
		}
	}
	return 0;
}

uint8_t find_custom_remote(uint8_t index)
{	
	if(remote_param_ir == NULL) return 0;
	uint8_t i;
	for(i=0; i<4; i++)
	{
		if((custom_ir.logic_ir[i] > remote_param_ir[i]+50) || (custom_ir.logic_ir[i] < remote_param_ir[i]-50)) return 0;
	}
	uint8_t number_section_ir;
	if(remote_param_ir[4] != custom_ir.number_section_ir) return 0;
	number_section_ir = custom_ir.number_section_ir;
	for(i=0; i<number_section_ir; i++)
	{
		if(remote_param_ir[i+5] != custom_ir.size_section_ir[i]) return 0;
	}
	for(i=0; i<4*number_section_ir; i++)
	{
		if((custom_ir.section_ir[i] > remote_param_ir[i+remote_param_ir[4]+5]+50) || (custom_ir.section_ir[i] < remote_param_ir[i+remote_param_ir[4]+5]-50)) return 0;
	}
	uint64_t one = 1;
	uint8_t error0 = 0;
	uint8_t error1 = 0;
	uint8_t position[16] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
	load_position_remote(params[index+1], params[index+2], params[index+3], position);
	for(i=0; i<16; i++)
	{
		if(position[i] == 255) continue;
		if(position[i] < 64)
		{
			if(remote_code3_ir != NULL)
			{
				if(custom_ir.result_section_ir[3] && remote_code3_ir[1])
				{
					if((custom_ir.result_section_ir[3] & (one << position[i])) ^ (remote_code3_ir[1] & (one << position[i]))) error0 = 1;
					else error1 = 1;
				}
				else return 0;
			}
		}
		else
		{
			if(position[i] < 128)
			{
				if(remote_code2_ir != NULL)
				{
					if(custom_ir.result_section_ir[2] && remote_code2_ir[1])
					{
						if((custom_ir.result_section_ir[2] & (one << (position[i]-64))) ^ (remote_code2_ir[1] & (one << (position[i]-64)))) error0 = 1;
						else error1 = 1;
					}
					else return 0;
				}
			}
			else
			{
				if(position[i] < 192)
				{
					if(remote_code1_ir != NULL)
					{
						if(custom_ir.result_section_ir[1] && remote_code1_ir[1])
						{
							if((custom_ir.result_section_ir[1] & (one << (position[i]-128))) ^ (remote_code1_ir[1] & (one << (position[i]-128)))) error0 = 1;
							else error1 = 1;
						}
						else return 0;
					}
				}
				else
				{
					if(remote_code0_ir != NULL)
					{
						if(custom_ir.result_section_ir[0] && remote_code0_ir[1])
						{
							if((custom_ir.result_section_ir[0] & (one << (position[i]-192))) ^ (remote_code0_ir[1] & (one << (position[i]-192)))) error0 = 1;
							else error1 = 1;
						}
						else return 0;
					}
				}
			}
		}
	}
	if((!error0) && (!error1)) return 0;
	else
	{
		if((!error0) && error1) return 2;
		else
		{
			if(error0 && (!error1)) return 13;
			else
			{
				uint8_t ischange = 0;
				for(i=0; i<16; i++)
				{
					if(position[i] == 255) continue;
					if(position[i] < 64)
					{
						if(custom_ir.result_section_ir[3] && remote_code3_ir[1])
						{
							if(((custom_ir.result_section_ir[3] & (one << position[i])) ^ (remote_code3_ir[1] & (one << position[i]))) == 0)
							{
								ischange = 1;
								position[i] = 255;
							}
						}
					}
					else
					{
						if(position[i] < 128)
						{
							if(custom_ir.result_section_ir[2] && remote_code2_ir[1])
							{
								if(((custom_ir.result_section_ir[2] & (one << (position[i]-64))) ^ (remote_code2_ir[1] & (one << (position[i]-64)))) == 0)
								{
									ischange = 1;
									position[i] = 255;
								}
							}
						}
						else
						{
							if(position[i] < 192)
							{
								if(custom_ir.result_section_ir[1] && remote_code1_ir[1])
								{
									if(((custom_ir.result_section_ir[1] & (one << (position[i]-128))) ^ (remote_code1_ir[1] & (one << (position[i]-128)))) == 0)
									{
										ischange = 1;
										position[i] = 255;
									}
								}
							}
							else
							{
								if(custom_ir.result_section_ir[0] && remote_code0_ir[1])
								{
									if(((custom_ir.result_section_ir[0] & (one << (position[i]-192))) ^ (remote_code0_ir[1] & (one << (position[i]-192)))) == 0)
									{
										ischange = 1;
										position[i] = 255;
									}
								}
							}
						}
					}
				}
				if(ischange) set_position_remote(params[index+1], params[index+2], params[index+3], position);
			}
		}
	}
	return 0;
}

uint8_t find_raw_remote(rmt_item32_t* items, uint16_t number, uint8_t index)
{
	if((memory[params[index+1]+256*params[index+2]] != 4) || (params[index+1]+256*params[index+2]>20) || (params[index+1]+256*params[index+2]<2) || (find_type_raw(params[index+1], params[index+2]) != 2)) return 0;
	uint32_t addr[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t i;
	uint8_t j = 2;
	uint8_t cache[6];
	for(i=0; i<4096; i+=6)
	{
		esp_partition_read_raw(partition, 4096*(params[index+1]+256*params[index+2])+i, cache, 6);
		if(cache[0] == '!') break;
		if(cache[1] > 1)
		{
			addr[j] = 4096*cache[2]+512*cache[3];
			addr[j+1] = 4096*cache[4]+512*cache[5];
			j += 2;
		}
		else
		{
			addr[0] = 4096*cache[2]+512*cache[3];
			addr[1] = 4096*cache[4]+512*cache[5];
		}
	}
	if((!addr[0]) || (!addr[2])) return 0;
	uint16_t sampleduration1;
	uint16_t sampleduration3;
	if(number < 129)
	{
		esp_partition_read_raw(partition, addr[0]+4*(number-1), cache, 4);
		sampleduration1 = 256*cache[2]+cache[3];
		esp_partition_read_raw(partition, addr[2]+4*(number-1), cache, 4);
		sampleduration3 = 256*cache[2]+cache[3];
	}
	else
	{
		esp_partition_read_raw(partition, addr[1]+4*(number-129), cache, 4);
		sampleduration1 = 256*cache[2]+cache[3];
		esp_partition_read_raw(partition, addr[3]+4*(number-129), cache, 4);
		sampleduration3 = 256*cache[2]+cache[3];
	}
	if(sampleduration1 && sampleduration3) return 0;
	uint8_t k = 0;
	uint16_t duration0[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t duration1[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	for(i=0; i<number-1; i++)
	{
		for(j=0; j<k; j++)
		{
			if((items[i].duration0 >= duration0[j] - 50) && (items[i].duration0 <= duration0[j] + 50) && (items[i].duration1 >= duration1[j] - 50) && (items[i].duration1 <= duration1[j] + 50)) break;
		}
		if(j == k)
		{
			duration0[k] = items[i].duration0;
			duration1[k] = items[i].duration1;
			k++;
		}
	}
	uint16_t sampleduration0;
	uint16_t sampleduration2;
	uint8_t l = 0;
	for(i=0; i<number-1; i++)
	{
		if(i < 128)
		{
			esp_partition_read_raw(partition, addr[0]+4*i, cache, 4);
			sampleduration0 = 256*cache[0]+cache[1];
			sampleduration1 = 256*cache[2]+cache[3];
			esp_partition_read_raw(partition, addr[2]+4*i, cache, 4);
			sampleduration2 = 256*cache[0]+cache[1];
			sampleduration3 = 256*cache[2]+cache[3];
		}
		else
		{
			esp_partition_read_raw(partition, addr[1]+4*(i-128), cache, 4);
			sampleduration0 = 256*cache[0]+cache[1];
			sampleduration1 = 256*cache[2]+cache[3];
			esp_partition_read_raw(partition, addr[3]+4*(i-128), cache, 4);
			sampleduration2 = 256*cache[0]+cache[1];
			sampleduration3 = 256*cache[2]+cache[3];
		}
		for(j=0; j<k; j++)
		{
			if(duration0[j])
			{
				if((duration0[j] < sampleduration0+50) && (duration0[j] > sampleduration0-50) && (duration1[j] < sampleduration1+50) && (duration1[j] > sampleduration1-50))
				{
					duration0[j] = 0;
					duration1[j] = 0;
					l++;
				}
				else
				{
					if((duration0[j] < sampleduration2+50) && (duration0[j] > sampleduration2-50) && (duration1[j] < sampleduration3+50) && (duration1[j] > sampleduration3-50))
					{
						duration0[j] = 0;
						duration1[j] = 0;
						l++;
					}
				}
			}
		}
		if(l >= k) break;
	}
	if(l < k) return 0;
	uint8_t ischange = 0;
	uint8_t position[16] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
	load_position_remote(params[index+1], params[index+2], params[index+3], position);
	uint16_t error0 = 0;
	uint16_t error1 = 0;
	for(i=0; i<16; i++)
	{
		if(position[i] >= 255) continue;
		if(position[i] < 128)
		{
			esp_partition_read_raw(partition, addr[0]+4*position[i], cache, 4);
			sampleduration0 = 256*cache[0]+cache[1];
			sampleduration1 = 256*cache[2]+cache[3];
		}
		else
		{
			esp_partition_read_raw(partition, addr[1]+4*(position[i]-128), cache, 4);
			sampleduration0 = 256*cache[0]+cache[1];
			sampleduration1 = 256*cache[2]+cache[3];
		}
		if((items[position[i]].duration0 < sampleduration0+50) && (items[position[i]].duration0 > sampleduration0-50) && (items[position[i]].duration1 < sampleduration1+50) && (items[position[i]].duration1 > sampleduration1-50)) error1 = 1;
		else error0 = 1;
	}
	if((!error0) && (!error1)) return 0;
	else
	{
		if((!error0) && error1) return 1;
		else
		{
			if(error0 && (!error1)) return 120;
			else
			{
				for(i=0; i<16; i++)
				{
					if(position[i] >= 255) continue;
					if(position[i] < 128) esp_partition_read_raw(partition, addr[0]+4*position[i], cache, 4);
					else esp_partition_read_raw(partition, addr[1]+4*(position[i]-128), cache, 4);
					sampleduration0 = 256*cache[0]+cache[1];
					sampleduration1 = 256*cache[2]+cache[3];
					if((items[position[i]].duration0 < sampleduration0+50) && (items[position[i]].duration0 > sampleduration0-50) && (items[position[i]].duration1 < sampleduration1+50) && (items[position[i]].duration1 > sampleduration1-50))
					{
						position[i] = 255;
						ischange = 1;
					}
				}
				if(ischange) set_position_remote(params[index+1], params[index+2], params[index+3], position);
			}
		}
	}
	return 0;
}

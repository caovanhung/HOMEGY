#include "thuvien.h"
#include "aes.h"

uint8_t dev_uuid[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0xAA, 0xAA, 000, 0x01, 0xAA, 0x03, 0x01};
uint8_t queue[240];
uint8_t indexqueue;
volatile uint8_t issenddone;

esp_ble_mesh_cfg_srv_t config_server = {
    .relay = ESP_BLE_MESH_RELAY_ENABLED,
    .beacon = ESP_BLE_MESH_BEACON_ENABLED,
    .friend_state = ESP_BLE_MESH_FRIEND_DISABLED,
    .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_DISABLED,
    .default_ttl = 5,
    .net_transmit = ESP_BLE_MESH_TRANSMIT(3, 20),
    .relay_retransmit = ESP_BLE_MESH_TRANSMIT(3, 20),
};

esp_ble_mesh_model_op_t vnd_op[] = {
	ESP_BLE_MESH_MODEL_OP(0xE00211, 0),
    ESP_BLE_MESH_MODEL_OP(0xE40211, 0),
    ESP_BLE_MESH_MODEL_OP_END,
};

esp_ble_mesh_model_op_t sig_op[] = {
	ESP_BLE_MESH_MODEL_OP(ESP_BLE_MESH_MODEL_OP_SCENE_RECALL, 0),
	ESP_BLE_MESH_MODEL_OP(ESP_BLE_MESH_MODEL_OP_SCENE_RECALL_UNACK, 0),
	ESP_BLE_MESH_MODEL_OP(ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET, 0),
	ESP_BLE_MESH_MODEL_OP_END,
};

esp_ble_mesh_model_t sig_models[] = {
    ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
    ESP_BLE_MESH_SIG_MODEL(ESP_BLE_MESH_MODEL_ID_SCENE_SRV, sig_op, NULL, NULL),
    ESP_BLE_MESH_SIG_MODEL(ESP_BLE_MESH_MODEL_ID_GEN_ONOFF_SRV, sig_op, NULL, NULL),
};

esp_ble_mesh_model_t vnd_models[] = {
    ESP_BLE_MESH_VENDOR_MODEL(0x0211, 0x01, vnd_op, NULL, NULL),
};

esp_ble_mesh_elem_t elements[] = {
    ESP_BLE_MESH_ELEMENT(0, sig_models, vnd_models),
};

esp_ble_mesh_comp_t composition = {
    .cid = 0x02AA,
    .pid = 0x0301,
    .vid = 0x0001,
    .elements = elements,
    .element_count = 1,
};

esp_ble_mesh_prov_t provision = {
    .uuid = dev_uuid,
};

uint8_t check_active(uint8_t ref[])
{
	uint8_t i;
	const uint8_t key[16] = {0x48,0x47,0x59,0x62,0x6c,0x75,0x65,0x74,0x6f,0x6f,0x74,0x68,0x76,0x65,0x72,0x32};
	uint8_t code[16] = {0x68,0x67,0x79,0x73,0x6d,0x61,0x72,0x74,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
	uint16_t addr = esp_ble_mesh_get_primary_element_address();
	struct AES_ctx ctx;
	AES_init_ctx(&ctx, (const uint8_t*)key);
	for(i=0; i<6; i++) code[i+8] = dev_uuid[7-i];
	code[14] = addr % 256;
	code[15] = addr / 256;
	AES_ECB_encrypt(&ctx, (uint8_t*)code);
	for(i=2; i<8; i++)
	{
		if(ref[i] != code[i+8]) return 0;
	}
	return 1;
}

void set_queue(uint8_t tmpqueue[])
{
	uint8_t i;
	if(indexqueue > 239)
	{
		for(i=0; i<232; i++) queue[i] = queue[i+8];
		indexqueue = 232;
	}
	for(i=0; i<8; i++) queue[indexqueue+i] = tmpqueue[i];
	indexqueue += 8;
}

void get_queue()
{
	if(indexqueue < 8)
	{
		request[0] = NONE_CMD;
		return;
	}
	uint8_t i;
	for(i=0; i<8; i++) request[i] = queue[i];
	for(i=0; i<indexqueue-8; i++) queue[i] = queue[i+8];
	for(i=indexqueue-8; i<indexqueue; i++) queue[i] = 0;
	indexqueue -= 8;
}

void update_queue(uint8_t idl, uint8_t idh)
{
	uint8_t i;
	for(i=0; i<indexqueue; i+=8)
	{
		if((queue[i] == RECV_LOCAL_CMD) && (queue[i+1] == idl) && (queue[i+2] == idh)) queue[i] = NONE_CMD;
	}
}

void ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event, esp_ble_mesh_model_cb_param_t *param)
{
	if(event == ESP_BLE_MESH_MODEL_OPERATION_EVT)
	{
		uint8_t i;
		uint8_t tmpqueue[100];
		for(i=0; i<100; i++) tmpqueue[i] = 0;
		for(i=0; i<param->model_operation.length; i++) tmpqueue[i] = param->model_operation.msg[i];
		tmpqueue[99] = param->model_operation.length;
		if (param->model_operation.opcode == 0xE00211)
		{
			if((tmpqueue[0] == 2) && (!tmpqueue[1]))
			{
				gateway = 256*tmpqueue[3] + tmpqueue[2];
				indexresponse = 256*tmpqueue[5] + tmpqueue[4];
				save_memory_system();
				ble_mesh_send(param->model_operation.ctx->addr, 0xE10211, 4, 2, 0, tmpqueue[2], tmpqueue[3], 0, 0, 0, 0);
			}
			else
			{
				if((tmpqueue[0] == 3) && (!tmpqueue[1]))
				{
					if(check_active(tmpqueue))
					{
						if(lockdevice == 2)
						{
							lockdevice = 0;
							save_memory_system();
						}
						ble_mesh_send(param->model_operation.ctx->addr, 0xE10211, 8, 3, 0, 0, 1, 0, 0, 0, 0);
					}
					else ble_mesh_send(param->model_operation.ctx->addr, 0xE10211, 8, 3, 0, 0xFF, 0xFE, 0xFF, 0xFE, 0xFF, 0xFE);
				}
				else
				{
					if((tmpqueue[0] == 5) && (!tmpqueue[1]))
					{
						lockdevice = tmpqueue[2];
						save_memory_system();
						ble_mesh_send(gateway, 0xE10211, 3, 5, 0, lockdevice, 0, 0, 0, 0, 0);
					}
					else
					{
						if((tmpqueue[0] == 192) && (!tmpqueue[1]))
						{
							if(lockdevice != 1)
							{
								if(tmpqueue[2] == 1) lockdevice = 2;
								else lockdevice = 0;
								save_memory_system();
							}
							ble_mesh_send(gateway, 0xE10211, 3, 192, 0, lockdevice, 0, 0, 0, 0, 0);
						}
						else
						{
							if((tmpqueue[0] == 64) && (!tmpqueue[1])) set_queue(tmpqueue);
							else
							{
								if((tmpqueue[0] == 112) && (!tmpqueue[1])) set_status_script(tmpqueue[2], tmpqueue[3], tmpqueue[4]);
								else
								{
									if((tmpqueue[0] == 241) && (!tmpqueue[1]))
									{
										if(!timewaittingresponse) timewaittingresponse = 10*indexresponse;
									}
									else
									{
										if((tmpqueue[0] == 242) && (!tmpqueue[1])) set_queue(tmpqueue);
										else
										{
											if((tmpqueue[0] == 243) && (!tmpqueue[1])) set_queue(tmpqueue);
											else
											{
												if((tmpqueue[0] == 244) && (!tmpqueue[1]))
												{
													ble_mesh_send(param->model_operation.ctx->addr, 0xE10211, 2, 244, 0, 0, 0, 0, 0, 0, 0);
													esp_ble_mesh_node_local_reset();
													esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
        }
        else
        {
        	if (param->model_operation.opcode == 0xE40211)
			{
				if(lockdevice == 0)
				{
					if((tmpqueue[0] > 1) && (tmpqueue[0] < 20))
					{
						if(tmpqueue[0] == RECV_WIFI_CMD)
						{
							ble_mesh_send(gateway, 0xE50211, 2, RECV_WIFI_CMD, save_wifi(tmpqueue), 0, 0, 0, 0, 0, 0);
							tmpqueue[0] = RECV_VOICE_CMD;
							tmpqueue[1]= 255;
							tmpqueue[2] = 0;
							uart_write_bytes(UART_NUM_0, tmpqueue, 3);
							gpio_set_level(LED, 1);
    						vTaskDelay(100);
							gpio_set_level(LED, 0);
						}
						else set_queue(tmpqueue);
					}
				}
	        }
	        else
	        {
	        	if ((param->model_operation.opcode == ESP_BLE_MESH_MODEL_OP_SCENE_RECALL_UNACK) || (param->model_operation.opcode == ESP_BLE_MESH_MODEL_OP_SCENE_RECALL))
				{
					if(lockdevice != 1)
					{
						tmpqueue[2] = tmpqueue[1];
						tmpqueue[1] = tmpqueue[0];
						tmpqueue[0] = RECV_LOCAL_CMD;
						tmpqueue[3] = param->model_operation.ctx->addr % 256;
						tmpqueue[4] = param->model_operation.ctx->addr / 256;
						set_queue(tmpqueue);
					}
		        }
		        else
		        {
		        	if (param->model_operation.opcode == ESP_BLE_MESH_MODEL_OP_GEN_ONOFF_GET) ble_mesh_send(gateway, 0x8204, 3, 0x82, 0x01, 1, 0, 0, 0, 0, 0);
				}
			}
        }
	}
	else
	{
		if(event == ESP_BLE_MESH_MODEL_SEND_COMP_EVT) issenddone = 1;
	}
}

void ble_mesh_send(uint16_t addr, uint32_t opcode, uint16_t length, uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7)
{
	if(((x0 != 5) || (x0 != 192)) && (x1 != 0))
	{
		if(lockdevice == 1) return;
	}
	esp_ble_mesh_msg_ctx_t client_addr = {
		.addr = addr,
		.net_idx = 0,
		.app_idx = 0,
		.send_ttl = 10,
	};
	uint8_t response[8] = {x0, x1, x2, x3, x4, x5, x6, x7};
	uint8_t i = 0;
	issenddone = 0;
	if((opcode == 0x8242) || (opcode == 0x8243) || (opcode == 0x8245)) esp_ble_mesh_server_model_send_msg(&sig_models[1], &client_addr, opcode, length, (uint8_t*)response);
	else
	{
		if(opcode == 0x8204) esp_ble_mesh_server_model_send_msg(&sig_models[2], &client_addr, opcode, length, (uint8_t*)response);
		else esp_ble_mesh_server_model_send_msg(&vnd_models[0], &client_addr, opcode, length, (uint8_t*)response);
	}
	while((issenddone == 0) && (i < 200))
	{
		i++;
		vTaskDelay(5);
	}
}

void ble_debug_send(uint16_t addr, uint32_t opcode, uint16_t length, uint8_t* debug)
{
	esp_ble_mesh_msg_ctx_t client_addr = {
		.addr = addr,
		.net_idx = 0,
		.app_idx = 0,
		.send_ttl = 5,
	};
	uint8_t i = 0;
	issenddone = 0;
	esp_ble_mesh_server_model_send_msg(&vnd_models[0], &client_addr, opcode, length, debug);
	while((issenddone == 0) && (i < 200))
	{
		i++;
		vTaskDelay(5);
	}
}

void ble_mesh_init()
{
	uint8_t i;
	uint8_t mac[6];
	bluetooth_init();
	ble_mesh_get_dev_uuid(dev_uuid);
	for(i=0; i<6; i++) mac[i] = dev_uuid[7-i];
	for(i=0; i<6; i++) dev_uuid[i+2] = mac[i];
	for(i=0; i<160; i++) queue[i] = 0;
	indexqueue = 0;
	esp_ble_mesh_register_custom_model_callback(ble_mesh_custom_model_cb);
    esp_ble_mesh_init(&provision, &composition);
	if(!esp_ble_mesh_node_is_provisioned()) esp_ble_mesh_node_prov_enable(ESP_BLE_MESH_PROV_ADV | ESP_BLE_MESH_PROV_GATT);
	else reset_relay_list(gateway);
}

void reset_relay_list(uint16_t addr)
{
	uint8_t i;
	for (i=0; i<CONFIG_BLE_MESH_CRPL; i++)
	{
		if(bt_mesh.rpl[i].src == addr) bt_mesh.rpl[i].seq = 0;
	}
}

void remove_relay_list(uint16_t addr)
{
    if(addr == gateway) return;
	uint8_t count = count_relay_list();
	if(bt_mesh.rpl[count-1].src == addr)
	{
		bt_mesh.rpl[count-1].src = 0;
		bt_mesh.rpl[count-1].seq = 0;
		char name[16] = {'\0'};
		sprintf(name, "mesh/rpl/%04x", addr);
    	bt_mesh_erase_core_settings(name);
    	bt_mesh_remove_core_settings_item("mesh/rpl", addr);
	}
}

uint8_t count_relay_list()
{
	uint8_t i;
	for (i=0; i<CONFIG_BLE_MESH_CRPL; i++)
	{
		if(bt_mesh.rpl[i].src == 0) break;
	}
	return i;
}

void print_relay_list()
{
	uint8_t i;
	for (i = 0; i < CONFIG_BLE_MESH_CRPL; i++)
	{
		uart_send_hex(bt_mesh.rpl[i].src);
		uart_write_bytes(UART_NUM_0, " ", 1);
		uart_send_hex(bt_mesh.rpl[i].seq);
		uart_write_bytes(UART_NUM_0, " - ", 3);
	}
}

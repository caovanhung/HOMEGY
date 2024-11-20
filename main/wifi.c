#include "thuvien.h"

uint8_t isconnectwifi = 0;
uint16_t count;
uint8_t statusled;
uint32_t sizebin;
uint8_t isotadone;

void wifi_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP)) isconnectwifi = 1;
    else
    {
    	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) esp_wifi_connect();
	}
}

uint8_t wifi_init()
{
	uint8_t i = 0;
	wifi_config_t wifi_config = {
		.sta = {
			.ssid = "",
			.password = "",
		},
	};
	if((ssid[0] != '\0') && (password[0] != '\0'))
	{
		for(i=0; i<32; i++)
		{
			wifi_config.sta.ssid[i] = ssid[i];
			if(ssid[i] == '\0') break;
		}
		for(i=0; i<64; i++)
		{
			wifi_config.sta.password[i] = password[i];
			if(password[i] == '\0') break;
		}
	}
	else return 0;
	uint8_t err = 1;
	esp_event_handler_instance_t instance_get_ip;
	esp_event_handler_instance_t instance_wifi_id;
    esp_netif_init();
	esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_init_config);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_handler, NULL, &instance_get_ip);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_handler, NULL, &instance_wifi_id);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();
    esp_wifi_connect();
    i = 0;
    while(!isconnectwifi)
	{
		if(i > 30)
		{
			err = 0;
			break;
		}
		i++;
		vTaskDelay(500);
	}
	esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_get_ip);
	esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_wifi_id);
	esp_event_loop_delete_default();
	return err;
}

esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt)
{
    if(evt->event_id == HTTP_EVENT_ON_DATA)
	{
		esp_partition_write_raw(ota_partition, sizebin, (char*)(evt->data), evt->data_len);
		sizebin += evt->data_len;
	}
	else
	{
		if(evt->event_id == HTTP_EVENT_ON_FINISH) isotadone = 1;
	}
	if(count%50 == 0)
	{
		gpio_set_level(LED, statusled);
		statusled = !statusled;
	}
	count++;
    return ESP_OK;
}

uint8_t get_wifi_data(uint8_t index)
{
	const char* const cert =
	"-----BEGIN CERTIFICATE-----\n"
	"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\n"
	"-----END CERTIFICATE-----";
	uint16_t i;
	char server[72] = "https://firebasestorage.googleapis.com/v0/b/homegy-75584.appspot.com/o/";
	char token[18] = "?alt=media&token=";
	char *url = (char*)malloc(256);
	for(i=0; i<71; i++) url[i] = server[i];
	if((index > 0) && (index < 4)) esp_partition_read_raw(config_partition, 714, &url[71], 10);
	else
	{
		if(index == 4) esp_partition_read_raw(config_partition, 724, &url[71], 10);
	}
	url[81] = '\0';
	esp_http_client_config_t http_config = {
	    .url = url,
	    .method = HTTP_METHOD_GET,
	    .cert_pem = cert,
	    .event_handler = client_event_get_handler,
	};
	esp_partition_erase_range(ota_partition, 0, 4096);
	esp_http_client_handle_t client = esp_http_client_init(&http_config);
	sizebin = 0;
	isotadone = 0;
	count = 0;
	statusled = 0;
	esp_http_client_perform(client);
	i = 0;
    while(!isotadone)
	{
		vTaskDelay(500);
		i++;
		if(i >= 30) return 0;
	}
    if(sizebin < 250) return 0;
    for(i=0; i<17; i++) url[i+81] = token[i];
    uint8_t data;
    uint16_t start;
    uint16_t end;
	for(i=sizebin-1; i>0; i--)
	{
		esp_partition_read_raw(ota_partition, i, &data, 1);
		if(data == '"') break;
	}
	if(data != '"') return 0;
	end = i;
	for(i=end-1; i>0; i--)
	{
		esp_partition_read_raw(ota_partition, i, &data, 1);
		if(data == '"') break;
	}
	if(data != '"') return 0;
	start = i+1;
	esp_partition_read_raw(ota_partition, start, &url[98], end-start);
	url[98+end-start] = '\0';
	if(index == 1) esp_partition_erase_range(ota_partition, 0, 1572864);
	else
	{
		if(index == 2) esp_partition_erase_range(ota_partition, 0, 4096);
		else
		{
			if((index == 3) || (index == 4)) esp_partition_erase_range(ota_partition, 0, 524288);
		}
	}
	esp_http_client_set_url(client, url);
	sizebin = 0;
	isotadone = 0;
	count = 0;
	statusled = 0;
	esp_http_client_perform(client);
    esp_http_client_cleanup(client);
    i = 0;
    while(!isotadone)
	{
		vTaskDelay(500);
		i++;
		if(i >= 120) return 0;
	}
    if(sizebin < 250) return 0;
    return 1;
}

uint8_t ota_firmware()
{
	uint32_t i;
	uint8_t data[1024];
	esp_ota_handle_t out_handle;
	const esp_partition_t *partition_running = esp_ota_get_running_partition();
    const esp_partition_t *partition_update = esp_ota_get_next_update_partition(partition_running);
    esp_ota_begin(partition_update, 0, &out_handle);
    for(i=0; i<sizebin; i+=1024)
    {
    	esp_partition_read_raw(ota_partition, i, &data, 1024);
    	esp_ota_write_with_offset(out_handle, &data, 1024, i);
	}
    if(esp_ota_end(out_handle) == ESP_OK)
	{
		esp_ota_set_boot_partition(partition_update);
		return 1;
	}
    else return 0;
}

uint8_t ota_data()
{
	uint8_t data;
    uint16_t i;
    uint16_t j = 0;
    uint32_t position = 0;
    for(i=0; i<sizebin; i++)
    {
    	esp_partition_read_raw(ota_partition, i, &data, 1);
		if(data < 32) break;
    	position = 10*position + data - 48;
	}
	memory[position] = 2;
	position *= 4096;
	esp_partition_erase_range(partition, position, 4096);
	i++;
	for(; i<sizebin; i++)
	{
		esp_partition_read_raw(ota_partition, i, &data, 1);
		if(data > 31)
		{
			esp_partition_write_raw(partition, position+j, &data, 1);
			j++;
		}
	}
    esp_partition_write_raw(partition, position+j, "!", 1);
    return 1;
}

uint8_t ota_voice()
{
	uint8_t i;
	uint8_t bytewrite[134];
	uint8_t byteread[7];
	size_t length_buffer_uart;
	uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
	};
	gpio_reset_pin(GPIO_NUM_16);
	gpio_reset_pin(GPIO_NUM_17);
	uart_param_config(UART_NUM_2, &uart_config);
    uart_driver_install(UART_NUM_2, 256, 0, 0, NULL, 0);
    uart_set_pin(UART_NUM_2, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    gpio_set_level(PIN_BOOT, 0);
    vTaskDelay(500);
    gpio_set_level(PIN_RES, 0);
    vTaskDelay(500);
    gpio_set_level(PIN_RES, 1);
	vTaskDelay(500);
	bytewrite[0] = 0;
	i = 0;
	while(1)
	{
		uart_write_bytes(UART_NUM_2, bytewrite, 1);
		vTaskDelay(10);
		uart_get_buffered_data_len(UART_NUM_2, &length_buffer_uart);
		if(length_buffer_uart >= 1)
		{
			uart_read_bytes(UART_NUM_2, byteread, length_buffer_uart, 100);
			if(byteread[0] == 0) break;
		}
		i++;
		if(i == 250) return 0;
	}
	bytewrite[0] = 0x55;
	uart_write_bytes(UART_NUM_2, bytewrite, 1);
	i = 0;
	while(1)
	{
		vTaskDelay(10);
		uart_get_buffered_data_len(UART_NUM_2, &length_buffer_uart);
		if(length_buffer_uart >= 1)
		{
			uart_read_bytes(UART_NUM_2, byteread, length_buffer_uart, 100);
			break;
		}
		i++;
		if(i == 250) return 0;
	}
	if(byteread[0] != 0xC3) return 0;
	bytewrite[0] = 0x01;
    bytewrite[1] = 0x00;
    bytewrite[2] = 0x01;
    bytewrite[3] = 0x00;
    bytewrite[4] = 0xFF;
    bytewrite[5] = 0x03;
    uart_write_bytes(UART_NUM_2, bytewrite, 6);
    i = 0;
    while(1)
	{
		vTaskDelay(10);
		uart_get_buffered_data_len(UART_NUM_2, &length_buffer_uart);
		if(length_buffer_uart >= 7)
		{
			uart_read_bytes(UART_NUM_2, byteread, length_buffer_uart, 100);
			break;
		}
		i++;
		if(i == 250) return 0;
	}
	if ((byteread[3]  != 0) || (byteread[4] != 0)) return 0;
	bytewrite[0] = 0x01;
    bytewrite[1] = 0x00;
    bytewrite[2] = 0x05;
    bytewrite[3] = 0x34;
    bytewrite[4] = 0x00;
    bytewrite[5] = 0x03;
    bytewrite[6] = 0xE8;
    bytewrite[7] = 0x00;
    bytewrite[8] = 0xDC;
    bytewrite[9] = 0x03;
    uart_write_bytes(UART_NUM_2, bytewrite, 10);
    i = 0;
    while(1)
	{
		vTaskDelay(10);
		uart_get_buffered_data_len(UART_NUM_2, &length_buffer_uart);
		if(length_buffer_uart >= 7)
		{
			uart_read_bytes(UART_NUM_2, byteread, length_buffer_uart, 100);
			break;
		}
		i++;
		if(i == 250) return 0;
	}
	if ((byteread[3]  != 52) || (byteread[4] != 0)) return 0;
	uart_config.baud_rate = 256000;
	uart_param_config(UART_NUM_2, &uart_config);
	bytewrite[0] = 0x01;
    bytewrite[1] = 0x00;
    bytewrite[2] = 0x09;
    bytewrite[3] = 0x12;
    bytewrite[4] = 0x00;
    bytewrite[5] = 0x00;
    bytewrite[6] = 0x00;
    bytewrite[7] = 0x00;
    bytewrite[8] = 0x00;
    bytewrite[9] = 0x07;
    bytewrite[10] = 0x7F;
    bytewrite[11] = 0xFF;
    bytewrite[12] = 0x60;
    bytewrite[13] = 0x03;
    uart_write_bytes(UART_NUM_2, bytewrite, 14);
    i = 0;
    while(1)
	{
		vTaskDelay(10);
		uart_get_buffered_data_len(UART_NUM_2, &length_buffer_uart);
		if(length_buffer_uart >= 7)
		{
			uart_read_bytes(UART_NUM_2, byteread, length_buffer_uart, 100);
			break;
		}
		i++;
		if(i == 250) return 0;
	}
	if ((byteread[3] != 18) || (byteread[4] != 0)) return 0;
	bytewrite[0] = 0x01;
    bytewrite[1] = 0x00;
    bytewrite[2] = 0x09;
    bytewrite[3] = 0x13;
    bytewrite[4] = 0x00;
    bytewrite[5] = 0x00;
    bytewrite[6] = 0x00;
    bytewrite[7] = 0x00;
    bytewrite[8] = 0x00;
    bytewrite[9] = 0x07;
    bytewrite[10] = 0x7F;
    bytewrite[11] = 0xFF;
    bytewrite[12] = 0x5F;
    bytewrite[13] = 0x03;
	uart_write_bytes(UART_NUM_2, bytewrite, 14);
	i = 0;
    while(1)
	{
		vTaskDelay(10);
		uart_get_buffered_data_len(UART_NUM_2, &length_buffer_uart);
		if(length_buffer_uart >= 7)
		{
			uart_read_bytes(UART_NUM_2, byteread, length_buffer_uart, 100);
			break;
		}
		i++;
		if(i == 250) return 0;
	}
	if ((byteread[3] != 19) || (byteread[4] != 0)) return 0;
	bytewrite[0] = 0x81;
    bytewrite[1] = 0x00;
    bytewrite[2] = 0x81;
    bytewrite[3] = 0x13;
    bytewrite[133] = 0x03;
    uint32_t j = 0;
    count = 0;
    statusled = 0;
    esp_partition_read_raw(ota_partition, 0, &bytewrite[4], 128);
    uint32_t sum = 0;
    for (i = 1; i < 132; i++) sum += bytewrite[i];
    bytewrite[132] = 256 - (sum & 0xFF);
    while(j<sizebin)
    {
	    uart_write_bytes(UART_NUM_2, bytewrite, 134);
	    j += 128;
	    if(j < sizebin)
	    {
			esp_partition_read_raw(ota_partition, j, &bytewrite[4], 128);
	    	sum = 0;
	    	for (i = 1; i < 132; i++) sum += bytewrite[i];
	    	bytewrite[132] = 256 - (sum & 0xFF);
		}
		else vTaskDelay(1);
	    i = 0;
	    while(1)
		{
			uart_get_buffered_data_len(UART_NUM_2, &length_buffer_uart);
			if(length_buffer_uart >= 7)
			{
				uart_read_bytes(UART_NUM_2, byteread, length_buffer_uart, 100);
				break;
			}
			vTaskDelay(1);
			i++;
			if(i > 250) return 0;
		}
		if ((byteread[3] != 19) || (byteread[4] != 0)) return 0;
		if(count%100 == 0)
		{
			gpio_set_level(LED, statusled);
			statusled = !statusled;
		}
		count++;
	}
	bytewrite[0] = 0x81;
    bytewrite[1] = 0x00;
    bytewrite[2] = 0x02;
    bytewrite[3] = 0x13;
    bytewrite[4] = 0x00;
    bytewrite[5] = 0xEB;
    bytewrite[6] = 0x03;
    uart_write_bytes(UART_NUM_2, bytewrite, 7);
    vTaskDelay(500);
    gpio_set_level(PIN_BOOT, 1);
    vTaskDelay(500);
    gpio_set_level(PIN_RES, 0);
    vTaskDelay(500);
    gpio_set_level(PIN_RES, 1);
    vTaskDelay(500);
    return 1;
}

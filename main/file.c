#include "thuvien.h"



/*
Hàm clear_memory_system() này có nhiệm vụ làm sạch một số bộ nhớ và các biến trong hệ thống. Sau đây là giải thích chi tiết từng phần của hàm:
Vấn đề có thể xảy ra
Ghi dữ liệu "!" vào bộ nhớ:
	Khi ghi dữ liệu "!" vào bộ nhớ, hàm esp_partition_write_raw chỉ ghi 1 byte. Điều này có thể gây vấn đề nếu yêu cầu ghi lớn hơn 1 byte hoặc nếu bộ nhớ đã bị xóa trước đó. Bạn cần chắc chắn rằng việc ghi lại 1 byte là phù hợp với yêu cầu ứng dụng.
Xóa phân vùng mà không kiểm tra xem phân vùng đó có hợp lệ không:
	Nếu biến partition không được khởi tạo đúng cách (hoặc không có phân vùng nào được chỉ định), việc xóa và ghi dữ liệu vào phân vùng có thể gây lỗi. Bạn cần kiểm tra tính hợp lệ của partition trước khi thực hiện thao tác này.
Sử dụng vùng bộ nhớ cố định:
	Việc sử dụng 4096*i có thể gây xung đột nếu không tính toán chính xác phạm vi bộ nhớ cần thao tác. Điều này cần phải kiểm tra kỹ để đảm bảo không gây ghi đè lên các dữ liệu quan trọng.

Cải tiến:
Kiểm tra tính hợp lệ của phân vùng: Trước khi thao tác với các phân vùng bộ nhớ, bạn cần kiểm tra xem các phân vùng đó đã được khởi tạo và có sẵn không.
Ghi dữ liệu nhiều byte thay vì chỉ 1 byte: Nếu cần ghi nhiều byte vào bộ nhớ sau khi xóa, bạn nên ghi dữ liệu lớn hơn thay vì chỉ ghi 1 byte. Điều này giúp tránh các vấn đề khi cần ghi thông tin lớn hơn.
Giới hạn phạm vi bộ nhớ: Đảm bảo rằng bạn không ghi đè lên các vùng bộ nhớ quan trọng, bằng cách tính toán cẩn thận phạm vi bộ nhớ mà bạn cần thao tác.
Cải tiến logic xóa dữ liệu: Thực hiện xóa và ghi một cách có kiểm soát để tránh sự cố với các phân vùng bộ nhớ.


*/
void clear_memory_system()
{
	uint16_t i;

	// Xóa mảng memory[512] và gán giá trị 1
    for(i = 0; i < 512; i++) {
        memory[i] = 1;
    }

    // Đặt lại các trạng thái
    lockdevice = 0;
    isota = 0;

    // Xóa mảng params[100] và gán giá trị 0
    for(i = 0; i < 100; i++) {
        params[i] = 0;
    }

    // Xóa mảng ssid[32] và password[64]
    for(i = 0; i < 32; i++) {
        ssid[i] = '\0';
    }
    for(i = 0; i < 64; i++) {
        password[i] = '\0';
    }

    // Đặt lại giá trị wakeupvoice
    wakeupvoice = 100;

   // Kiểm tra tính hợp lệ của phân vùng partition trước khi thao tác
    if (partition != NULL) {
        // Xóa và ghi lại các vùng bộ nhớ trong phân vùng partition
        for(i = 0; i < 21; i++) {
            // Xóa 4096 bytes mỗi lần
            esp_partition_erase_range(partition, 4096 * i, 4096);

            // Ghi dữ liệu vào phân vùng sau khi xóa (ví dụ: ghi ký tự '!' vào mỗi vùng)
            const char data_to_write[] = "!";
            if (esp_partition_write_raw(partition, 4096 * i, data_to_write, sizeof(data_to_write) - 1) != ESP_OK) {
				// ESP_LOGE(LOG, "[clear_memory_system]Lỗi khi ghi vào phân vùng bộ nhớ ");
            }
        }
    } else {
		// ESP_LOGE(LOG, "[clear_memory_system]Partition không hợp lệ. ");
    }
}


/*
Hàm load_memory_system() được sử dụng để đọc dữ liệu từ phân vùng bộ nhớ (config_partition) và nạp chúng vào các biến toàn cục. 
Mặc dù về cơ bản nó có thể thực hiện đúng mục đích, nhưng có một số điểm có thể cải tiến và cần lưu ý để tăng tính ổn định, hiệu quả và bảo mật.

*/
void load_memory_system() {
    uint8_t array[2];
    
    // Đọc các đoạn bộ nhớ vào các mảng
    if (esp_partition_read_raw(config_partition, 0, &memory, 512) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc memory\n");
    }
    
    if (esp_partition_read_raw(config_partition, 512, &lockdevice, 1) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc lockdevice\n");
    }
    
    if (esp_partition_read_raw(config_partition, 513, &isota, 1) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc isota\n");
    }
    
    if (esp_partition_read_raw(config_partition, 514, &array, 2) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc gateway\n");
    }
    gateway = 256 * array[0] + array[1];

    if (esp_partition_read_raw(config_partition, 516, &params, 100) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc params\n");
    }

    if (esp_partition_read_raw(config_partition, 616, &ssid, 32) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc ssid\n");
    }

    if (esp_partition_read_raw(config_partition, 648, &password, 64) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc password\n");
    }

    if (esp_partition_read_raw(config_partition, 712, &array, 2) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc wakeupvoice\n");
    }
    wakeupvoice = 256 * array[0] + array[1];

    if (esp_partition_read_raw(config_partition, 734, &array, 2) != ESP_OK) {
        // ESP_LOGE(LOG,"[load_memory_system]Lỗi khi đọc indexresponse\n");
    }
    indexresponse = 256 * array[0] + array[1];
    
    if (!indexresponse) {
        indexresponse = 1;
    }
}

/*
Hàm save_memory_system() thực hiện việc lưu các giá trị vào phân vùng bộ nhớ config_partition.

*/
void save_memory_system()
{
	uint8_t array[2];
	esp_partition_erase_range(config_partition, 0, 4096);

	// Write memory data (512 bytes)

	esp_partition_write_raw(config_partition, 0, &memory, 512);

	esp_partition_write_raw(config_partition, 512, &lockdevice, 1);

	esp_partition_write_raw(config_partition, 513, &isota, 1);
    


	array[0] = gateway / 256;
	array[1] = gateway % 256;

	esp_partition_write_raw(config_partition, 514, &array, 2);

	esp_partition_write_raw(config_partition, 516, &params, 100);

	esp_partition_write_raw(config_partition, 616, &ssid, 32);

	esp_partition_write_raw(config_partition, 648, &password, 64);
    
	
    array[0] = wakeupvoice / 256;
	array[1] = wakeupvoice % 256;

	esp_partition_write_raw(config_partition, 712, &array, 2);
    

    array[0] = indexresponse / 256;
	array[1] = indexresponse % 256;

	esp_partition_write_raw(config_partition, 734, &array, 2);
    
    esp_partition_write_raw(config_partition, 736, "!", 1);
}



/*
Update thêm timeout 20s tự thoát khỏi vòng lặp nếu ko nhận được tối thiểu 2 byte qua UART
*/
void save_flash()
{
	size_t length_buffer_uart;
	uint8_t data;
	uint32_t position;
	uint16_t i;
	esp_err_t err;  // Biến kiểm tra lỗi cho các thao tác bộ nhớ
	int64_t start_time, current_time;  // Biến lưu thời gian để kiểm tra timeout

	uart_write_bytes(UART_NUM_0, "!", 1);
	while(1)
	{
		// Bắt đầu đếm thời gian cho timeout 20 giây
        start_time = esp_timer_get_time();
        // Chờ nhận được ít nhất 2 byte từ UART với timeout 20 giây
		while(1)
		{
            // Kiểm tra nếu đã nhận được đủ dữ liệu hoặc đã hết thời gian
            current_time = esp_timer_get_time();
            if (length_buffer_uart > 1 || (current_time - start_time) > TIME_OUT_F_SAVE_FLASH_US)  // 20.000.000 micro giây = 20 giây
				// ESP_LOGE(LOG,"[save_flash] Timeout waiting for UART data\n");
                break;
		}

		// Nếu đã hết thời gian chờ mà vẫn không nhận đủ dữ liệu thì thoát
        if ((current_time - start_time) > TIME_OUT_F_SAVE_FLASH_US){
			// ESP_LOGE(LOG,"[save_flash] Timeout waiting for UART data\n");
            return;  // Thoát do timeout
        }

        // Đọc vị trí cần ghi vào bộ nhớ flash
        if (uart_read_bytes(UART_NUM_0, &data, 1, 100) == 1)
            position = data;
        else {
			// ESP_LOGE(LOG,"[save_flash]Timeout on reading pisition low byte\n");
            return;  // Lỗi đọc UART
        }

        if (uart_read_bytes(UART_NUM_0, &data, 1, 100) == 1)
            position += 256 * data;
        else {
			// ESP_LOGE(LOG,"[save_flash]Timeout on reading position high byte\n");
            return;  // Lỗi đọc UART
        }

        memory[position] = 2;  // Ghi vào mảng `memory`
        position *= 4096;

        // Xóa sector bộ nhớ flash tại vị trí
        err = esp_partition_erase_range(partition, position, 4096);
        if (err != ESP_OK){
			// ESP_LOGE(LOG,"[save_flash]Flash erasa failed\n");
            return;  // Lỗi khi xóa bộ nhớ flash
        }

        i = 0;
        uart_write_bytes(UART_NUM_0, "!", 1);  // Báo hiệu sẵn sàng nhận dữ liệu

        // Bắt đầu đếm thời gian cho quá trình nhận dữ liệu (timeout 20 giây)
        start_time = esp_timer_get_time();

        // Vòng lặp nhận dữ liệu và ghi vào bộ nhớ flash
        while (1)
        {
            uart_get_buffered_data_len(UART_NUM_0, &length_buffer_uart);

            // Kiểm tra nếu có dữ liệu hoặc timeout
            current_time = esp_timer_get_time();
            if (length_buffer_uart || (current_time - start_time) > TIME_OUT_F_SAVE_FLASH_US)  // 20 giây timeout
                break;

            if (uart_read_bytes(UART_NUM_0, &data, 1, 100) == 1){
                // Kiểm tra ký tự kết thúc hoặc nếu đã ghi đủ 4096 byte
                if ((data == '!') || (data == '$') || (i >= 4096)) break;

                // Ghi dữ liệu vào bộ nhớ flash
                err = esp_partition_write_raw(partition, position + i, &data, 1);
                if (err != ESP_OK){
					//ESP_LOGE(LOG,"[save_flash]Lash write failed\n");
                    return;  // Lỗi khi ghi vào bộ nhớ flash
                }

                i++;

                // Gửi lại ký tự báo hiệu nếu gặp dấu ';' hoặc ','
                if ((data == ';') || (data == ','))
                    uart_write_bytes(UART_NUM_0, "!", 1);
            }
            else {
				//ESP_LOGE(LOG,"[save_flash]UART read timeout\n");
                return;  // Lỗi đọc UART
            }
        }

        // Nếu đã hết thời gian chờ mà không nhận được dữ liệu, thoát vòng lặp
        if ((current_time - start_time) > TIME_OUT_F_SAVE_FLASH_US){
			// ESP_LOGE(LOG,"[save_flash]Timeout while receiving data\n");
            return;  // Thoát do timeout
        }

        // Ghi ký tự kết thúc vào bộ nhớ flash
        err = esp_partition_write_raw(partition, position + i, "!", 1);
        if (err != ESP_OK){
			// ESP_LOGE(LOG,"[save_flash]Flash write failed at end\n");
            return;  // Lỗi khi ghi ký tự kết thúc vào bộ nhớ flash
        }

        // Điều kiện kết thúc vòng lặp ngoài
        if ((data == '!') || (i >= 4096)) break;
        uart_write_bytes(UART_NUM_0, "!", 1);  // Báo hiệu tiếp tục nếu chưa kết thúc
    }

    save_memory_system();  // Gọi hàm lưu hệ thống bộ nhớ
	//ESP_LOGE(LOG,"[save_flash]Done\n");
}

void load_flash(uint8_t idl, uint8_t idh, uint8_t index)
{
	uint8_t data[6];
	uint16_t i;
	uint16_t id = idl+ 256*idh;
	esp_err_t err;  // Biến kiểm tra lỗi cho các thao tác bộ nhớ

	if(id == 65535){
		for(i=0; i<4096; i++)
		{
			err = esp_partition_read_raw(config_partition, i, data, 1);
			if (err != ESP_OK)
			{
				return;  // Lỗi thao tác với flash
			}

			if(data[0] == '!') break;
			uart_write_bytes(UART_NUM_0, data, 1);
		}
	}
	else{
		uint32_t position = 4096 * id;
		if((memory[id] == 2) || (id < 2)){
			for(i=0; i<4096; i++)
			{
				err = esp_partition_read_raw(partition, position+i, data, 1);
				if (err != ESP_OK)
				{
					return;  // Lỗi thao tác với flash
				}
				if(data[0] == '!') break;
				uart_write_bytes(UART_NUM_0, data, 1);
			}
		}
		else{
			if(memory[id] == 4){
				if(index){
					uint8_t matrix[4] = {0, 0, 0, 0};
					uint8_t j;
					if(find_raw_key(matrix, idl, idh, index)){
						for(j=0; j<2; j++)
						{
							if(matrix[2*j] && matrix[2*j+1]){
								position = 4096*matrix[2*j]+512*matrix[2*j+1];
								for(i=0; i<512; i+=2)
								{
									esp_partition_read_raw(partition, position+i, data, 2);
									uart_write_bytes(UART_NUM_0, data, 2);
									if((!data[0]) && (!data[1])){
										break;
									}
								}
								if((!data[0]) && (!data[1])) {
									break;
								}
							}
						}
					}
				}
				else{
					position = 4096*id;
					for(i=0; i<4096; i+=6)
					{
						err = esp_partition_read_raw(partition, position+i, data, 6);
						if (err != ESP_OK)
						{
							return;  // Lỗi thao tác với flash
						}
						if(data[0] == '!') {
							break;
						}
						uart_write_bytes(UART_NUM_0, data, 6);
					}
				}
			}
		}
	}
}

void clear_flash(uint8_t idl, uint8_t idh)
{
	esp_err_t err;  // Biến kiểm tra lỗi cho các thao tác bộ nhớ
	uint16_t id = idl + 256*idh;
	if(id < 65535){
		err = esp_partition_erase_range(partition, 4096*id, 4096);
		if (err != ESP_OK){
			return;  // Lỗi thao tác với flash
		}
		err = esp_partition_write_raw(partition, 4096*id, "!", 1);
		if (err != ESP_OK){
			return;  // Lỗi thao tác với flash
		}
		memory[id] = 1;
	}
	else{
		if(id == 65535) {
			clear_memory_system();
		}
	}
	save_memory_system();
}

uint8_t find_list_remote(uint8_t idl, uint8_t idh, uint8_t index)
{
	uint8_t i;
	for(i=0; i<100; i+=4)
	{
		if((params[i+1] == idl) && (params[i+2] == idh) && (params[i+3] == index)) {
			return 1;
		}
	}
	return 0;
}

uint8_t check_list_remote(uint8_t idl, uint8_t idh, uint8_t index)
{
	uint8_t i;
	for(i=0; i<100; i+=4)
	{
		if((params[i+1] == idl) && (params[i+2] == idh) && (params[i+3] == index) && ((params[i] == 1) || (params[i] == 3) || (params[i] == 7) || (params[i] == 5) || (params[i] == 8) || (params[i] == 10))) return 1;
	}
	return 0;
}

void send_list_remote()
{
	uint8_t i;
	uint8_t remote[103];
	uint8_t index = 3;
	remote[0] = 242;
	remote[1] = 0;
	for(i=0; i<100; i+=4)
	{
		if(params[i]){
			remote[index] 	= params[i];
			remote[index+1] = params[i+1];
			remote[index+2] = params[i+2];
			remote[index+2] = params[i+3];
			index += 4;
		}
	}
	remote[2] = (index-3) / 4;
	ble_debug_send(gateway, 0xE10211, index, remote);
}

uint8_t check_params_remote()
{
	uint8_t i;
	uint8_t isreceive = 1;
	for(i=0; i<100; i+=4)
	{
		if((params[i] == 1) || (params[i] == 6) || (params[i] == 7) || (params[i] == 9)) isreceive |= 0x02;
		else
		{
			if((params[i] == 5) || (params[i] == 8)) isreceive |= 0x04;	
		}
	}
	return isreceive;
}

uint8_t add_params_remote(uint8_t type, uint8_t idl, uint8_t idh, uint8_t index)
{
	if((type > 10) || (idl+256*idh < 2) || (!index)) return 0;
	uint8_t i;
	for(i=0; i<100; i+=4)
	{
		if((params[i+1] == idl) && (params[i+2] == idh) && (params[i+3] == index))
		{
			if(type) params[i] = type;
			else params[i] = 11;
			return 1;
		}
	}
	for(i=0; i<100; i+=4)
	{
		if(!params[i])
		{
			if(type) params[i] = type;
			else params[i] = 11;
			params[i+1] = idl;
			params[i+2] = idh;
			params[i+3] = index;
			if((memory[params[i+1]+256*params[i+2]] == 2) && (params[i+1]+256*params[i+2] > 20) && (params[i+1]%2 == 0) && ((params[i] == 1) || (params[i] == 7)))
			{
				load_custom_remote(i);
				find_power_custom(i);
				delete_custom_remote();
			}
			if((memory[params[i+1]+256*params[i+2]] == 4) && (params[i+1]+256*params[i+2] < 21) && (params[i+1]+256*params[i+2] > 1) && (find_type_raw(params[i+1], params[i+2]) == 2) && ((params[i] == 1) || (params[i] == 7))) find_power_raw(i);
			return 1;
		}
	}
	return 0;
}

uint8_t delete_params_remote(uint8_t idl, uint8_t idh, uint8_t index)
{
	if((idl+256*idh < 2) || (!index)) return 0;
	uint8_t i;
	for(i=0; i<100; i+=4)
	{
		if(params[i])
		{
			if((params[i+1] == idl) && (params[i+2] == idh) && (params[i+3] == index))
			{
				if((params[i+1]+256*params[i+2] > 1) && (params[i+1]+256*params[i+2] < 21) && (memory[params[i+1]+256*params[i+2]] == 4))
				{
					memory[params[i+1]+256*params[i+2]] = 1;
					clear_flash(params[i+1], params[i+2]);
				}
				delete_action(params[i+1], params[i+2], params[i+3]);
				params[i] = 0;
				params[i+1] = 0;
				params[i+2] = 0;
				params[i+3] = 0;
				return 1;
			}
		}
	}
	return 0;
}

uint8_t delete_all_params_remote()
{
	uint8_t i;
	for(i=0; i<100; i+=4)
	{
		if(params[i])
		{
			if((params[i+1]+256*params[i+2] > 1) && (params[i+1]+256*params[i+2] < 21) && (memory[params[i+1]+256*params[i+2]] == 4))
			{
				memory[params[i+1]+256*params[i+2]] = 1;
				clear_flash(params[i+1], params[i+2]);
			}
			delete_action(params[i+1], params[i+2], params[i+3]);
			params[i] = 0;
			params[i+1] = 0;
			params[i+2] = 0;
			params[i+3] = 0;
		}
	}
	return 1;
}

uint8_t find_type_raw(uint8_t idl, uint8_t idh)
{
	esp_err_t err;
	if((memory[idl+256*idh] != 4) || (idl+256*idh < 2) || (idl+256*idh > 20) ) return 0;
	uint32_t position = 4096*(idl+256*idh);
	uint8_t cache[6];
	uint16_t i;
	for(i=0; i<4096; i+=6)
	{
		err = esp_partition_read_raw(partition, position+i, cache, 6);
		if (err != ESP_OK){
			return 0;  // Lỗi đọc flash
		}
		if(cache[0] == '!') break;
		if(cache[1] > 50) return 2;
	}
	return 1;
}

uint8_t find_raw_key(uint8_t* matrix, uint8_t idl, uint8_t idh, uint8_t key)
{
	esp_err_t err;
	if((idl+256*idh < 2) || (idl+256*idh > 20) || (memory[idl+256*idh] != 4) || (!key)) return 0;
	uint32_t position = 4096*(idl+256*idh);
	uint8_t cache[6];
	uint16_t i;
	for(i=0; i<4096; i+=6)
	{
		err = esp_partition_read_raw(partition, position+i, cache, 6);
		if (err != ESP_OK){
			return 0;  // Lỗi đọc flash
		}
		if(cache[0] == '!') return 0;
		if(cache[0] == RECV_MODE_CMD)
		{
			if(cache[1] == key)
			{
				matrix[0] = cache[2];
				matrix[1] = cache[3];
				matrix[2] = cache[4];
				matrix[3] = cache[5];
				break;
			}
		}
	}
	return 1; 
}

uint8_t check_raw_key(uint8_t x, uint8_t y)
{
	uint32_t position;
	uint8_t cache[6];
	uint8_t i;
	uint16_t j;
	esp_err_t err;
	for(i=2; i<21; i++)
	{
		if(memory[i] != 4) continue;
		position = 4096*i;
		for(j=0; j<4096; j+=6)
		{
			err = esp_partition_read_raw(partition, position+j, cache, 6);
			if (err != ESP_OK){
				return 0;  // Lỗi đọc flash
			}
			if(cache[0] == '!') break;
			if(cache[0] == RECV_MODE_CMD)
			{
				if(((cache[2] == x) && (cache[3] == y)) || ((cache[4] == x) && (cache[5] == y))) return 0;
			}
		}
	}
	return 1; 
}


/*
Fix thêm trường hợp malloc không thành công thì return
Fix thêm trường hợp không check được key cần xoá thì return luôn

*/
uint8_t delete_raw_key(uint8_t idl, uint8_t idh, uint8_t key)
{
    uint16_t index = idl + 256 * idh;
    if ((index < 2) || (index > 20) || (memory[index] != 4) || (!key))
        return 0;

    uint32_t position = 4096 * index;
    uint8_t* cache = (uint8_t*)malloc(4096);
    if (!cache)
        return 0;  // Lỗi cấp phát bộ nhớ

    esp_err_t err = esp_partition_read_raw(partition, position, cache, 4096);
    if (err != ESP_OK){
        free(cache);
        return 0;  // Lỗi đọc flash
    }

    uint16_t i, j = 0;
    uint8_t found = 0;

    // Tìm và xóa khóa
    for (i = 0; i < 4096; i += 6)
    {
        if (cache[i] == '!')
            break;
        if ((cache[i] == RECV_MODE_CMD) && (cache[i + 1] == key)){
            cache[i] = 0;
            found = 1;
            break;
        }
    }

    if (!found){
        free(cache);
        return 0;  // Không tìm thấy key
    }

    // Xóa vùng nhớ flash
    err = esp_partition_erase_range(partition, position, 4096);
    if (err != ESP_OK){
        free(cache);
        return 0;  // Lỗi xóa flash
    }

    // Ghi lại dữ liệu
    for (i = 0; i < 4096; i += 6)
    {
        if (cache[i] == '!')
        {
            err = esp_partition_write_raw(partition, position + j, "!", 1);
			if (err != ESP_OK){
				free(cache);
				return 0;  // Lỗi thao tác flash
			}
            break;
        }
        if (cache[i] == RECV_MODE_CMD)
        {
            err = esp_partition_write_raw(partition, position + j, &cache[i], 6);
			if (err != ESP_OK){
				free(cache);
				return 0;  // Lỗi thao tác flash
			}
            j += 6;
        }
    }

    free(cache);

    // Nếu không còn dữ liệu hợp lệ, cập nhật trạng thái vùng nhớ
    if (!j)
    {
        memory[index] = 1;
        save_memory_system();
    }

    return 1;  // Thành công
}

	
/*
Fix thêm trường hợp malloc không thành công thì return
Fix thêm trường hợp không check được key cần xoá thì return luôn

*/
uint8_t add_raw_key(rmt_item32_t* items, uint16_t number, uint8_t idl, uint8_t idh, uint8_t key)
{
	if(((memory[idl+256*idh] != 1) && (memory[idl+256*idh] != 4)) || (idl+256*idh < 2) || (idl+256*idh > 20) || (!key)) return 0;
	uint8_t matrix[4] = {0, 0, 0, 0};
	uint16_t i;
	uint16_t j;
	uint8_t k = 0;
	esp_err_t err;
	for(i=TOP_DATA_MEMORY; i>BOTTOM_DATA_MEMORY; i--)
	{
		for(j=8; j>0; j--)
		{
			if(check_raw_key(i, j-1))
			{
				matrix[2*k] = i;
				matrix[2*k+1] = j-1;
				k++;
				if(k == number/128 + 1) break;
			}
		}
		if(k == number/128 + 1) break;
	}
	if(k < number/128 + 1) return 0;
	uint32_t position = 4096*(idl+256*idh);
	uint8_t* cache = (uint8_t*)malloc(4096);
    if(cache == NULL)
        return 0;  // Lỗi cấp phát bộ nhớ

	err = esp_partition_read_raw(partition, position, cache, 4096);
	if (err != ESP_OK){
        free(cache);
        return 0;  // Lỗi đọc flash
    }
	for(i=0; i<4096; i+=6)
	{
		if(cache[i] == '!'){
			cache[i] = RECV_MODE_CMD;
			cache[i+1] = key;
			cache[i+2] = matrix[0];
			cache[i+3] = matrix[1];
			cache[i+4] = matrix[2];
			cache[i+5] = matrix[3];
			cache[i+6] = '!';
			break;
		}
		if((cache[i] == RECV_MODE_CMD) && (cache[i+1] == key)){
			cache[i+2] = matrix[0];
			cache[i+3] = matrix[1];
			cache[i+4] = matrix[2];
			cache[i+5] = matrix[3];
			break;
		}
	}
	err = esp_partition_erase_range(partition, position, 4096);
	if (err != ESP_OK){
		return 0;  // Lỗi thao tác flash
	}
	err = esp_partition_write_raw(partition, position, cache, 4096);
	if (err != ESP_OK){
		return 0;  // Lỗi thao tác flash
	}
	j = 0;
	for(k=0; k<1+number/128; k++)
	{
		position = 4096*matrix[2*k];
		err = esp_partition_read_raw(partition, position, cache, 4096);
		if (err != ESP_OK){
			return 0;  // Lỗi thao tác flash
		}
		position = 512*matrix[2*k+1];
		for(i=0; i<512; i+=4)
		{
			cache[position+i] = items[j].duration0 / 256;
			cache[position+i+1] = items[j].duration0 % 256;
			cache[position+i+2] = items[j].duration1 / 256;
			cache[position+i+3] = items[j].duration1 % 256;
			j++;
			if(j >= number) break;
		}
		position = 4096*matrix[2*k];
		err = esp_partition_erase_range(partition, position, 4096);
		if (err != ESP_OK){
			return 0;  // Lỗi thao tác flash
		}
		err = esp_partition_write_raw(partition, position, cache, 4096);
		if (err != ESP_OK){
			return 0;  // Lỗi thao tác flash
		}
	}

	free(cache);
	cache = NULL;
	if(memory[idl+256*idh] != 4){
		memory[idl+256*idh] = 4;
		save_memory_system();
	}
	return 1;
}

uint8_t choose_raw_key(uint8_t idl, uint8_t idh, uint8_t temp, uint8_t mode, uint8_t fan)
{
	esp_err_t err;
	if((idl+256*idh<2) || (idl+256*idh>20) || (memory[idl+256*idh] != 4)) return 0;
	uint16_t i;
	uint8_t min = 255;
	uint32_t position = 4096*(idl+256*idh);
	uint8_t cache[2];
	for(i=0; i<4096; i+=6)
	{
		err = esp_partition_read_raw(partition, position+i, cache, 2);
		if (err != ESP_OK)
		{
			return 0;  // Lỗi thao tác flash
		}
		if(cache[0] == '!') break;
		if(cache[0] == RECV_MODE_CMD)
		{
			if(temp > 0)
			{
				if(cache[1] & 0xC0)
				{
					if(((cache[1] >> 2)&0x0F)+16 == temp)
					{
						if(cache[1] < min) min = cache[1];
					}
				}
			}
		}
	}
	if(min == 255) min = 0;
	return min;
}

uint8_t load_custom_remote(uint8_t index)
{
	if((!params[index+3]) || (memory[params[index+1]+256*params[index+2]] != 2) || (params[index+1]+256*params[index+2] < 21)) return 0;
	uint32_t position = (params[index+1]+256*params[index+2]) * 4096;
	uint16_t i = 0;
	uint16_t start;
	uint8_t id;
	uint8_t data;
	struct CUSTOM_IR custom_ir;
	int64_t start_time, current_time;  // Biến lưu thời gian để kiểm tra timeout
	esp_err_t err;
	while(1)
	{
		id = 0;
		start = i;
		while(1)
		{
			err = esp_partition_read_raw(partition, position+i, &data, 1);
			if (err != ESP_OK){
				return 0;  // Lỗi thao tác flash
			}
			if(data == ',') {
				break;
			}
			id = 10 * id + data - 48;
			i++;
		}
		if(id == params[index+3]) {
			break;
		}
		while(1)
		{
			err = esp_partition_read_raw(partition, position+i, &data, 1);
			if (err != ESP_OK){
				return 0;  // Lỗi thao tác flash
			}
			i++;
			if(data == ';') {
				break;
			}
		}
		err = esp_partition_read_raw(partition, position+i, &data, 1);
		if (err != ESP_OK){
			return 0;  // Lỗi thao tác flash
		}
		if(data == '!') {
			break;
		}
	}
	if(id != params[index+3]) return;
	uint8_t j;
	i = start;
	while(1)
	{
		err = esp_partition_read_raw(partition, position+i, &data, 1);
		if (err != ESP_OK){
			return 0;  // Lỗi thao tác flash
		}
		i++;
		if(data == ',') break;
	}
	for(j=0; j<4; j++)
	{
		custom_ir.logic_ir[j] = 0;
		while(1)
		{
			err = esp_partition_read_raw(partition, position+i, &data, 1);
			if (err != ESP_OK){
				return 0;  // Lỗi thao tác flash
			}
			i++;
			if(data == ',') break;
			custom_ir.logic_ir[j] = 10 * custom_ir.logic_ir[j] + data - 48;
		}
	}
	err = esp_partition_read_raw(partition, position+i, &data, 1);
	if (err != ESP_OK){
		return 0;  // Lỗi thao tác flash
	}
	custom_ir.number_section_ir = data - 48;
	i += 2;
	for(j=0; j<custom_ir.number_section_ir; j++)
	{
		custom_ir.size_section_ir[j] = 0;
		while(1)
		{
			err = esp_partition_read_raw(partition, position+i, &data, 1);
			if (err != ESP_OK){
				return 0;  // Lỗi thao tác flash
			}
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
			err =  esp_partition_read_raw(partition, position+i, &data, 1);
			if (err != ESP_OK){
				return 0;  // Lỗi thao tác flash
			}
			i++;
			if(data == ',') break;
			custom_ir.section_ir[j] = 10 * custom_ir.section_ir[j] + data - 48;
		}
	}
	remote_param_ir = (uint16_t*)malloc(2*(5*custom_ir.number_section_ir+5));
	for(j=0; j<4; j++) remote_param_ir[j] = custom_ir.logic_ir[j];
	remote_param_ir[4] = custom_ir.number_section_ir;
	for(j=0; j<custom_ir.number_section_ir; j++) remote_param_ir[j+5] = custom_ir.size_section_ir[j];
	for(j=0; j<4*custom_ir.number_section_ir; j++) remote_param_ir[j+custom_ir.number_section_ir+5] = custom_ir.section_ir[j];
	
	remote_code3_ir = (uint64_t*)malloc(136);
	if (remote_code3_ir == NULL) {
		return 0;
	}
	if(custom_ir.size_section_ir[custom_ir.number_section_ir-1] > 64) {
		remote_code2_ir = (uint64_t*)malloc(136);
	}
	if(custom_ir.size_section_ir[custom_ir.number_section_ir-1] > 128) {
		remote_code1_ir = (uint64_t*)malloc(136);
	}
	if(custom_ir.size_section_ir[custom_ir.number_section_ir-1] > 192) {
		remote_code0_ir = (uint64_t*)malloc(136);
	}
	
	for(j=0; j<17; j++)
	{
		remote_code3_ir[j] = 0;
		if(remote_code2_ir != NULL) remote_code2_ir[j] = 0;
		if(remote_code1_ir != NULL) remote_code1_ir[j] = 0;
		if(remote_code0_ir != NULL) remote_code0_ir[j] = 0;
	}
	uint8_t k = 0;
	while(1)
	{
		j = 0;
		custom_ir.result_section_ir[0] = 0;
		custom_ir.result_section_ir[1] = 0;
		custom_ir.result_section_ir[2] = 0;
		custom_ir.result_section_ir[3] = 0;
		while(1)
		{
			err = esp_partition_read_raw(partition, position+i, &data, 1);
			if (err != ESP_OK){
				return 0;  // Lỗi thao tác flash
			}
			i++;
			if((data == ',') || (data == ';')){
				remote_code3_ir[k] = custom_ir.result_section_ir[3];
				if(remote_code2_ir != NULL) remote_code2_ir[k] = custom_ir.result_section_ir[2];
				if(remote_code1_ir != NULL) remote_code1_ir[k] = custom_ir.result_section_ir[1];
				if(remote_code0_ir != NULL) remote_code0_ir[k] = custom_ir.result_section_ir[0];
				k++;
				break;
			}
			if((data >= '0') && (data <= '9')) {
				data -= 48;
			}
			else{
				if((data >= 'A') && (data <= 'F')) data -= 55;
			}
			if(j < 16) {
				custom_ir.result_section_ir[3] = 16 * custom_ir.result_section_ir[3] + data;
			}
			else{
				if(j < 32) {
					custom_ir.result_section_ir[2] = 16 * custom_ir.result_section_ir[2] + data;
				}
				else{
					if(j < 48) {
						custom_ir.result_section_ir[1] = 16 * custom_ir.result_section_ir[1] + data;
					}
					else {
						custom_ir.result_section_ir[0] = 16 * custom_ir.result_section_ir[0] + data;
					}
				}
			}
			j++;
		}
		if(data == ';') break;
	}
}

void delete_custom_remote()
{
	if(remote_param_ir != NULL)
	{
		free(remote_param_ir);
		remote_param_ir = NULL;
	}
	if(remote_code0_ir != NULL)
	{
		free(remote_code0_ir);
		remote_code0_ir = NULL;
	}
	if(remote_code1_ir != NULL)
	{
		free(remote_code1_ir);
		remote_code1_ir = NULL;
	}
	if(remote_code2_ir != NULL)
	{
		free(remote_code2_ir);
		remote_code2_ir = NULL;
	}
	if(remote_code3_ir != NULL)
	{
		free(remote_code3_ir);
		remote_code3_ir = NULL;
	}
}

uint8_t add_script(uint8_t idl, uint8_t idh, uint8_t info1, uint8_t info2, uint8_t info3, uint8_t info4, uint8_t info5)
{
	uint8_t tmpcache[11] = {RECV_SCRIPT_CMD, idl, idh, 0, info2, info3, 0, 0, 0, 0, 0};
	tmpcache[3] = info1 & 0x03;
	if(tmpcache[3] == RECV_AC_CMD)
	{
		tmpcache[6] = (info4 >> 3) & 0x1F;
		tmpcache[7] = (info5 >> 4) & 0x0F;
		if(tmpcache[7] > 1) tmpcache[7] += 16;
		tmpcache[8] = info4 & 0x07;
		tmpcache[9] = ((info5 >> 2) & 0x03) + 1;
		tmpcache[10] = 2*(info5 & 0x03)+1;
	}
	else
	{
		if(tmpcache[3] == RECV_KEY_CMD)
		{
			tmpcache[6] = (info4 >> 3) & 0x1F;
			tmpcache[7] = (info5 >> 4) & 0x0F;
			if(tmpcache[4]+256*tmpcache[5] > 20)
			{
				if(tmpcache[4]%2 == 0)
				{
					if(tmpcache[7] > 1) tmpcache[7] += 16;
				}
			}
			else
			{
				if((tmpcache[4]+256*tmpcache[5] > 1) && (tmpcache[4]+256*tmpcache[5] < 21))
				{
					if(find_type_raw(tmpcache[4], tmpcache[5]) == 2)
					{
						if(tmpcache[7] > 1) tmpcache[7] += 16;
					}
				}
			}
		}
	}
	if(tmpcache[3] == RECV_AC_CMD)
	{
		if((tmpcache[4]+256*tmpcache[5] < 21) || (tmpcache[4]%2) || (!tmpcache[6])) return 0;
		if((tmpcache[7] != 1) && ((tmpcache[7] < 16) || (tmpcache[7] > 30))) return 0;
		if((!tmpcache[8]) || (tmpcache[8] > 5) || (!tmpcache[9]) || (tmpcache[9] > 4) || (!tmpcache[10]) || (tmpcache[10] > 7)) return 0;
	}
	else
	{
		if(tmpcache[3] == RECV_KEY_CMD)
		{
			if((tmpcache[4]+256*tmpcache[5] < 2) || (!tmpcache[6]) || (!tmpcache[7])) return 0;
		}
		else
		{
			if(!tmpcache[3])
			{
				if((tmpcache[4]+256*tmpcache[5] < 110) && (tmpcache[4]+256*tmpcache[5] > 99)) return 0;
			}
			else return 0;
		}
	}
	uint16_t i;
	uint8_t* cache = (uint8_t*)malloc(4096);
	esp_err_t err = esp_partition_read_raw(partition, 4096, cache, 4096);
	if(err != ESP_OK || cache == NULL){
		return 0;
	}
	for(i=0; i<4096; i+=11)
	{
		if(cache[i] == '!')
		{
			cache[i] = tmpcache[0];
			cache[i+1] = tmpcache[1];
			cache[i+2] = tmpcache[2];
			cache[i+3] = tmpcache[3];
			cache[i+4] = tmpcache[4];
			cache[i+5] = tmpcache[5];
			cache[i+6] = tmpcache[6];
			cache[i+7] = tmpcache[7];
			cache[i+8] = tmpcache[8];
			cache[i+9] = tmpcache[9];
			cache[i+10] = tmpcache[10];
			cache[i+11] = '!';
			break;
		}
		if(tmpcache[3] == 0)
		{
			if((cache[i] == RECV_SCRIPT_CMD) && (cache[i+1] == idl) && (cache[i+2] == idh) && (cache[i+3] == 0))
			{
				cache[i+4] = tmpcache[4];
				cache[i+5] = tmpcache[5];
				break;
			}
		}
		else
		{
			if((cache[i] == RECV_SCRIPT_CMD) && (cache[i+1] == idl) && (cache[i+2] == idh) && (cache[i+3] == tmpcache[3]) && (cache[i+4] == tmpcache[4]) && (cache[i+5] == tmpcache[5]) && (cache[i+6] == tmpcache[6]))
			{
				cache[i+7] = tmpcache[7];
				cache[i+8] = tmpcache[8];
				cache[i+9] = tmpcache[9];
				cache[i+10] = tmpcache[10];
				break;
			}
		}
	}
	err = esp_partition_erase_range(partition, 4096, 4096);
	if(err != ESP_OK){
		free(cache);
		return 0;
	}
	err = esp_partition_write_raw(partition, 4096, cache, 4096);
	if(err != ESP_OK){
		free(cache);
		return 0;
	}
	free(cache);
	cache = NULL;
	return 1;
}

uint8_t delete_script(uint8_t idl, uint8_t idh, uint8_t info1, uint8_t info2, uint8_t info3, uint8_t info4)
{
	uint8_t tmpcache[7] = {RECV_SCRIPT_CMD, idl, idh, 0, info2, info3, 0};
	tmpcache[3] = info1 & 0x03;
	if(tmpcache[3] == RECV_AC_CMD) tmpcache[6] = (info4 >> 3) & 0x1F;
	else
	{
		if(tmpcache[3] == RECV_KEY_CMD) tmpcache[6] = (info4 >> 3) & 0x1F;
	}
	if(tmpcache[3] == RECV_AC_CMD)
	{
		if((tmpcache[4]+256*tmpcache[5] < 21) || (tmpcache[4]%2) || (!tmpcache[6])) return 0;
	}
	else
	{
		if(tmpcache[3] == RECV_KEY_CMD)
		{
			if((tmpcache[4]+256*tmpcache[5] < 2) || (!tmpcache[6])) return 0;
		}
		else
		{
			if(tmpcache[3]) return 0;
		}
	}
	uint16_t i;
	uint8_t* cache = (uint8_t*)malloc(4096);
	esp_err_t err = esp_partition_read_raw(partition, 4096, cache, 4096);
	if(err != ESP_OK || cache == NULL){
		return 0;
	}
	for(i=0; i<4096; i+=11)
	{
		if(cache[i] == '!')
		{
			free(cache);
			cache = NULL;
			return 0;
		}
		if((cache[i] == RECV_SCRIPT_CMD) && (cache[i+1] == idl) && (cache[i+2] == idh) && (cache[i+3] == tmpcache[3]))
		{
			if(cache[i+3] == 0)
			{
				cache[i] = 0;
				break;
			}
			else
			{
				if((cache[i+3] == RECV_KEY_CMD) || (cache[i+3] == RECV_AC_CMD))
				{
					if((cache[i+4] == tmpcache[4]) && (cache[i+5] == tmpcache[5]) && (cache[i+6] == tmpcache[6]))
					{
						cache[i] = 0;
						break;
					}
				}
			}
		}
	}
	uint16_t j = 0;
	esp_partition_erase_range(partition, 4096, 4096);
	for(i=0; i<4096; i+=11)
	{
		if(cache[i] == '!')
		{
			esp_partition_write_raw(partition, 4096+j, "!", 1);
			break;
		}
		if(cache[i])
		{
			esp_partition_write_raw(partition, 4096+j, &cache[i], 11);
			j += 11;
		}
	}
	free(cache);
	cache = NULL;
	return 1;
}

void delete_action(uint8_t idl, uint8_t idh, uint8_t index)
{
	uint16_t i;
	uint8_t* cache = (uint8_t*)malloc(4096);
	esp_err_t err = esp_partition_read_raw(partition, 4096, cache, 4096);
	if(err != ESP_OK || cache == NULL){
		return 0;
	}
	for(i=0; i<4096; i+=11)
	{
		if(cache[i] == '!') break;
		if((cache[i] == RECV_SCRIPT_CMD) && cache[i+3] && (cache[i+4] == idl) && (cache[i+5] == idh) && (cache[i+6] == index)) cache[i] = 0;
	}
	uint16_t j = 0;
	esp_partition_erase_range(partition, 4096, 4096);
	for(i=0; i<4096; i+=11)
	{
		if(cache[i] == '!')
		{
			esp_partition_write_raw(partition, 4096+j, "!", 1);
			break;
		}
		if(cache[i])
		{
			esp_partition_write_raw(partition, 4096+j, &cache[i], 11);
			j += 11;
		}
	}
	free(cache);
	cache = NULL;
}

uint8_t delete_all_script(uint8_t idl, uint8_t idh)
{
	uint16_t i;
	uint8_t isok = 0;
	uint8_t* cache = (uint8_t*)malloc(4096);
	esp_err_t err = esp_partition_read_raw(partition, 4096, cache, 4096);
	if(err != ESP_OK || cache == NULL){
		return 0;
	}
	for(i=0; i<4096; i+=11)
	{
		if(cache[i] == '!') break;
		if((cache[i+1] == idl) && (cache[i+2] == idh))
		{
			cache[i] = 0;
			isok = 1;
		}
	}
	if(isok)
	{
		uint16_t j = 0;
		esp_partition_erase_range(partition, 4096, 4096);
		for(i=0; i<4096; i+=11)
		{
			if(cache[i] == '!')
			{
				esp_partition_write_raw(partition, 4096+j, "!", 1);
				break;
			}
			if(cache[i])
			{
				esp_partition_write_raw(partition, 4096+j, &cache[i], 11);
				j += 12;
			}
		}
	}
	free(cache);
	cache = NULL;
	return isok;
}

uint8_t find_script(uint8_t idl, uint8_t idh)
{
	uint16_t i;
	uint8_t cache[11];
	uint8_t isfind = 0;
	esp_err_t err;
	for(i=0; i<4096; i+=11)
	{
		err = esp_partition_read_raw(partition, 4096+i, cache, 11);
		if(err != ESP_OK){
			return 0;
		}
		if(cache[0] == '!') break;
		if((cache[4]+256*cache[5] == idl+256*idh) && (!cache[3])){
			isfind = 1;
			ble_mesh_send(0xFFFF, 0x8243, 3, cache[1], cache[2], 1, 0, 0, 0, 0, 0);
		}
	}
	return isfind;
}

void set_status_script(uint8_t idl, uint8_t idh, uint8_t status)
{
	uint16_t i;
	uint8_t* cache = (uint8_t*)malloc(4096);
	esp_err_t err = esp_partition_read_raw(partition, 4096, cache, 4096);
	if(err != ESP_OK || cache == NULL){
		return 0;
	}
	for(i=0; i<4096; i+=11)
	{
		if(cache[i] == '!') break;
		if((cache[i+1]+256*cache[i+2] == idl+256*idh) && (!cache[i+3]))
		{
			if((cache[i+4]+256*cache[i+5]) && (!status))
			{
				cache[i+6] = cache[i+4];
				cache[i+7] = cache[i+5];
				cache[i+4] = 0;
				cache[i+5] = 0;
			}
			else
			{
				if((cache[i+4]+256*cache[i+5] == 0) && status)
				{
					cache[i+4] = cache[i+6];
					cache[i+5] = cache[i+7];
					cache[i+6] = 0;
					cache[i+7] = 0;
				}
			}
		}
	}
	esp_partition_erase_range(partition, 4096, 4096);
	esp_partition_write_raw(partition, 4096, cache, 4096);
	free(cache);
	cache = NULL;
	ble_mesh_send(gateway, 0xE10211, 5, 112, 0, idl, idh, status, 0, 0, 0);
}

void send_list_script()
{
	uint16_t i;
	uint8_t j;
	uint8_t cache[11];
	uint8_t script[254];
	uint8_t index = 3;
	script[0] = 243;
	script[1] = 0;
	esp_err_t err;
	for(i=0; i<4096; i+=11)
	{
		err = esp_partition_read_raw(partition, 4096+i, cache, 11);
		if(err != ESP_OK){
			return 0;
		}
		if(cache[0] == '!') break;
		for(j=3; j<index; j+=2)
		{
			if((cache[1] == script[j]) && (cache[2] == script[j+1])) break;
		}
		if(j >= index)
		{
			script[index] = cache[1];
			script[index+1] = cache[2];
			index += 2;
		}
	}
	script[2] = (index-3) / 2;
	ble_debug_send(gateway, 0xE10211, index, script);
}

uint8_t save_wifi(uint8_t info[])
{
	if((info[1] > 32) || (info[2] > 64) || (info[1] + info[2] + 3 != info[99])) return 0;
	uint8_t i;
	for(i=3; i<info[1]+3; i++) ssid[i-3] = info[i];
	if(info[1] < 32) ssid[i-3] = '\0';
	for(i=info[1]+3; i<info[1]+info[2]+3; i++) password[i-info[1]-3] = info[i];
	if(info[2] < 64) password[i-info[1]-3] = '\0';
	isota = 0;
	save_memory_system();
	return 1;
}

uint8_t save_ota(uint8_t type, uint32_t file1, uint32_t file2)
{
	if((!type) || (type > 4) || (!file1)) return 0;
	isota = type;
	save_memory_system();
	uint8_t array[20];
	uint8_t i;
	for(i=0; i<20; i++) array[i] = 0;
	switch(type)
	{
		case 1:
			array[0] = 'E';
			array[6] = '.';
			array[7] = 'b';
			array[8] = 'i';
			array[9] = 'n';
			break;
		case 2:
			array[0] = 'D';
			array[6] = '.';
			array[7] = 't';
			array[8] = 'x';
			array[9] = 't';
			break;
		case 3:
			array[0] = 'R';
			array[6] = '.';
			array[7] = 'b';
			array[8] = 'i';
			array[9] = 'n';
			break;
		case 4:
			array[0] = 'E';
			array[6] = '.';
			array[7] = 'b';
			array[8] = 'i';
			array[9] = 'n';
			array[10] = 'R';
			array[16] = '.';
			array[17] = 'b';
			array[18] = 'i';
			array[19] = 'n';
			break;
	}
	for(i=1; i<6; i++)
	{
		array[6-i] = file1 % 10 + 48;
		file1 /= 10;
	}
	if(type == 4)
	{
		for(i=1; i<6; i++)
		{
			array[16-i] = file2 % 10 + 48;
			file2 /= 10;
		}
	}
	esp_partition_write_raw(config_partition, 714, array, 20);
	return 1;
}

void save_status_remote(uint8_t idl, uint8_t idh, uint8_t index, uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	if(!check_list_remote(idl, idh, index)) return;
	uint16_t i;
	uint8_t cache[480];
	esp_err_t err = esp_partition_read_raw(partition, 0, cache, 480);
	if(err != ESP_OK ){
		return 0;
	}
	for(i=0; i<480; i+=24)
	{
		if(cache[i] == '!')
		{
			cache[i] = SEND_KEY_CMD;
			cache[i+1] = idl;
			cache[i+2] = idh;
			cache[i+3] = index;
			cache[i+4] = temp;
			cache[i+5] = mode;
			cache[i+6] = fan;
			cache[i+7] = swing;
			cache[i+8] = 255;
			cache[i+9] = 255;
			cache[i+10] = 255;
			cache[i+11] = 255;
			cache[i+12] = 255;
			cache[i+13] = 255;
			cache[i+14] = 255;
			cache[i+15] = 255;
			cache[i+16] = 255;
			cache[i+17] = 255;
			cache[i+18] = 255;
			cache[i+19] = 255;
			cache[i+20] = 255;
			cache[i+21] = 255;
			cache[i+22] = 255;
			cache[i+23] = 255;
			cache[i+24] = '!';
			break;
		}
		if(cache[i] == SEND_KEY_CMD)
		{
			if((cache[i+1] == idl) && (cache[i+2] == idh) && (cache[i+3] == index))
			{
				cache[i+4] = temp;
				cache[i+5] = mode;
				cache[i+6] = fan;
				cache[i+7] = swing;
				break;
			}
		}
	}
	esp_partition_erase_range(partition, 0, 4096);
	esp_partition_write_raw(partition, 0, cache, 480);
}

uint8_t load_status_remote(uint8_t idl, uint8_t idh, uint8_t index, uint8_t *status)
{
	uint16_t i;
	uint8_t data[8];
	esp_err_t err;
	for(i=0; i<480; i+=24)
	{
		err = esp_partition_read_raw(partition, i, data, 8);
		if(err != ESP_OK){
			return 0;
		}
		if(data[0] == '!') break;
		if(data[0] == SEND_KEY_CMD)
		{
			if((data[1] == idl) && (data[2] == idh) && (data[3] == index))
			{
				status[0] = data[4];
				status[1] = data[5];
				status[2] = data[6];
				status[3] = data[7];
				return 1;
			}
		}
	}
	return 0;
}

void set_position_remote(uint8_t idl, uint8_t idh, uint8_t index, uint8_t* position)
{
	uint16_t i;
	uint8_t cache[480];
	esp_err_t err = esp_partition_read_raw(partition, 0, cache, 480);
	if(err != ESP_OK){
		return 0;
	}
	for(i=0; i<480; i+=24)
	{
		if(cache[i] == '!'){
			cache[i] = SEND_KEY_CMD;
			cache[i+1] = idl;
			cache[i+2] = idh;
			cache[i+3] = index;
			cache[i+4] = 26;
			cache[i+5] = 2;
			cache[i+6] = 1;
			cache[i+7] = 1;
			cache[i+8] = position[0];
			cache[i+9] = position[1];
			cache[i+10] = position[2];
			cache[i+11] = position[3];
			cache[i+12] = position[4];
			cache[i+13] = position[5];
			cache[i+14] = position[6];
			cache[i+15] = position[7];
			cache[i+16] = position[8];
			cache[i+17] = position[9];
			cache[i+18] = position[10];
			cache[i+19] = position[11];
			cache[i+20] = position[12];
			cache[i+21] = position[13];
			cache[i+22] = position[14];
			cache[i+23] = position[15];
			cache[i+24] = '!';
			break;
		}
		if(cache[i] == SEND_KEY_CMD){
			if((cache[i+1] == idl) && (cache[i+2] == idh) && (cache[i+3] == index))
			{
				cache[i+8] = position[0];
				cache[i+9] = position[1];
				cache[i+10] = position[2];
				cache[i+11] = position[3];
				cache[i+12] = position[4];
				cache[i+13] = position[5];
				cache[i+14] = position[6];
				cache[i+15] = position[7];
				cache[i+16] = position[8];
				cache[i+17] = position[9];
				cache[i+18] = position[10];
				cache[i+19] = position[11];
				cache[i+20] = position[12];
				cache[i+21] = position[13];
				cache[i+22] = position[14];
				cache[i+23] = position[15];
				break;
			}
		}
	}
	esp_partition_erase_range(partition, 0, 4096);
	esp_partition_write_raw(partition, 0, cache, 480);
}

void load_position_remote(uint8_t idl, uint8_t idh, uint8_t index, uint8_t *position)
{
	uint16_t i;
	uint8_t data[24];
	esp_err_t err;
	for(i=0; i<480; i+=24)
	{
		err = esp_partition_read_raw(partition, i, data, 24);
		if(err != ESP_OK){
			return 0;
		}
		if(data[0] == '!') break;
		if(data[0] == SEND_KEY_CMD)
		{
			if((data[1] == idl) && (data[2] == idh) && (data[3] == index))
			{
				position[0] = data[8];
				position[1] = data[9];
				position[2] = data[10];
				position[3] = data[11];
				position[4] = data[12];
				position[5] = data[13];
				position[6] = data[14];
				position[7] = data[15];
				position[8] = data[16];
				position[9] = data[17];
				position[10] = data[18];
				position[11] = data[19];
				position[12] = data[20];
				position[13] = data[21];
				position[14] = data[22];
				position[15] = data[23];
				return;
			}
		}
	}
}

void find_power_custom(uint8_t index)
{
	if(remote_param_ir == NULL) return;
	uint64_t one = 1;
	uint8_t i;
	uint8_t j;
	uint8_t k = 0;
	uint8_t position[16] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
	uint8_t error3;
	uint8_t error2;
	uint8_t error1;
	uint8_t error0;
	for(i=0; i<64; i++)
	{
		error3 = 0;
		error2 = 0;
		error1 = 0;
		error0 = 0;
		for(j=2; j<17; j++)
		{
			if(remote_code3_ir != NULL)
			{
				if (remote_code3_ir[j] && remote_code3_ir[1])
				{
					if(((remote_code3_ir[1] & (one << i)) ^ (remote_code3_ir[j] & (one << i))) == 0) error3 = 1;
				}
			}
			else error3 = 1;
			if(remote_code2_ir != NULL)
			{
				if (remote_code2_ir[j] && remote_code2_ir[1])
				{
					if(((remote_code2_ir[1] & (one << i)) ^ (remote_code2_ir[j] & (one << i))) == 0) error2 = 1;
				}
			}
			else {
				error2 = 1;
			}
			if(remote_code1_ir != NULL){
				if (remote_code1_ir[j] && remote_code1_ir[1]){
					if(((remote_code1_ir[1] & (one << i)) ^ (remote_code1_ir[j] & (one << i))) == 0) error1 = 1;
				}
			}
			else {
				error1 = 1;
			}
			if(remote_code0_ir != NULL){
				if (remote_code0_ir[j] && remote_code0_ir[1]){
					if(((remote_code0_ir[1] & (one << i)) ^ (remote_code0_ir[j] & (one << i))) == 0) error0 = 1;
				}
			}
			else {
				error0 = 1;
			}
		}
		if(error3 == 0){
			if(k < 16){
				position[k] = i;
				k++;
			}
		}
		if(error2 == 0){
			if(k < 16){
				position[k] = i+64;
				k++;
			}
		}
		if(error1 == 0){
			if(k < 16){
				position[k] = i+128;
				k++;
			}
		}
		if(error0 == 0){
			if(k < 16){
				position[k] = i+192;
				k++;
			}
		}
	}
	set_position_remote(params[index+1], params[index+2], params[index+3], position);
}

void find_power_raw(uint8_t index)
{
	if((memory[params[index+1]+256*params[index+2]] != 4) || (params[index+1]+256*params[index+2]>20) || (params[index+1]+256*params[index+2]<2) || (find_type_raw(params[index+1], params[index+2]) != 2)) return;
	uint32_t addr[32] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	uint16_t i;
	uint8_t j = 2;
	uint8_t cache[6];
	esp_err_t err;
	for(i=0; i<4096; i+=6)
	{
		err = esp_partition_read_raw(partition, 4096*(params[index+1]+256*params[index+2])+i, cache, 6);
		if(err != ESP_OK){
			return 0;
		}
		if(cache[0] == '!') break;
		if(cache[1] > 1){
			addr[j] = 4096*cache[2]+512*cache[3];
			addr[j+1] = 4096*cache[4]+512*cache[5];
			j += 2;
		}
		else{
			addr[0] = 4096*cache[2]+512*cache[3];
			addr[1] = 4096*cache[4]+512*cache[5];
		}
	}
	if((!addr[0]) || (!addr[2])) return;
	uint8_t position[16] = {255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255};
	uint16_t sampleduration0;
	uint16_t sampleduration1;
	uint16_t duration0;
	uint16_t duration1 = 0;
	uint8_t error;
	uint8_t k = 0;
	for(i=0; i<512; i+=4)
	{
		error = 0;
		err = esp_partition_read_raw(partition, addr[0]+i, cache, 4);
		if(err != ESP_OK){
			return 0;
		}
		sampleduration0 = 256*cache[0]+cache[1];
		sampleduration1 = 256*cache[2]+cache[3];
		for(j=2; j<32; j+=2)
		{
			if(!addr[j]) break;
			err = esp_partition_read_raw(partition, addr[j]+i, cache, 4);
			if(err != ESP_OK){
				return 0;
			}
			duration0 = 256*cache[0]+cache[1];
			duration1 = 256*cache[2]+cache[3];
			if(sampleduration1 * duration1 == 0) break;
			if((duration0 > sampleduration0-50) && (duration0 < sampleduration0+50) && (duration1 > sampleduration1-50) && (duration1 < sampleduration1+50))
			{
				error = 1;
				break;
			}
		}
		if(sampleduration1 * duration1 == 0) break;
		if(!error)
		{
			if(k < 16)
			{
				position[k] = i/4;
				k++;
			}
		}
	}
	if(i >= 512)
	{
		for(i=0; i<512; i+=4)
		{
			error = 0;
			err = esp_partition_read_raw(partition, addr[1]+i, cache, 4);
			if(err != ESP_OK){
				return 0;
			}
			sampleduration0 = 256*cache[0]+cache[1];
			sampleduration1 = 256*cache[2]+cache[3];
			for(j=3; j<32; j+=2)
			{
				if(!addr[j]) break;
				err = esp_partition_read_raw(partition, addr[j]+i, cache, 4);
				if(err != ESP_OK){
					return 0;
				}
				duration0 = 256*cache[0]+cache[1];
				duration1 = 256*cache[2]+cache[3];
				if(sampleduration1 * duration1 == 0) break;
				if((duration0 > sampleduration0-50) && (duration0 < sampleduration0+50) && (duration1 > sampleduration1-50) && (duration1 < sampleduration1+50)){
					error = 1;
					break;
				}
			}
			if(sampleduration1 * duration1 == 0) break;
			if(!error){
				if(k < 16){
					position[k] = 128+i/4;
					k++;
				}
			}
		}
	}
	set_position_remote(params[index+1], params[index+2], params[index+3], position);
}

void kick_out()
{
	uint8_t i;
	ble_mesh_send(gateway, 0xE10211, 3, 22, 0, 1, 0, 0, 0, 0 ,0);
	nvs_flash_erase();
	if(!lockdevice) lockdevice = 2;
	isota = 0;
	gateway = 0xFFFF;
	for(i=0; i<100; i++) params[i] = 0;
	for(i=0; i<32; i++) ssid[i] = '\0';
	for(i=0; i<64; i++) password[i] = '\0';
	wakeupvoice = 100;
	indexresponse = 1;
	for(i=0; i<21; i++)
	{
		memory[i] = 1;
		esp_partition_erase_range(partition, 4096*i, 4096);
		esp_partition_write_raw(partition, 4096*i, "!", 1);
	}
	save_memory_system();
}

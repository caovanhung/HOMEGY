#include "thuvien.h"

uint8_t request[8];
uint8_t memory[512];
uint8_t lockdevice = 0;
uint8_t isota = 0;
uint16_t gateway = 0xFFFF;
uint8_t params[100];
uint8_t ssid[32];
uint8_t password[64];
uint16_t wakeupvoice = 100;
uint16_t indexresponse = 0;
uint16_t* remote_param_ir = NULL;
uint64_t* remote_code0_ir = NULL;
uint64_t* remote_code1_ir = NULL;
uint64_t* remote_code2_ir = NULL;
uint64_t* remote_code3_ir = NULL;
struct CUSTOM_IR custom_ir;
RingbufHandle_t ring_buffer_ir;
rmt_item32_t* items_tx_ir = NULL;
uint16_t number_tx_ir = 0;
const esp_partition_t* partition = NULL;
const esp_partition_t* ota_partition = NULL;
const esp_partition_t* config_partition = NULL;
uint16_t timewaittingresponse = 0;

#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/uart.h"
#include "nvs_flash.h"
#include "esp_partition.h"
#include "esp_wifi.h"
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_ble_mesh_common_api.h"
#include "esp_ble_mesh_networking_api.h"
#include "esp_ble_mesh_provisioning_api.h"
#include "esp_ble_mesh_config_model_api.h"
#include "esp_ble_mesh_local_data_operation_api.h"
#include "ble_mesh_example_init.h"
#include "settings.h"
#include "storage/settings_nvs.h"
#include "esp_log.h"

#define TOP_DATA_MEMORY 255
#define BOTTOM_DATA_MEMORY 114

#define NONE_CMD 			0
#define CONFIG_MEMORY_CMD	1
#define RECV_KEY_CMD		2
#define RECV_AC_CMD			3
#define RECV_PARAM_CMD		4
#define RECV_MODE_CMD		5
#define RECV_SCRIPT_CMD		6
#define RECV_LOCAL_CMD		7
#define RECV_WIFI_CMD		8
#define RECV_VOICE_CMD		9
#define SEND_KEY_CMD		10
#define SEND_TEACHING_CMD	11
#define RECV_OTA_CMD		12
#define RECV_WAKEUP_CMD		13
#define RECV_VOLUME_CMD		14
#define SEND_RELAY_CMD		48
#define RECV_RESET_CMD		64
#define SEND_REMOTE_CMD		242
#define SEND_SCRIPT_CMD		243

#define LOGIC0 1
#define LOGIC1 0

#define LED			GPIO_NUM_21
#define PIN_RES		GPIO_NUM_5
#define PIN_BOOT	GPIO_NUM_18
#define BUTTON		GPIO_NUM_4

struct CUSTOM_IR
{
	uint8_t number_section_ir;
	uint8_t size_section_ir[10];
	uint16_t logic_ir[4];
	uint16_t section_ir[40];
	uint64_t result_section_ir[4];
};

extern uint8_t request[8];
extern uint8_t memory[512];
extern uint8_t lockdevice;
extern uint8_t isota;
extern uint16_t gateway;
extern uint8_t params[100];
extern uint8_t ssid[32];
extern uint8_t password[64];
extern uint16_t wakeupvoice;
extern uint16_t indexresponse;
extern uint16_t* remote_param_ir;
extern uint64_t* remote_code0_ir;
extern uint64_t* remote_code1_ir;
extern uint64_t* remote_code2_ir;
extern uint64_t* remote_code3_ir;
extern struct CUSTOM_IR custom_ir;
extern RingbufHandle_t ring_buffer_ir;
extern rmt_item32_t* items_tx_ir;
extern uint16_t number_tx_ir;
extern const esp_partition_t* partition;
extern const esp_partition_t* ota_partition;
extern const esp_partition_t* config_partition;
extern uint16_t timewaittingresponse;

#ifdef __cplusplus
extern "C" {
#endif
// file wifi.c
void wifi_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
uint8_t wifi_init();
esp_err_t client_event_get_handler(esp_http_client_event_handle_t evt);
uint8_t get_wifi_data(uint8_t index_file);
uint8_t ota_firmware();
uint8_t ota_data();
uint8_t ota_voice();
//file bluetooth_mesh.c
void set_queue(uint8_t tmpqueue[]);
void get_queue();
void update_queue(uint8_t idl, uint8_t idh);
uint8_t check_active(uint8_t ref[]);
void ble_mesh_provisioning_cb(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t *param);
void ble_mesh_custom_model_cb(esp_ble_mesh_model_cb_event_t event, esp_ble_mesh_model_cb_param_t *param);
void ble_mesh_send(uint16_t addr, uint32_t opcode, uint16_t length, uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7);
void ble_debug_send(uint16_t addr, uint32_t opcode, uint16_t length, uint8_t* debug);
void ble_mesh_init();
void reset_relay_list(uint16_t addr);
void print_relay_list();
void remove_relay_list(uint16_t addr);
uint8_t count_relay_list();
//file file.c
void clear_memory_system();
void load_memory_system();
void save_memory_system();
void load_flash(uint8_t idl, uint8_t idh, uint8_t index);
void save_flash();
void clear_flash(uint8_t idl, uint8_t idh);
uint8_t find_list_remote(uint8_t idl, uint8_t idh, uint8_t index);
uint8_t check_list_remote(uint8_t idl, uint8_t idh, uint8_t index);
void send_list_remote();
uint8_t check_params_remote();
uint8_t add_params_remote(uint8_t type, uint8_t idl, uint8_t idh, uint8_t index);
uint8_t delete_params_remote(uint8_t idl, uint8_t idh, uint8_t index);
uint8_t delete_all_params_remote();
uint8_t find_type_raw(uint8_t idl, uint8_t idh);
void free_cache_memory();
uint8_t find_raw_key(uint8_t* matrix, uint8_t idl, uint8_t idh, uint8_t key);
uint8_t check_raw_key(uint8_t idl, uint8_t idh);
uint8_t delete_raw_key(uint8_t idl, uint8_t idh, uint8_t key);
uint8_t add_raw_key(rmt_item32_t* items, uint16_t number, uint8_t idl, uint8_t idh, uint8_t key);
uint8_t choose_raw_key(uint8_t idl, uint8_t idh, uint8_t temp, uint8_t mode, uint8_t fan);
void load_custom_remote(uint8_t index);
void delete_custom_remote();
uint8_t add_script(uint8_t idl, uint8_t idh, uint8_t info1, uint8_t info2, uint8_t info3, uint8_t info4, uint8_t info5);
uint8_t delete_script(uint8_t idl, uint8_t idh, uint8_t info1, uint8_t info2, uint8_t info3, uint8_t info4);
void delete_action(uint8_t idl, uint8_t idh, uint8_t index);
uint8_t delete_all_script(uint8_t idl, uint8_t idh);
uint8_t find_script(uint8_t idl, uint8_t idh);
void set_status_script(uint8_t idl, uint8_t idh, uint8_t status);
void send_list_script();
uint8_t save_wifi(uint8_t info[]);
uint8_t save_ota(uint8_t type, uint32_t file1, uint32_t file2);
void save_status_remote(uint8_t idl, uint8_t idh, uint8_t index, uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing);
uint8_t load_status_remote(uint8_t idl, uint8_t idh, uint8_t index, uint8_t *status);
void set_position_remote(uint8_t idl, uint8_t idh, uint8_t index, uint8_t* position);
void load_position_remote(uint8_t idl, uint8_t idh, uint8_t index, uint8_t *position);
void find_power_custom(uint8_t index);
void find_power_raw(uint8_t index);
void kick_out();
//file remote_ir.c
void ir_init();
void send_buffer_ir();
uint8_t send_custom_ir(uint8_t idl, uint8_t idh, uint8_t index, uint8_t key);
uint8_t send_raw_ir(uint8_t idl, uint8_t idh, uint8_t key);
uint8_t send_memory_ir(uint8_t idl, uint8_t idh, uint8_t index, uint8_t key);
uint8_t decode_custom_ir(rmt_item32_t* items, uint16_t number);
uint8_t find_memory_remote(rmt_item32_t* items, uint16_t number, uint8_t statusdecode);
uint8_t find_custom_remote(uint8_t index);
uint8_t find_raw_remote(rmt_item32_t* items, uint16_t number, uint8_t index);
//file serial.c
void uart_init();
void uart_send_hex(uint64_t n);
void uart_send_dec(uint64_t n);
void uart_send_bin(uint64_t n);
void uart_send_custom_ir(struct CUSTOM_IR custom_ir);
void uart_send_raw_ir(rmt_item32_t* items, uint8_t number);
//file main.c
uint8_t send_ac_ir(uint8_t idl, uint8_t idh, uint8_t index, uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing);
void fill_recv_ir(rmt_item32_t* items, uint8_t number);
uint8_t find_ac_remote();
uint8_t find_ac_custom_remote();
uint8_t find_ac_teaching();
void find_voice_remote(uint8_t type, uint16_t command);
uint8_t run_script(uint16_t id);
#ifdef __cplusplus
}
#endif

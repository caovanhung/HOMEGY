#include "IRremoteESP8266.h"
#include "ir_Carrier.h"
#include "ir_Coolix.h"
#include "ir_Daikin.h"
#include "ir_Electra.h"
#include "ir_Fujitsu.h"
#include "ir_Gree.h"
#include "ir_Hitachi.h"
#include "ir_LG.h"
#include "ir_Midea.h"
#include "ir_Mitsubishi.h"
#include "ir_MitsubishiHeavy.h"
#include "ir_Panasonic.h"
#include "ir_Samsung.h"
#include "ir_Sanyo.h"
#include "ir_Sharp.h"
#include "ir_Tcl.h"
#include "ir_Toshiba.h"
#include "ir_Airton.h"
#include "ir_Airwell.h"
#include "ir_Amcor.h"
#include "ir_Argo.h"
#include "ir_Bosch.h"
#include "ir_Corona.h"
#include "ir_Delonghi.h"
#include "ir_Ecoclim.h"
#include "ir_Goodweather.h"
#include "ir_Haier.h"
#include "ir_Kelon.h"
#include "ir_Kelvinator.h"
#include "ir_Mirage.h"
#include "ir_Neoclima.h"
#include "ir_Rhoss.h"
#include "ir_Technibel.h"
#include "ir_Teco.h"
#include "ir_Transcold.h"
#include "ir_Trotec.h"
#include "ir_Truma.h"
#include "ir_Vestel.h"
#include "ir_Voltas.h"
#include "ir_Whirlpool.h"
#include "IRsend.h"
#include "IRrecv.h"
#include "esp_log.h"

#define _PANASONIC_AC	22
#define _DAIKIN_AC		24
#define _TOSHIBA_AC		26
#define _SAMSUNG_AC		28	
#define _LG_AC			30
#define _SHARP_AC		32
#define _AQUA_AC		34
#define _GREE_AC		36
#define _MIDEA_AC		40
#define _TCL_AC			42
#define _MITSUBISHI_AC	44
#define _HITACHI_AC		48
#define _ASANZO_AC		52
#define _CARRIER_AC		54
#define _FUJITSU_AC		56
#define _AIRTON_AC		58
#define _AIRWELL_AC		60
#define _AMCOR_AC		62
#define _ARGO_AC		64
#define _BOSCH_AC		66
#define _CORONA_AC		68
#define _DELONGHI_AC	70
#define _ECOCLIM_AC		72
#define _GOODWEATHER_AC	74
#define _HAIER_AC		76
#define _KELON_AC		78
#define _KELVINATOR_AC	80
#define _MIRAGE_AC		82
#define _NEOCLIMA_AC	84
#define _RHOSS_AC		86
#define _TECHNIBEL_AC	88
#define _TECO_AC		90
#define _TRANSCOLD_AC	92
#define _TROTEC_AC		94
#define _TRUMA_AC		96
#define _VESTEL_AC		98
#define _VOLTAS_AC		100
#define _WHIRLPOOL_AC	102
#define _BEKO_AC		104
#define _ELECTRA_AC		106
#define _HISENSE_AC		108
#define _COMFEE_AC		110
#define _ALASKA_AC		112
#define _COOLIX_AC		114

//#define TOKIO_AC		108
//#define KASTRON_AC		110
//#define KAYSUN_AC		112
//#define AUX_AC			114
//#define FRIGIDAIRE_AC	116
//#define SUBTROPIC_AC	118
//#define CENTEK_AC		120
//#define AEG_AC			122
//#define GENERAL_AC		124
//#define ULTIMATEL_AC	126
//#define EKOKAI_AC		128
//#define GREEN_AC		130
//#define AMANA_AC		132
//#define CH_AC			134
//#define VAILLANT_AC		136
//#define SOLEUSAIR_AC	138
//#define MABE_AC			140
//#define PIONEER_AC		144
//#define KEYSTONE_AC		148
//#define MRCOOL_AC		150
//#define DANBY_AC		152
//#define LENNOX_AC		154
//#define MAXELL_AC		156
//#define TRONITECHNIK_AC	158
//#define LEBERG_AC		160
//#define TEKNOPOINT_AC	162
//#define DAEWOO_AC		164
//#define DUUX_AC			170

IRrecv irrecv(15, 1024, 200);
decode_results results;
uint8_t (*decode_ac)(uint8_t* response) = NULL;

uint8_t decode_daikin_custom1_ac(uint8_t* response);

uint8_t reserval_byte(uint8_t input)
{
	uint8_t i;
	uint8_t t;
	for(i=0; i<4; i++)
	{
		t = ((input >> i) & 0x01) ^ ((input >> (7-i)) & 0x01);
		input ^= t << i;
		input ^= t << (7-i);
	}
	return input;
}

uint8_t decode_electra_ac(uint8_t* response)
{
	if(results.decode_type != decode_type_t::ELECTRA_AC) return 0;
	IRElectraAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kElectraAcFanAuto:
			response[1] = 1;
			break;
		case kElectraAcDry:
			response[1] = 3;
			break;
		case kElectraAcHeat:
			response[1] = 4;
			break;
		case kElectraAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kElectraAcFanLow:
			response[2] = 2;
			break;
		case kElectraAcFanMed:
			response[2] = 3;
			break;
		case kElectraAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingH() << 1) | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_coolix_ac(uint8_t* response)
{
	if(results.decode_type != COOLIX) return 0;
	IRCoolixAC ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kCoolixAuto:
			response[1] = 1;
			break;
		case kCoolixDry:
			response[1] = 3;
			break;
		case kCoolixHeat:
			response[1] = 4;
			break;
		case kElectraAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kCoolixFanMin:
			response[2] = 2;
			break;
		case kCoolixFanMed:
			response[2] = 3;
			break;
		case kCoolixFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwing() << 1) | (ac.getSwingVStep() << 2);
	return 1;
}

uint8_t decode_midea_ac(uint8_t* response)
{
	if(results.decode_type != MIDEA) return 0;
	IRMideaAC ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kMideaACAuto:
			response[1] = 1;
			break;
		case kMideaACDry:
			response[1] = 3;
			break;
		case kMideaACHeat:
			response[1] = 4;
			break;
		case kMideaACFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kMideaACFanLow:
			response[2] = 2;
			break;
		case kMideaACFanMed:
			response[2] = 3;
			break;
		case kMideaACFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingVToggle() << 2);
	return 1;
}

uint8_t decode_carrier64_ac(uint8_t* response)
{
	if(results.decode_type != CARRIER_AC64) return 0;
	IRCarrierAc64 ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kCarrierAc64Heat:
			response[1] = 4;
			break;
		case kCarrierAc64Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kCarrierAc64FanLow:
			response[2] = 2;
			break;
		case kCarrierAc64FanMedium:
			response[2] = 3;
			break;
		case kCarrierAc64FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_daikinesp_ac(uint8_t* response)
{
	if(results.decode_type != DAIKIN) return 0;
	IRDaikinESP ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDaikinAuto:
			response[1] = 1;
			break;
		case kDaikinDry:
			response[1] = 3;
			break;
		case kDaikinHeat:
			response[1] = 4;
			break;
		case kDaikinFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDaikinFanMin:
			response[2] = 2;
			break;
		case kDaikinFanMed:
			response[2] = 3;
			break;
		case kDaikinFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingHorizontal() << 1) | (ac.getSwingVertical() << 2);
	return 1;
}

uint8_t decode_daikin2_ac(uint8_t* response)
{
	if(results.decode_type != DAIKIN2) return 0;
	IRDaikin2 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDaikinAuto:
			response[1] = 1;
			break;
		case kDaikinDry:
			response[1] = 3;
			break;
		case kDaikinHeat:
			response[1] = 4;
			break;
		case kDaikinFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDaikinFanMin:
			response[2] = 2;
			break;
		case kDaikinFanMed:
			response[2] = 3;
			break;
		case kDaikinFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_daikin216_ac(uint8_t* response)
{
	if(results.decode_type != DAIKIN216) return 0;
	IRDaikin216 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDaikinAuto:
			response[1] = 1;
			break;
		case kDaikinDry:
			response[1] = 3;
			break;
		case kDaikinHeat:
			response[1] = 4;
			break;
		case kDaikinFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDaikinFanMin:
			response[2] = 2;
			break;
		case kDaikinFanMed:
			response[2] = 3;
			break;
		case kDaikinFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingHorizontal() << 1) | (ac.getSwingVertical() << 2);
	return 1;
}

uint8_t decode_daikin160_ac(uint8_t* response)
{
	if(results.decode_type != DAIKIN160) return 0;
	IRDaikin160 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDaikinAuto:
			response[1] = 1;
			break;
		case kDaikinDry:
			response[1] = 3;
			break;
		case kDaikinHeat:
			response[1] = 4;
			break;
		case kDaikinFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDaikinFanMin:
			response[2] = 2;
			break;
		case kDaikinFanMed:
			response[2] = 3;
			break;
		case kDaikinFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_daikin176_ac(uint8_t* response)
{
	if(results.decode_type != DAIKIN176) return 0;
	IRDaikin176 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDaikin176Auto:
			response[1] = 1;
			break;
		case kDaikin176Dry:
			response[1] = 3;
			break;
		case kDaikin176Heat:
			response[1] = 4;
			break;
		case kDaikin176Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDaikinFanMin:
			response[2] = 2;
			break;
		case kDaikin176FanMax:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_daikin128_ac(uint8_t* response)
{
	if(results.decode_type != DAIKIN128) return 0;
	IRDaikin128 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPowerToggle()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDaikin128Auto:
			response[1] = 1;
			break;
		case kDaikin128Dry:
			response[1] = 3;
			break;
		case kDaikin128Heat:
			response[1] = 4;
			break;
		case kDaikin128Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDaikin128FanLow:
			response[2] = 2;
			break;
		case kDaikin128FanMed:
			response[2] = 3;
			break;
		case kDaikin128FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingVertical() << 2);
	return 1;
}

uint8_t decode_daikin152_ac(uint8_t* response)
{
	if(results.decode_type != DAIKIN152) return 0;
	IRDaikin152 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDaikinAuto:
			response[1] = 1;
			break;
		case kDaikinDry:
			response[1] = 3;
			break;
		case kDaikinHeat:
			response[1] = 4;
			break;
		case kDaikinFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDaikinFanMin:
			response[2] = 2;
			break;
		case kDaikinFanMed:
			response[2] = 3;
			break;
		case kDaikinFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_daikin64_ac(uint8_t* response)
{
	if(results.decode_type != DAIKIN64) return 0;
	IRDaikin64 ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPowerToggle()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDaikin64Dry:
			response[1] = 3;
			break;
		case kDaikin64Heat:
			response[1] = 4;
			break;
		case kDaikin64Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDaikin64FanLow:
			response[2] = 2;
			break;
		case kDaikin64FanMed:
			response[2] = 3;
			break;
		case kDaikin64FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingVertical() << 2);
	return 1;
}

uint8_t decode_fujitsu_ac(uint8_t* response)
{
	if(results.decode_type != FUJITSU_AC) return 0;
	IRFujitsuAC ac(-1);
	ac.setRaw(results.state, results.bits / 8);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kFujitsuAcModeAuto:
			response[1] = 1;
			break;
		case kFujitsuAcModeDry:
			response[1] = 3;
			break;
		case kFujitsuAcModeHeat:
			response[1] = 4;
			break;
		case kFujitsuAcModeFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFanSpeed())
	{
		case kFujitsuAcFanLow:
			response[2] = 2;
			break;
		case kFujitsuAcFanMed:
			response[2] = 3;
			break;
		case kFujitsuAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_gree_ac(uint8_t* response)
{
	if(results.decode_type != GREE) return 0;
	IRGreeAC ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kGreeAuto:
			response[1] = 1;
			break;
		case kGreeDry:
			response[1] = 3;
			break;
		case kGreeHeat:
			response[1] = 4;
			break;
		case kGreeFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kGreeFanMin:
			response[2] = 2;
			break;
		case kGreeFanMed:
			response[2] = 3;
			break;
		case kGreeFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_hitachi_ac(uint8_t* response)
{
	if(results.decode_type != HITACHI_AC) return 0;
	IRHitachiAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHitachiAcAuto:
			response[1] = 1;
			break;
		case kHitachiAcDry:
			response[1] = 3;
			break;
		case kHitachiAcHeat:
			response[1] = 4;
			break;
		case kHitachiAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHitachiAcFanLow:
			response[2] = 2;
			break;
		case kHitachiAcFanMed:
			response[2] = 3;
			break;
		case kHitachiAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingHorizontal() << 1) | (ac.getSwingVertical() << 2);
	return 1;
}

uint8_t decode_hitachi1_ac(uint8_t* response)
{
	if(results.decode_type != HITACHI_AC1) return 0;
	IRHitachiAc1 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHitachiAc1Auto:
			response[1] = 1;
			break;
		case kHitachiAc1Dry:
			response[1] = 3;
			break;
		case kHitachiAc1Heat:
			response[1] = 4;
			break;
		case kHitachiAc1Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHitachiAc1FanLow:
			response[2] = 2;
			break;
		case kHitachiAc1FanMed:
			response[2] = 3;
			break;
		case kHitachiAc1FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingH() << 1) | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_hitachi424_ac(uint8_t* response)
{
	if(results.decode_type != HITACHI_AC424) return 0;
	IRHitachiAc424 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHitachiAc424Dry:
			response[1] = 3;
			break;
		case kHitachiAc424Heat:
			response[1] = 4;
			break;
		case kHitachiAc424Fan:
			response[1] = 5;
			break;
	}
	response[3] = 1;
	switch(ac.getFan())
	{
		case 2:
			response[3] = 2;
			break;
		case 3:
			response[3] = 3;
			break;
		case 4:
			response[3] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingVToggle() << 2);
	return 1;
}

uint8_t decode_hitachi344_ac(uint8_t* response)
{
	if(results.decode_type != HITACHI_AC344) return 0;
	IRHitachiAc344 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHitachiAc344Dry:
			response[1] = 3;
			break;
		case kHitachiAc344Heat:
			response[1] = 4;
			break;
		case kHitachiAc344Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHitachiAc344FanLow:
			response[2] = 2;
			break;
		case kHitachiAc344FanMedium:
			response[2] = 3;
			break;
		case kHitachiAc344FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_hitachi264_ac(uint8_t* response)
{
	if(results.decode_type != HITACHI_AC264) return 0;
	IRHitachiAc264 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHitachiAc264Dry:
			response[1] = 3;
			break;
		case kHitachiAc264Heat:
			response[1] = 4;
			break;
		case kHitachiAc264Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHitachiAc264FanLow:
			response[2] = 2;
			break;
		case kHitachiAc264FanMedium:
			response[2] = 3;
			break;
		case kHitachiAc264FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_hitachi296_ac(uint8_t* response)
{
	if(results.decode_type != HITACHI_AC296) return 0;
	IRHitachiAc296 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHitachiAc296Auto:
			response[1] = 1;
			break;
		case kHitachiAc296Heat:
			response[1] = 4;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHitachiAc296FanLow:
			response[2] = 2;
			break;
		case kHitachiAc296FanMedium:
			response[2] = 3;
			break;
		case kHitachiAc296FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_panasonic_ac(uint8_t* response)
{
	if(results.decode_type != PANASONIC_AC) return 0;
	IRPanasonicAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kPanasonicAcAuto:
			response[1] = 1;
			break;
		case kPanasonicAcDry:
			response[1] = 3;
			break;
		case kPanasonicAcHeat:
			response[1] = 4;
			break;
		case kPanasonicAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kPanasonicAcFanLow:
			response[2] = 2;
			break;
		case kPanasonicAcFanMed:
			response[2] = 3;
			break;
		case kPanasonicAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_panasonic32_ac(uint8_t* response)
{
	if(results.decode_type != PANASONIC_AC32) return 0;
	IRPanasonicAc32 ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPowerToggle()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kPanasonicAc32Auto:
			response[1] = 1;
			break;
		case kPanasonicAc32Dry:
			response[1] = 3;
			break;
		case kPanasonicAc32Heat:
			response[1] = 4;
			break;
		case kPanasonicAc32Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kPanasonicAc32FanLow:
			response[2] = 2;
			break;
		case kPanasonicAc32FanMed:
			response[2] = 3;
			break;
		case kPanasonicAc32FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingHorizontal() << 1);
	return 1;
}

uint8_t decode_toshiba_ac(uint8_t* response)
{
	if(results.decode_type != TOSHIBA_AC) return 0;
	IRToshibaAC ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kToshibaAcAuto:
			response[1] = 1;
			break;
		case kToshibaAcDry:
			response[1] = 3;
			break;
		case kToshibaAcHeat:
			response[1] = 4;
			break;
		case kToshibaAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kToshibaAcFanMin:
			response[2] = 2;
			break;
		case kToshibaAcFanMed:
			response[2] = 3;
			break;
		case kToshibaAcFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_tcl_ac(uint8_t* response)
{
	if(results.decode_type != TCL112AC) return 0;
	IRTcl112Ac ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kTcl112AcAuto:
			response[1] = 1;
			break;
		case kTcl112AcDry:
			response[1] = 3;
			break;
		case kTcl112AcHeat:
			response[1] = 4;
			break;
		case kTcl112AcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kTcl112AcFanLow:
			response[2] = 2;
			break;
		case kTcl112AcFanMed:
			response[2] = 3;
			break;
		case kTcl112AcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingHorizontal() << 1);
	return 1;
}

uint8_t decode_mitsubishi_ac(uint8_t* response)
{
	if(results.decode_type != MITSUBISHI_AC) return 0;
	IRMitsubishiAC ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kMitsubishiAcAuto:
			response[1] = 1;
			break;
		case kMitsubishiAcDry:
			response[1] = 3;
			break;
		case kMitsubishiAcHeat:
			response[1] = 4;
			break;
		case kMitsubishiAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kMitsubishiAcFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_mitsubishi136_ac(uint8_t* response)
{
	if(results.decode_type != MITSUBISHI136) return 0;
	IRMitsubishi136 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kMitsubishi136Auto:
			response[1] = 1;
			break;
		case kMitsubishi136Dry:
			response[1] = 3;
			break;
		case kMitsubishi136Heat:
			response[1] = 4;
			break;
		case kMitsubishi136Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kMitsubishi136FanMin:
			response[2] = 2;
			break;
		case kMitsubishi136FanMed:
			response[2] = 3;
			break;
		case kMitsubishi136FanMax:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_mitsubishi112_ac(uint8_t* response)
{
	if(results.decode_type != MITSUBISHI112) return 0;
	IRMitsubishi112 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kMitsubishi112Auto:
			response[1] = 1;
			break;
		case kMitsubishi112Dry:
			response[1] = 3;
			break;
		case kMitsubishi112Heat:
			response[1] = 4;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kMitsubishi112FanMin:
			response[2] = 2;
			break;
		case kMitsubishi112FanMed:
			response[2] = 3;
			break;
		case kMitsubishi112FanMax:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_mitsubishi152_ac(uint8_t* response)
{
	if(results.decode_type != MITSUBISHI_HEAVY_152) return 0;
	IRMitsubishiHeavy152Ac ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kMitsubishiHeavyAuto:
			response[1] = 1;
			break;
		case kMitsubishiHeavyDry:
			response[1] = 3;
			break;
		case kMitsubishiHeavyHeat:
			response[1] = 4;
			break;
		case kMitsubishiHeavyFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kMitsubishiHeavy152FanLow:
			response[2] = 2;
			break;
		case kMitsubishiHeavy152FanMed:
			response[2] = 3;
			break;
		case kMitsubishiHeavy152FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_mitsubishi88_ac(uint8_t* response)
{
	if(results.decode_type != MITSUBISHI_HEAVY_88) return 0;
	IRMitsubishiHeavy88Ac ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kMitsubishiHeavyAuto:
			response[1] = 1;
			break;
		case kMitsubishiHeavyDry:
			response[1] = 3;
			break;
		case kMitsubishiHeavyHeat:
			response[1] = 4;
			break;
		case kMitsubishiHeavyFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kMitsubishiHeavy88FanLow:
			response[2] = 2;
			break;
		case kMitsubishiHeavy88FanMed:
			response[2] = 3;
			break;
		case kMitsubishiHeavy88FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_sanyo_ac(uint8_t* response)
{
	if(results.decode_type != SANYO_AC) return 0;
	IRSanyoAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kSanyoAcAuto:
			response[1] = 1;
			break;
		case kSanyoAcDry:
			response[1] = 3;
			break;
		case kSanyoAcHeat:
			response[1] = 4;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kSanyoAcFanLow:
			response[2] = 2;
			break;
		case kSanyoAcFanMedium:
			response[2] = 3;
			break;
		case kSanyoAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 1;
	return 1;
}

uint8_t decode_sanyo88_ac(uint8_t* response)
{
	if(results.decode_type != SANYO_AC88) return 0;
	IRSanyoAc88 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kSanyoAc88Auto:
			response[1] = 1;
			break;
		case kSanyoAc88Heat:
			response[1] = 4;
			break;
		case kSanyoAc88Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kSanyoAc88FanLow:
			response[2] = 2;
			break;
		case kSanyoAc88FanMedium:
			response[2] = 3;
			break;
		case kSanyoAc88FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_sharp_ac(uint8_t* response)
{
	if(results.decode_type != SHARP_AC) return 0;
	IRSharpAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kSharpAcAuto:
			response[1] = 1;
			break;
		case kSharpAcDry:
			response[1] = 3;
			break;
		case kSharpAcHeat:
			response[1] = 4;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kSharpAcFanMin:
			response[2] = 2;
			break;
		case kSharpAcFanMed:
			response[2] = 3;
			break;
		case kSharpAcFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingToggle() << 2);
	return 1;
}

uint8_t decode_lg_ac(uint8_t* response)
{
	if(results.decode_type != LG) return 0;
	IRLgAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kLgAcAuto:
			response[1] = 1;
			break;
		case kLgAcDry:
			response[1] = 3;
			break;
		case kLgAcHeat:
			response[1] = 4;
			break;
		case kLgAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kLgAcFanLow:
			response[2] = 2;
			break;
		case kLgAcFanMedium:
			response[2] = 3;
			break;
		case kLgAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingH() << 1);
	return 1;
}

uint8_t decode_lg2_ac(uint8_t* response)
{
	if(results.decode_type != LG2) return 0;
	IRLgAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0] = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kLgAcAuto:
			response[1] = 1;
			break;
		case kLgAcDry:
			response[1] = 3;
			break;
		case kLgAcHeat:
			response[1] = 4;
			break;
		case kLgAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kLgAcFanLow:
			response[2] = 2;
			break;
		case kLgAcFanMedium:
			response[2] = 3;
			break;
		case kLgAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingH() << 1);
	return 1;
}

uint8_t decode_samsung_ac(uint8_t* response)
{
	if(results.decode_type != SAMSUNG_AC) return 0;
	IRSamsungAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kSamsungAcAuto:
			response[1] = 1;
			break;
		case kSamsungAcCool:
			response[1] = 2;
			break;
		case kSamsungAcDry:
			response[1] = 3;
			break;
		case kSamsungAcHeat:
			response[1] = 4;
			break;
		case kSamsungAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kSamsungAcFanAuto:
			response[2] = 1;
			break;
		case kSamsungAcFanLow:
			response[2] = 2;
			break;
		case kSamsungAcFanMed:
			response[2] = 3;
			break;
		case kSamsungAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingH() << 1) | (ac.getSwing() << 2);
	return 1;
}

uint8_t decode_airton_ac(uint8_t* response)
{
	if(results.decode_type != AIRTON) return 0;
	IRAirtonAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kAirtonAuto:
			response[1] = 1;
			break;
		case kAirtonCool:
			response[1] = 2;
			break;
		case kAirtonDry:
			response[1] = 3;
			break;
		case kAirtonHeat:
			response[1] = 4;
			break;
		case kAirtonFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kAirtonFanAuto:
			response[2] = 1;
			break;
		case kAirtonFanLow:
			response[2] = 2;
			break;
		case kAirtonFanMed:
			response[2] = 3;
			break;
		case kAirtonFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_airwell_ac(uint8_t* response)
{
	if(results.decode_type != AIRWELL) return 0;
	IRAirwellAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPowerToggle()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kAirwellAuto:
			response[1] = 1;
			break;
		case kAirwellCool:
			response[1] = 2;
			break;
		case kAirwellDry:
			response[1] = 3;
			break;
		case kAirwellHeat:
			response[1] = 4;
			break;
		case kAirwellFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kAirwellFanAuto:
			response[2] = 1;
			break;
		case kAirwellFanLow:
			response[2] = 2;
			break;
		case kAirwellFanMedium:
			response[2] = 3;
			break;
		case kAirwellFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_amcor_ac(uint8_t* response)
{
	if(results.decode_type != AMCOR) return 0;
	IRAmcorAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kAmcorAuto:
			response[1] = 1;
			break;
		case kAmcorCool:
			response[1] = 2;
			break;
		case kAmcorDry:
			response[1] = 3;
			break;
		case kAmcorHeat:
			response[1] = 4;
			break;
		case kAmcorFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kAmcorFanAuto:
			response[2] = 1;
			break;
		case kAmcorFanMin:
			response[2] = 2;
			break;
		case kAmcorFanMed:
			response[2] = 3;
			break;
		case kAmcorFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_argo_ac(uint8_t* response)
{
	if(results.decode_type != ARGO) return 0;
	IRArgoAC ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kArgoAuto:
			response[1] = 1;
			break;
		case kArgoCool:
			response[1] = 2;
			break;
		case kArgoDry:
			response[1] = 3;
			break;
		case kArgoHeat:
			response[1] = 4;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kArgoFanAuto:
			response[2] = 1;
			break;
		case kArgoFan1:
			response[2] = 2;
			break;
		case kArgoFan2:
			response[2] = 3;
			break;
		case kArgoFan3:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_bosch_ac(uint8_t* response)
{
	if(results.decode_type != BOSCH144) return 0;
	IRBosch144AC ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kBosch144Auto:
			response[1] = 1;
			break;
		case kBosch144Cool:
			response[1] = 2;
			break;
		case kBosch144Dry:
			response[1] = 3;
			break;
		case kBosch144Heat:
			response[1] = 4;
			break;
		case kBosch144Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kBosch144FanAuto:
			response[2] = 1;
			break;
		case kBosch144Fan40:
			response[2] = 2;
			break;
		case kBosch144Fan60:
			response[2] = 3;
			break;
		case kBosch144Fan80:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_corona_ac(uint8_t* response)
{
	if(results.decode_type != CORONA_AC) return 0;
	IRCoronaAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kCoronaAcModeCool:
			response[1] = 2;
			break;
		case kCoronaAcModeDry:
			response[1] = 3;
			break;
		case kCoronaAcModeHeat:
			response[1] = 4;
			break;
		case kCoronaAcModeFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kCoronaAcFanAuto:
			response[2] = 1;
			break;
		case kCoronaAcFanLow:
			response[2] = 2;
			break;
		case kCoronaAcFanMedium:
			response[2] = 3;
			break;
		case kCoronaAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingVToggle() << 2);
	return 1;
}

uint8_t decode_delonghi_ac(uint8_t* response)
{
	if(results.decode_type != DELONGHI_AC) return 0;
	IRDelonghiAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kDelonghiAcAuto:
			response[1] = 1;
			break;
		case kDelonghiAcCool:
			response[1] = 2;
			break;
		case kDelonghiAcDry:
			response[1] = 3;
			break;
		case kDelonghiAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kDelonghiAcFanAuto:
			response[2] = 1;
			break;
		case kDelonghiAcFanLow:
			response[2] = 2;
			break;
		case kDelonghiAcFanMedium:
			response[2] = 3;
			break;
		case kDelonghiAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_ecoclim_ac(uint8_t* response)
{
	if(results.decode_type != ECOCLIM) return 0;
	IREcoclimAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kEcoclimAuto:
			response[1] = 1;
			break;
		case kEcoclimCool:
			response[1] = 2;
			break;
		case kEcoclimDry:
			response[1] = 3;
			break;
		case kEcoclimHeat:
			response[1] = 4;
			break;
		case kEcoclimFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kEcoclimFanAuto:
			response[2] = 1;
			break;
		case kEcoclimFanMin:
			response[2] = 2;
			break;
		case kEcoclimFanMed:
			response[2] = 3;
			break;
		case kEcoclimFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_goodweather_ac(uint8_t* response)
{
	if(results.decode_type != GOODWEATHER) return 0;
	IRGoodweatherAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kGoodweatherAuto:
			response[1] = 1;
			break;
		case kGoodweatherCool:
			response[1] = 2;
			break;
		case kGoodweatherDry:
			response[1] = 3;
			break;
		case kGoodweatherHeat:
			response[1] = 4;
			break;
		case kGoodweatherFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kGoodweatherFanAuto:
			response[2] = 1;
			break;
		case kGoodweatherFanLow:
			response[2] = 2;
			break;
		case kGoodweatherFanMed:
			response[2] = 3;
			break;
		case kGoodweatherFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_haier_ac(uint8_t* response)
{
	if(results.decode_type != HAIER_AC) return 0;
	IRHaierAC ac(-1);
	ac.setRaw(results.state);
	if(!ac.getCommand()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHaierAcAuto:
			response[1] = 1;
			break;
		case kHaierAcCool:
			response[1] = 2;
			break;
		case kHaierAcDry:
			response[1] = 3;
			break;
		case kHaierAcHeat:
			response[1] = 4;
			break;
		case kHaierAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHaierAcFanAuto:
			response[2] = 1;
			break;
		case kHaierAcFanLow:
			response[2] = 2;
			break;
		case kHaierAcFanMed:
			response[2] = 3;
			break;
		case kHaierAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_haier176_ac(uint8_t* response)
{
	if(results.decode_type != HAIER_AC176) return 0;
	IRHaierAC176 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHaierAcYrw02Auto:
			response[1] = 1;
			break;
		case kHaierAcYrw02Cool:
			response[1] = 2;
			break;
		case kHaierAcYrw02Dry:
			response[1] = 3;
			break;
		case kHaierAcYrw02Heat:
			response[1] = 4;
			break;
		case kHaierAcYrw02Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHaierAcYrw02FanAuto:
			response[2] = 1;
			break;
		case kHaierAcYrw02FanLow:
			response[2] = 2;
			break;
		case kHaierAcYrw02FanMed:
			response[2] = 3;
			break;
		case kHaierAcYrw02FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_haieryrw02_ac(uint8_t* response)
{
	if(results.decode_type != HAIER_AC_YRW02) return 0;
	IRHaierACYRW02 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHaierAcYrw02Auto:
			response[1] = 1;
			break;
		case kHaierAcYrw02Cool:
			response[1] = 2;
			break;
		case kHaierAcYrw02Dry:
			response[1] = 3;
			break;
		case kHaierAcYrw02Heat:
			response[1] = 4;
			break;
		case kHaierAcYrw02Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHaierAcYrw02FanAuto:
			response[2] = 1;
			break;
		case kHaierAcYrw02FanLow:
			response[2] = 2;
			break;
		case kHaierAcYrw02FanMed:
			response[2] = 3;
			break;
		case kHaierAcYrw02FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_haier160_ac(uint8_t* response)
{
	if(results.decode_type != HAIER_AC160) return 0;
	IRHaierAC160 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kHaierAcYrw02Auto:
			response[1] = 1;
			break;
		case kHaierAcYrw02Cool:
			response[1] = 2;
			break;
		case kHaierAcYrw02Dry:
			response[1] = 3;
			break;
		case kHaierAcYrw02Heat:
			response[1] = 4;
			break;
		case kHaierAcYrw02Fan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kHaierAcYrw02FanAuto:
			response[2] = 1;
			break;
		case kHaierAcYrw02FanLow:
			response[2] = 2;
			break;
		case kHaierAcYrw02FanMed:
			response[2] = 3;
			break;
		case kHaierAcYrw02FanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_kelon_ac(uint8_t* response)
{
	if(results.decode_type != KELON) return 0;
	IRKelonAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getTogglePower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kKelonModeCool:
			response[1] = 2;
			break;
		case kKelonModeDry:
			response[1] = 3;
			break;
		case kKelonModeHeat:
			response[1] = 4;
			break;
		case kKelonModeFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kKelonFanAuto:
			response[2] = 1;
			break;
		case kKelonFanMin:
			response[2] = 2;
			break;
		case kKelonFanMedium:
			response[2] = 3;
			break;
		case kKelonFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getToggleSwingVertical() << 2);
	return 1;
}

uint8_t decode_kelvinator_ac(uint8_t* response)
{
	if(results.decode_type != KELVINATOR) return 0;
	IRKelvinatorAC ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kKelvinatorAuto:
			response[1] = 1;
			break;
		case kKelvinatorCool:
			response[1] = 2;
			break;
		case kKelvinatorDry:
			response[1] = 3;
			break;
		case kKelvinatorHeat:
			response[1] = 4;
			break;
		case kKelvinatorFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kKelvinatorFanAuto:
			response[2] = 1;
			break;
		case kKelvinatorFanMin:
			response[2] = 2;
			break;
		case kKelvinatorFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingHorizontal() << 1);
	return 1;
}

uint8_t decode_mirage_ac(uint8_t* response)
{
	if(results.decode_type != MIRAGE) return 0;
	IRMirageAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kMirageAcCool:
			response[1] = 2;
			break;
		case kMirageAcDry:
			response[1] = 3;
			break;
		case kMirageAcHeat:
			response[1] = 4;
			break;
		case kMirageAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kMirageAcFanAuto:
			response[2] = 1;
			break;
		case kMirageAcFanLow:
			response[2] = 2;
			break;
		case kMirageAcFanMed:
			response[2] = 3;
			break;
		case kMirageAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingH() << 1);
	return 1;
}

uint8_t decode_neoclima_ac(uint8_t* response)
{
	if(results.decode_type != NEOCLIMA) return 0;
	IRNeoclimaAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kNeoclimaAuto:
			response[1] = 1;
			break;
		case kNeoclimaCool:
			response[1] = 2;
			break;
		case kNeoclimaDry:
			response[1] = 3;
			break;
		case kNeoclimaHeat:
			response[1] = 4;
			break;
		case kNeoclimaFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kNeoclimaFanAuto:
			response[2] = 1;
			break;
		case kNeoclimaFanLow:
			response[2] = 2;
			break;
		case kNeoclimaFanMed:
			response[2] = 3;
			break;
		case kNeoclimaFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingH() << 1) | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_rhoss_ac(uint8_t* response)
{
	if(results.decode_type != RHOSS) return 0;
	IRRhossAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kRhossModeAuto:
			response[1] = 1;
			break;
		case kRhossModeCool:
			response[1] = 2;
			break;
		case kRhossModeDry:
			response[1] = 3;
			break;
		case kRhossModeHeat:
			response[1] = 4;
			break;
		case kRhossModeFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kRhossFanAuto:
			response[2] = 1;
			break;
		case kRhossFanMin:
			response[2] = 2;
			break;
		case kRhossFanMed:
			response[2] = 3;
			break;
		case kRhossFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwing() << 2);
	return 1;
}

uint8_t decode_technibel_ac(uint8_t* response)
{
	if(results.decode_type != TECHNIBEL_AC) return 0;
	IRTechnibelAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kTechnibelAcCool:
			response[1] = 2;
			break;
		case kTechnibelAcDry:
			response[1] = 3;
			break;
		case kTechnibelAcHeat:
			response[1] = 4;
			break;
		case kTechnibelAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kTechnibelAcFanLow:
			response[2] = 2;
			break;
		case kTechnibelAcFanMedium:
			response[2] = 3;
			break;
		case kTechnibelAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwing() << 2);
	return 1;
}

uint8_t decode_teco_ac(uint8_t* response)
{
	if(results.decode_type != TECO) return 0;
	IRTecoAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kTecoAuto:
			response[1] = 1;
			break;
		case kTecoCool:
			response[1] = 2;
			break;
		case kTecoDry:
			response[1] = 3;
			break;
		case kTecoHeat:
			response[1] = 4;
			break;
		case kTecoFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kTecoFanAuto:
			response[2] = 1;
			break;
		case kTecoFanLow:
			response[2] = 2;
			break;
		case kTecoFanMed:
			response[2] = 3;
			break;
		case kTecoFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwing() << 2);
	return 1;
}

uint8_t decode_transcold_ac(uint8_t* response)
{
	if(results.decode_type != TRANSCOLD) return 0;
	IRTranscoldAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kTranscoldAuto:
			response[1] = 1;
			break;
		case kTranscoldCool:
			response[1] = 2;
			break;
		case kTranscoldDry:
			response[1] = 3;
			break;
		case kTranscoldHeat:
			response[1] = 4;
			break;
		case kTranscoldFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kTranscoldFanAuto:
			response[2] = 1;
			break;
		case kTranscoldFanMin:
			response[2] = 2;
			break;
		case kTranscoldFanMed:
			response[2] = 3;
			break;
		case kTranscoldFanMax:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwing() << 2);
	return 1;
}

uint8_t decode_trotecesp_ac(uint8_t* response)
{
	if(results.decode_type != TROTEC) return 0;
	IRTrotecESP ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kTrotecAuto:
			response[1] = 1;
			break;
		case kTrotecCool:
			response[1] = 2;
			break;
		case kTrotecDry:
			response[1] = 3;
			break;
		case kTrotecFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kTrotecFanLow:
			response[2] = 2;
			break;
		case kTrotecFanMed:
			response[2] = 3;
			break;
		case kTrotecFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_trotec3550_ac(uint8_t* response)
{
	if(results.decode_type != TROTEC_3550) return 0;
	IRTrotec3550 ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kTrotecAuto:
			response[1] = 1;
			break;
		case kTrotecCool:
			response[1] = 2;
			break;
		case kTrotecDry:
			response[1] = 3;
			break;
		case kTrotecFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kTrotecFanLow:
			response[2] = 2;
			break;
		case kTrotecFanMed:
			response[2] = 3;
			break;
		case kTrotecFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_truma_ac(uint8_t* response)
{
	if(results.decode_type != TRUMA) return 0;
	IRTrumaAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kTrumaAuto:
			response[1] = 1;
			break;
		case kTrumaCool:
			response[1] = 2;
			break;
		case kTrumaFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kTrumaFanLow:
			response[2] = 2;
			break;
		case kTrumaFanMed:
			response[2] = 3;
			break;
		case kTrumaFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01;
	return 1;
}

uint8_t decode_vestel_ac(uint8_t* response)
{
	if(results.decode_type != VESTEL_AC) return 0;
	IRVestelAc ac(-1);
	ac.setRaw(results.value);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kVestelAcAuto:
			response[1] = 1;
			break;
		case kVestelAcCool:
			response[1] = 2;
			break;
		case kVestelAcDry:
			response[1] = 3;
			break;
		case kVestelAcHeat:
			response[1] = 4;
			break;
		case kVestelAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kVestelAcFanAuto:
			response[2] = 1;
			break;
		case kVestelAcFanLow:
			response[2] = 2;
			break;
		case kVestelAcFanMed:
			response[2] = 3;
			break;
		case kVestelAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwing() << 2);
	return 1;
}

uint8_t decode_voltas_ac(uint8_t* response)
{
	if(results.decode_type != VOLTAS) return 0;
	IRVoltas ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPower()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kVoltasCool:
			response[1] = 2;
			break;
		case kVoltasDry:
			response[1] = 3;
			break;
		case kVoltasHeat:
			response[1] = 4;
			break;
		case kVoltasFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kVoltasFanAuto:
			response[2] = 1;
			break;
		case kVoltasFanLow:
			response[2] = 2;
			break;
		case kVoltasFanMed:
			response[2] = 3;
			break;
		case kVoltasFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwingH() << 1) | (ac.getSwingV() << 2);
	return 1;
}

uint8_t decode_whirlpool_ac(uint8_t* response)
{
	if(results.decode_type != WHIRLPOOL_AC) return 0;
	IRWhirlpoolAc ac(-1);
	ac.setRaw(results.state);
	if(!ac.getPowerToggle()) response[0]  = 1;
	else response[0] = ac.getTemp();
	response[1] = 2;
	switch(ac.getMode())
	{
		case kWhirlpoolAcAuto:
			response[1] = 1;
			break;
		case kWhirlpoolAcCool:
			response[1] = 2;
			break;
		case kWhirlpoolAcDry:
			response[1] = 3;
			break;
		case kWhirlpoolAcHeat:
			response[1] = 4;
			break;
		case kWhirlpoolAcFan:
			response[1] = 5;
			break;
	}
	response[2] = 1;
	switch(ac.getFan())
	{
		case kWhirlpoolAcFanAuto:
			response[2] = 1;
			break;
		case kWhirlpoolAcFanLow:
			response[2] = 2;
			break;
		case kWhirlpoolAcFanMedium:
			response[2] = 3;
			break;
		case kWhirlpoolAcFanHigh:
			response[2] = 4;
			break;
	}
	response[3] = 0x01 | (ac.getSwing() << 2);
	return 1;
}

void setup_receive_custom_ac(uint8_t idl, uint8_t idh, uint8_t index)
{
	decode_ac = NULL;
	if((idl+256*idh == _DAIKIN_AC) && (index == 9)) decode_ac = decode_daikin_custom1_ac;
}

void setup_receive_ac(uint8_t idl, uint8_t idh, uint8_t index)
{
	decode_ac = NULL;
	if((idl+256*idh == _AIRTON_AC) && (index == 1)) decode_ac = decode_airton_ac;
	if((idl+256*idh == _AIRWELL_AC) && (index == 1)) decode_ac = decode_airwell_ac;
	if((idl+256*idh == _ALASKA_AC) && (index == 1)) decode_ac = decode_teco_ac;
	if((idl+256*idh == _AMCOR_AC) && (index == 1)) decode_ac = decode_amcor_ac;
	if((idl+256*idh == _AQUA_AC) && (index == 1)) decode_ac = decode_sanyo_ac;
	if((idl+256*idh == _AQUA_AC) && (index == 2)) decode_ac = decode_sanyo88_ac;
	if((idl+256*idh == _AQUA_AC) && (index == 3)) decode_ac = decode_haieryrw02_ac;
	if((idl+256*idh == _ARGO_AC) && (index == 1)) decode_ac = decode_argo_ac;
	if((idl+256*idh == _ASANZO_AC) && (index == 1)) decode_ac = decode_electra_ac;
	if((idl+256*idh == _BEKO_AC) && (index == 2)) decode_ac = decode_coolix_ac;
	if((idl+256*idh == _BOSCH_AC) && (index == 1)) decode_ac = decode_bosch_ac;
	if((idl+256*idh == _CARRIER_AC) && (index == 1)) decode_ac = decode_carrier64_ac;
	if((idl+256*idh == _COMFEE_AC) && (index == 1)) decode_ac = decode_midea_ac;
	if((idl+256*idh == _COOLIX_AC) && (index == 2)) decode_ac = decode_coolix_ac;
	if((idl+256*idh == _CORONA_AC) && (index == 1)) decode_ac = decode_corona_ac;
	if((idl+256*idh == _DAIKIN_AC) && (index == 1)) decode_ac = decode_daikinesp_ac;
	if((idl+256*idh == _DAIKIN_AC) && (index == 2)) decode_ac = decode_daikin2_ac;
	if((idl+256*idh == _DAIKIN_AC) && (index == 3)) decode_ac = decode_daikin216_ac;
	if((idl+256*idh == _DAIKIN_AC) && (index == 4)) decode_ac = decode_daikin160_ac;
	if((idl+256*idh == _DAIKIN_AC) && (index == 5)) decode_ac = decode_daikin176_ac;
	if((idl+256*idh == _DAIKIN_AC) && (index == 6)) decode_ac = decode_daikin128_ac;
	if((idl+256*idh == _DAIKIN_AC) && (index == 7)) decode_ac = decode_daikin152_ac;
	if((idl+256*idh == _DAIKIN_AC) && (index == 8)) decode_ac = decode_daikin64_ac;
	if((idl+256*idh == _DELONGHI_AC) && (index == 1)) decode_ac = decode_delonghi_ac;
	if((idl+256*idh == _ECOCLIM_AC) && (index == 1)) decode_ac = decode_ecoclim_ac;
	if((idl+256*idh == _ELECTRA_AC) && (index == 1)) decode_ac = decode_electra_ac;
	if((idl+256*idh == _FUJITSU_AC) && (index == 1)) decode_ac = decode_fujitsu_ac;
	if((idl+256*idh == _GOODWEATHER_AC) && (index == 1)) decode_ac = decode_goodweather_ac;
	if((idl+256*idh == _GREE_AC) && (index == 1)) decode_ac = decode_gree_ac;
	if((idl+256*idh == _HAIER_AC) && (index == 1)) decode_ac = decode_haier_ac;
	if((idl+256*idh == _HAIER_AC) && (index == 2)) decode_ac = decode_haier176_ac;
	if((idl+256*idh == _HAIER_AC) && (index == 3)) decode_ac = decode_haier160_ac;
	if((idl+256*idh == _HAIER_AC) && (index == 4)) decode_ac = decode_haieryrw02_ac;
	if((idl+256*idh == _HISENSE_AC) && (index == 1)) decode_ac = decode_kelon_ac;
	if((idl+256*idh == _HITACHI_AC) && (index == 1)) decode_ac = decode_hitachi_ac;
	if((idl+256*idh == _HITACHI_AC) && (index == 2)) decode_ac = decode_hitachi1_ac;
	if((idl+256*idh == _HITACHI_AC) && (index == 3)) decode_ac = decode_hitachi424_ac;
	if((idl+256*idh == _HITACHI_AC) && (index == 4)) decode_ac = decode_hitachi344_ac;
	if((idl+256*idh == _HITACHI_AC) && (index == 5)) decode_ac = decode_hitachi264_ac;
	if((idl+256*idh == _HITACHI_AC) && (index == 6)) decode_ac = decode_hitachi296_ac;
	if((idl+256*idh == _KELON_AC) && (index == 1)) decode_ac = decode_kelon_ac;
	if((idl+256*idh == _KELVINATOR_AC) && (index == 1)) decode_ac = decode_kelvinator_ac;
	if((idl+256*idh == _LG_AC) && (index == 1)) decode_ac = decode_lg_ac;
	if((idl+256*idh == _LG_AC) && (index == 2)) decode_ac = decode_lg2_ac;
	if((idl+256*idh == _MIDEA_AC) && (index == 1)) decode_ac = decode_midea_ac;
	if((idl+256*idh == _MIDEA_AC) && (index == 2)) decode_ac = decode_coolix_ac;
	if((idl+256*idh == _MIRAGE_AC) && (index == 1)) decode_ac = decode_mirage_ac;
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 1)) decode_ac = decode_mitsubishi_ac;
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 2)) decode_ac = decode_mitsubishi136_ac;
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 3)) decode_ac = decode_mitsubishi112_ac;
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 4)) decode_ac = decode_mitsubishi152_ac;
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 5)) decode_ac = decode_mitsubishi88_ac;
	if((idl+256*idh == _NEOCLIMA_AC) && (index == 1)) decode_ac = decode_neoclima_ac;
	if((idl+256*idh == _PANASONIC_AC) && (index == 1)) decode_ac = decode_panasonic_ac;
	if((idl+256*idh == _PANASONIC_AC) && (index == 2)) decode_ac = decode_panasonic32_ac;
	if((idl+256*idh == _RHOSS_AC) && (index == 1)) decode_ac = decode_rhoss_ac;
	if((idl+256*idh == _SAMSUNG_AC) && (index == 1)) decode_ac = decode_samsung_ac;
	if((idl+256*idh == _SHARP_AC) && (index == 1)) decode_ac = decode_sharp_ac;
	if((idl+256*idh == _TCL_AC) && (index == 1)) decode_ac = decode_tcl_ac;
	if((idl+256*idh == _TECHNIBEL_AC) && (index == 1)) decode_ac = decode_technibel_ac;
	if((idl+256*idh == _TECO_AC) && (index == 1)) decode_ac = decode_teco_ac;
	if((idl+256*idh == _TOSHIBA_AC) && (index == 1)) decode_ac = decode_toshiba_ac;
	if((idl+256*idh == _TRANSCOLD_AC) && (index == 1)) decode_ac = decode_transcold_ac;
	if((idl+256*idh == _TROTEC_AC) && (index == 1)) decode_ac = decode_trotecesp_ac;
	if((idl+256*idh == _TROTEC_AC) && (index == 2)) decode_ac = decode_trotec3550_ac;
	if((idl+256*idh == _TRUMA_AC) && (index == 1)) decode_ac = decode_truma_ac;
	if((idl+256*idh == _VESTEL_AC) && (index == 1)) decode_ac = decode_vestel_ac;
	if((idl+256*idh == _VOLTAS_AC) && (index == 1)) decode_ac = decode_voltas_ac;
	if((idl+256*idh == _WHIRLPOOL_AC) && (index == 1)) decode_ac = decode_whirlpool_ac;
}

//----------------------------------------------------------------------------------------------------------------

void setup_electra_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRElectraAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kElectraAcFanAuto);
			break;
		case 2:
			ac.setMode(kElectraAcCool);
			break;
		case 3:
			ac.setMode(kElectraAcDry);
			break;
		case 4:
			ac.setMode(kElectraAcHeat);
			break;
		case 5:
			ac.setMode(kElectraAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kElectraAcFanAuto);
			break;
		case 2:
			ac.setFan(kElectraAcFanLow);
			break;
		case 3:
			ac.setFan(kElectraAcFanMed);
			break;
		case 4:
			ac.setFan(kElectraAcFanHigh);
			break;
	}
	ac.setSwingH((swing >> 1)&0x01);
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_coolix_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRCoolixAC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kCoolixAuto);
			break;
		case 2:
			ac.setMode(kCoolixCool);
			break;
		case 3:
			ac.setMode(kCoolixDry);
			break;
		case 4:
			ac.setMode(kCoolixHeat);
			break;
		case 5:
			ac.setMode(kElectraAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kCoolixFanAuto);
			break;
		case 2:
			ac.setFan(kCoolixFanMin);
			break;
		case 3:
			ac.setFan(kCoolixFanMed);
			break;
		case 4:
			ac.setFan(kCoolixFanMax);
			break;
	}
	if((swing >> 1) & 0x01) ac.setSwing();
	if((swing >> 2) & 0x01) ac.setSwingVStep();
	ac.send(kNoRepeat);
}

void setup_midea_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRMideaAC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kMideaACAuto);
			break;
		case 2:
			ac.setMode(kMideaACCool);
			break;
		case 3:
			ac.setMode(kMideaACDry);
			break;
		case 4:
			ac.setMode(kMideaACHeat);
			break;
		case 5:
			ac.setMode(kMideaACFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kMideaACFanAuto);
			break;
		case 2:
			ac.setFan(kMideaACFanLow);
			break;
		case 3:
			ac.setFan(kMideaACFanMed);
			break;
		case 4:
			ac.setFan(kMideaACFanHigh);
			break;
	}
	ac.setSwingVToggle((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_carrier64_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRCarrierAc64 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kCarrierAc64Cool);
			break;
		case 4:
			ac.setMode(kCarrierAc64Heat);
			break;
		case 5:
			ac.setMode(kCarrierAc64Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kCarrierAc64FanAuto);
			break;
		case 2:
			ac.setFan(kCarrierAc64FanLow);
			break;
		case 3:
			ac.setFan(kCarrierAc64FanMedium);
			break;
		case 4:
			ac.setFan(kCarrierAc64FanHigh);
			break;
	}
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_daikinesp_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDaikinESP ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kDaikinAuto);
			break;
		case 2:
			ac.setMode(kDaikinCool);
			break;
		case 3:
			ac.setMode(kDaikinDry);
			break;
		case 4:
			ac.setMode(kDaikinHeat);
			break;
		case 5:
			ac.setMode(kDaikinFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kDaikinFanAuto);
			break;
		case 2:
			ac.setFan(kDaikinFanMin);
			break;
		case 3:
			ac.setFan(kDaikinFanMed);
			break;
		case 4:
			ac.setFan(kDaikinFanMax);
			break;
	}
	ac.setSwingHorizontal((swing >> 1)&0x01);
	ac.setSwingVertical((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_daikin2_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDaikin2 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kDaikinAuto);
			break;
		case 2:
			ac.setMode(kDaikinCool);
			break;
		case 3:
			ac.setMode(kDaikinDry);
			break;
		case 4:
			ac.setMode(kDaikinHeat);
			break;
		case 5:
			ac.setMode(kDaikinFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kDaikinFanAuto);
			break;
		case 2:
			ac.setFan(kDaikinFanMin);
			break;
		case 3:
			ac.setFan(kDaikinFanMed);
			break;
		case 4:
			ac.setFan(kDaikinFanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_daikin216_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDaikin216 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kDaikinAuto);
			break;
		case 2:
			ac.setMode(kDaikinCool);
			break;
		case 3:
			ac.setMode(kDaikinDry);
			break;
		case 4:
			ac.setMode(kDaikinHeat);
			break;
		case 5:
			ac.setMode(kDaikinFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kDaikinFanAuto);
			break;
		case 2:
			ac.setFan(kDaikinFanMin);
			break;
		case 3:
			ac.setFan(kDaikinFanMed);
			break;
		case 4:
			ac.setFan(kDaikinFanMax);
			break;
	}
	ac.setSwingHorizontal((swing >> 1)&0x01);
	ac.setSwingVertical((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_daikin160_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDaikin160 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kDaikinAuto);
			break;
		case 2:
			ac.setMode(kDaikinCool);
			break;
		case 3:
			ac.setMode(kDaikinDry);
			break;
		case 4:
			ac.setMode(kDaikinHeat);
			break;
		case 5:
			ac.setMode(kDaikinFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kDaikinFanAuto);
			break;
		case 2:
			ac.setFan(kDaikinFanMin);
			break;
		case 3:
			ac.setFan(kDaikinFanMed);
			break;
		case 4:
			ac.setFan(kDaikinFanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_daikin176_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDaikin176 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kDaikin176Auto);
			break;
		case 2:
			ac.setMode(kDaikin176Cool);
			break;
		case 3:
			ac.setMode(kDaikin176Dry);
			break;
		case 4:
			ac.setMode(kDaikin176Heat);
			break;
		case 5:
			ac.setMode(kDaikin176Fan);
			break;
	}
	switch(fan)
	{
		case 2:
			ac.setFan(kDaikinFanMin);
			break;
		case 4:
			ac.setFan(kDaikin176FanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_daikin128_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDaikin128 ac(13);
	if(temp != 1)
	{
		ac.setPowerToggle(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPowerToggle(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kDaikin128Auto);
			break;
		case 2:
			ac.setMode(kDaikin128Cool);
			break;
		case 3:
			ac.setMode(kDaikin128Dry);
			break;
		case 4:
			ac.setMode(kDaikin128Heat);
			break;
		case 5:
			ac.setMode(kDaikin128Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kDaikin128FanAuto);
			break;
		case 2:
			ac.setFan(kDaikin128FanLow);
			break;
		case 3:
			ac.setFan(kDaikin128FanMed);
			break;
		case 4:
			ac.setFan(kDaikin128FanHigh);
			break;
	}
	ac.setSwingVertical((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_daikin152_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDaikin152 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kDaikinAuto);
			break;
		case 2:
			ac.setMode(kDaikinCool);
			break;
		case 3:
			ac.setMode(kDaikinDry);
			break;
		case 4:
			ac.setMode(kDaikinHeat);
			break;
		case 5:
			ac.setMode(kDaikinFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kDaikinFanAuto);
			break;
		case 2:
			ac.setFan(kDaikinFanMin);
			break;
		case 3:
			ac.setFan(kDaikinFanMed);
			break;
		case 4:
			ac.setFan(kDaikinFanMax);
			break;
	}
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_daikin64_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDaikin64 ac(13);
	if(temp != 1)
	{
		ac.setPowerToggle(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPowerToggle(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kDaikin64Cool);
			break;
		case 3:
			ac.setMode(kDaikin64Dry);
			break;
		case 4:
			ac.setMode(kDaikin64Heat);
			break;
		case 5:
			ac.setMode(kDaikin64Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kDaikin64FanAuto);
			break;
		case 2:
			ac.setFan(kDaikin64FanLow);
			break;
		case 3:
			ac.setFan(kDaikin64FanMed);
			break;
		case 4:
			ac.setFan(kDaikin64FanHigh);
			break;
	}
	ac.setSwingVertical((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_fujitsu_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRFujitsuAC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kFujitsuAcModeAuto);
			break;
		case 2:
			ac.setMode(kFujitsuAcModeCool);
			break;
		case 3:
			ac.setMode(kFujitsuAcModeDry);
			break;
		case 4:
			ac.setMode(kFujitsuAcModeHeat);
			break;
		case 5:
			ac.setMode(kFujitsuAcModeFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFanSpeed(kFujitsuAcFanAuto);
			break;
		case 2:
			ac.setFanSpeed(kFujitsuAcFanLow);
			break;
		case 3:
			ac.setFanSpeed(kFujitsuAcFanMed);
			break;
		case 4:
			ac.setFanSpeed(kFujitsuAcFanHigh);
			break;
	}
	ac.toggleSwingHoriz((swing >> 1)&0x01);
	ac.toggleSwingVert((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_gree_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRGreeAC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kGreeAuto);
			break;
		case 2:
			ac.setMode(kGreeCool);
			break;
		case 3:
			ac.setMode(kGreeDry);
			break;
		case 4:
			ac.setMode(kGreeHeat);
			break;
		case 5:
			ac.setMode(kGreeFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kGreeFanAuto);
			break;
		case 2:
			ac.setFan(kGreeFanMin);
			break;
		case 3:
			ac.setFan(kGreeFanMed);
			break;
		case 4:
			ac.setFan(kGreeFanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_hitachi_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHitachiAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kHitachiAcAuto);
			break;
		case 2:
			ac.setMode(kHitachiAcCool);
			break;
		case 3:
			ac.setMode(kHitachiAcDry);
			break;
		case 4:
			ac.setMode(kHitachiAcHeat);
			break;
		case 5:
			ac.setMode(kHitachiAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHitachiAcFanAuto);
			break;
		case 2:
			ac.setFan(kHitachiAcFanLow);
			break;
		case 3:
			ac.setFan(kHitachiAcFanMed);
			break;
		case 4:
			ac.setFan(kHitachiAcFanHigh);
			break;
	}
	ac.setSwingHorizontal((swing >> 1)&0x01);
	ac.setSwingVertical((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_hitachi1_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHitachiAc1 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kHitachiAc1Auto);
			break;
		case 2:
			ac.setMode(kHitachiAc1Cool);
			break;
		case 3:
			ac.setMode(kHitachiAc1Dry);
			break;
		case 4:
			ac.setMode(kHitachiAc1Heat);
			break;
		case 5:
			ac.setMode(kHitachiAc1Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHitachiAc1FanAuto);
			break;
		case 2:
			ac.setFan(kHitachiAc1FanLow);
			break;
		case 3:
			ac.setFan(kHitachiAc1FanMed);
			break;
		case 4:
			ac.setFan(kHitachiAc1FanHigh);
			break;
	}
	ac.setSwingH((swing >> 1)&0x01);
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_hitachi424_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHitachiAc424 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kHitachiAc424Cool);
			break;
		case 3:
			ac.setMode(kHitachiAc424Dry);
			break;
		case 4:
			ac.setMode(kHitachiAc424Heat);
			break;
		case 5:
			ac.setMode(kHitachiAc424Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHitachiAc424FanAuto);
			break;
		case 2:
			ac.setFan(kHitachiAc424FanLow);
			break;
		case 3:
			ac.setFan(kHitachiAc424FanMedium);
			break;
		case 4:
			ac.setFan(kHitachiAc424FanHigh);
			break;
	}
	ac.setSwingVToggle((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_hitachi344_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHitachiAc344 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kHitachiAc344Cool);
			break;
		case 3:
			ac.setMode(kHitachiAc344Dry);
			break;
		case 4:
			ac.setMode(kHitachiAc344Heat);
			break;
		case 5:
			ac.setMode(kHitachiAc344Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHitachiAc344FanAuto);
			break;
		case 2:
			ac.setFan(kHitachiAc344FanLow);
			break;
		case 3:
			ac.setFan(kHitachiAc344FanMedium);
			break;
		case 4:
			ac.setFan(kHitachiAc344FanHigh);
			break;
	}
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_hitachi264_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHitachiAc264 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kHitachiAc264Cool);
			break;
		case 3:
			ac.setMode(kHitachiAc264Dry);
			break;
		case 4:
			ac.setMode(kHitachiAc264Heat);
			break;
		case 5:
			ac.setMode(kHitachiAc264Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHitachiAc264FanAuto);
			break;
		case 2:
			ac.setFan(kHitachiAc264FanLow);
			break;
		case 3:
			ac.setFan(kHitachiAc264FanMedium);
			break;
		case 4:
			ac.setFan(kHitachiAc264FanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_hitachi296_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHitachiAc296 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kHitachiAc296Auto);
			break;
		case 2:
			ac.setMode(kHitachiAc296Cool);
			break;
		case 4:
			ac.setMode(kHitachiAc296Heat);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHitachiAc296FanAuto);
			break;
		case 2:
			ac.setFan(kHitachiAc296FanLow);
			break;
		case 3:
			ac.setFan(kHitachiAc296FanMedium);
			break;
		case 4:
			ac.setFan(kHitachiAc296FanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_panasonic_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRPanasonicAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kPanasonicAcAuto);
			break;
		case 2:
			ac.setMode(kPanasonicAcCool);
			break;
		case 3:
			ac.setMode(kPanasonicAcDry);
			break;
		case 4:
			ac.setMode(kPanasonicAcHeat);
			break;
		case 5:
			ac.setMode(kPanasonicAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kPanasonicAcFanAuto);
			break;
		case 2:
			ac.setFan(kPanasonicAcFanLow);
			break;
		case 3:
			ac.setFan(kPanasonicAcFanMed);
			break;
		case 4:
			ac.setFan(kPanasonicAcFanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_panasonic32_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRPanasonicAc32 ac(13);
	if(temp != 1)
	{
		ac.setPowerToggle(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPowerToggle(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kPanasonicAc32Auto);
			break;
		case 2:
			ac.setMode(kPanasonicAc32Cool);
			break;
		case 3:
			ac.setMode(kPanasonicAc32Dry);
			break;
		case 4:
			ac.setMode(kPanasonicAc32Heat);
			break;
		case 5:
			ac.setMode(kPanasonicAc32Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kPanasonicAc32FanAuto);
			break;
		case 2:
			ac.setFan(kPanasonicAc32FanLow);
			break;
		case 3:
			ac.setFan(kPanasonicAc32FanMed);
			break;
		case 4:
			ac.setFan(kPanasonicAc32FanHigh);
			break;
	}
	ac.setSwingHorizontal((swing >> 1)&0x01);
	ac.send(kNoRepeat);
}

void setup_toshiba_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRToshibaAC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kToshibaAcAuto);
			break;
		case 2:
			ac.setMode(kToshibaAcCool);
			break;
		case 3:
			ac.setMode(kToshibaAcDry);
			break;
		case 4:
			ac.setMode(kToshibaAcHeat);
			break;
		case 5:
			ac.setMode(kToshibaAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kToshibaAcFanAuto);
			break;
		case 2:
			ac.setFan(kToshibaAcFanMin);
			break;
		case 3:
			ac.setFan(kToshibaAcFanMed);
			break;
		case 4:
			ac.setFan(kToshibaAcFanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_tcl_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRTcl112Ac ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kTcl112AcAuto);
			break;
		case 2:
			ac.setMode(kTcl112AcCool);
			break;
		case 3:
			ac.setMode(kTcl112AcDry);
			break;
		case 4:
			ac.setMode(kTcl112AcHeat);
			break;
		case 5:
			ac.setMode(kTcl112AcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kTcl112AcFanAuto);
			break;
		case 2:
			ac.setFan(kTcl112AcFanLow);
			break;
		case 3:
			ac.setFan(kTcl112AcFanMed);
			break;
		case 4:
			ac.setFan(kTcl112AcFanHigh);
			break;
	}
	ac.setSwingHorizontal((swing >> 1)&0x01);
	ac.send(kNoRepeat);
}

void setup_mitsubishi_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRMitsubishiAC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kMitsubishiAcAuto);
			break;
		case 2:
			ac.setMode(kMitsubishiAcCool);
			break;
		case 3:
			ac.setMode(kMitsubishiAcDry);
			break;
		case 4:
			ac.setMode(kMitsubishiAcHeat);
			break;
		case 5:
			ac.setMode(kMitsubishiAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kMitsubishiAcFanAuto);
			break;
		case 4:
			ac.setFan(kMitsubishiAcFanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_mitsubishi136_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRMitsubishi136 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kMitsubishi136Auto);
			break;
		case 2:
			ac.setMode(kMitsubishi136Cool);
			break;
		case 3:
			ac.setMode(kMitsubishi136Dry);
			break;
		case 4:
			ac.setMode(kMitsubishi136Heat);
			break;
		case 5:
			ac.setMode(kMitsubishi136Fan);
			break;
	}
	switch(fan)
	{
		case 2:
			ac.setFan(kMitsubishi136FanMin);
			break;
		case 3:
			ac.setFan(kMitsubishi136FanMed);
			break;
		case 4:
			ac.setFan(kMitsubishi136FanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_mitsubishi112_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRMitsubishi112 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kMitsubishi112Auto);
			break;
		case 2:
			ac.setMode(kMitsubishi112Cool);
			break;
		case 3:
			ac.setMode(kMitsubishi112Dry);
			break;
		case 4:
			ac.setMode(kMitsubishi112Heat);
			break;
	}
	switch(fan)
	{
		case 2:
			ac.setFan(kMitsubishi112FanMin);
			break;
		case 3:
			ac.setFan(kMitsubishi112FanMed);
			break;
		case 4:
			ac.setFan(kMitsubishi112FanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_mitsubishi152_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRMitsubishiHeavy152Ac ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kMitsubishiHeavyAuto);
			break;
		case 2:
			ac.setMode(kMitsubishiHeavyCool);
			break;
		case 3:
			ac.setMode(kMitsubishiHeavyDry);
			break;
		case 4:
			ac.setMode(kMitsubishiHeavyHeat);
			break;
		case 5:
			ac.setMode(kMitsubishiHeavyFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kMitsubishiHeavy152FanAuto);
			break;
		case 2:
			ac.setFan(kMitsubishiHeavy152FanLow);
			break;
		case 3:
			ac.setFan(kMitsubishiHeavy152FanMed);
			break;
		case 4:
			ac.setFan(kMitsubishiHeavy152FanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_mitsubishi88_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRMitsubishiHeavy88Ac ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kMitsubishiHeavyAuto);
			break;
		case 2:
			ac.setMode(kMitsubishiHeavyCool);
			break;
		case 3:
			ac.setMode(kMitsubishiHeavyDry);
			break;
		case 4:
			ac.setMode(kMitsubishiHeavyHeat);
			break;
		case 5:
			ac.setMode(kMitsubishiHeavyFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kMitsubishiHeavy88FanAuto);
			break;
		case 2:
			ac.setFan(kMitsubishiHeavy88FanLow);
			break;
		case 3:
			ac.setFan(kMitsubishiHeavy88FanMed);
			break;
		case 4:
			ac.setFan(kMitsubishiHeavy88FanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_sanyo_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRSanyoAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kSanyoAcAuto);
			break;
		case 2:
			ac.setMode(kSanyoAcCool);
			break;
		case 3:
			ac.setMode(kSanyoAcDry);
			break;
		case 4:
			ac.setMode(kSanyoAcHeat);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kSanyoAcFanAuto);
			break;
		case 2:
			ac.setFan(kSanyoAcFanLow);
			break;
		case 3:
			ac.setFan(kSanyoAcFanMedium);
			break;
		case 4:
			ac.setFan(kSanyoAcFanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_sanyo88_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRSanyoAc88 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kSanyoAc88Auto);
			break;
		case 2:
			ac.setMode(kSanyoAc88Cool);
			break;
		case 4:
			ac.setMode(kSanyoAc88Heat);
			break;
		case 5:
			ac.setMode(kSanyoAc88Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kSanyoAc88FanAuto);
			break;
		case 2:
			ac.setFan(kSanyoAc88FanLow);
			break;
		case 3:
			ac.setFan(kSanyoAc88FanMedium);
			break;
		case 4:
			ac.setFan(kSanyoAc88FanHigh);
			break;
	}
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_sharp_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRSharpAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kSharpAcAuto);
			break;
		case 2:
			ac.setMode(kSharpAcCool);
			break;
		case 3:
			ac.setMode(kSharpAcDry);
			break;
		case 4:
			ac.setMode(kSharpAcHeat);
			break;
		case 5:
			ac.setMode(kSharpAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kSharpAcFanAuto);
			break;
		case 2:
			ac.setFan(kSharpAcFanMin);
			break;
		case 3:
			ac.setFan(kSharpAcFanMed);
			break;
		case 4:
			ac.setFan(kSharpAcFanMax);
			break;
	}
	ac.setSwingToggle((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_lg_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRLgAc ac(13);
	ac._protocol = LG;
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kLgAcAuto);
			break;
		case 2:
			ac.setMode(kLgAcCool);
			break;
		case 3:
			ac.setMode(kLgAcDry);
			break;
		case 4:
			ac.setMode(kLgAcHeat);
			break;
		case 5:
			ac.setMode(kLgAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kLgAcFanAuto);
			break;
		case 2:
			ac.setFan(kLgAcFanLow);
			break;
		case 3:
			ac.setFan(kLgAcFanMedium);
			break;
		case 4:
			ac.setFan(kLgAcFanHigh);
			break;
	}
	ac.setSwingH((swing >> 1)&0x01);
	ac.send(kNoRepeat);
}

void setup_lg2_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRLgAc ac(13);
	ac._protocol = LG2;
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kLgAcAuto);
			break;
		case 2:
			ac.setMode(kLgAcCool);
			break;
		case 3:
			ac.setMode(kLgAcDry);
			break;
		case 4:
			ac.setMode(kLgAcHeat);
			break;
		case 5:
			ac.setMode(kLgAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kLgAcFanAuto);
			break;
		case 2:
			ac.setFan(kLgAcFanLow);
			break;
		case 3:
			ac.setFan(kLgAcFanMedium);
			break;
		case 4:
			ac.setFan(kLgAcFanHigh);
			break;
	}
	ac.setSwingH((swing >> 1)&0x01);
	ac.send(kNoRepeat);
}

void setup_samsung_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRSamsungAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kSamsungAcAuto);
			break;
		case 2:
			ac.setMode(kSamsungAcCool);
			break;
		case 3:
			ac.setMode(kSamsungAcDry);
			break;
		case 4:
			ac.setMode(kSamsungAcHeat);
			break;
		case 5:
			ac.setMode(kSamsungAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kSamsungAcFanAuto);
			break;
		case 2:
			ac.setFan(kSamsungAcFanLow);
			break;
		case 3:
			ac.setFan(kSamsungAcFanMed);
			break;
		case 4:
			ac.setFan(kSamsungAcFanHigh);
			break;
	}
	ac.setSwingH((swing >> 1)&0x01);
	ac.setSwing((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_airton_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRAirtonAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kAirtonAuto);
			break;
		case 2:
			ac.setMode(kAirtonCool);
			break;
		case 3:
			ac.setMode(kAirtonDry);
			break;
		case 4:
			ac.setMode(kAirtonHeat);
			break;
		case 5:
			ac.setMode(kAirtonFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kAirtonFanAuto);
			break;
		case 2:
			ac.setFan(kAirtonFanLow);
			break;
		case 3:
			ac.setFan(kAirtonFanMed);
			break;
		case 4:
			ac.setFan(kAirtonFanHigh);
			break;
	}
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_airwell_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRAirwellAc ac(13);
	if(temp != 1)
	{
		ac.setPowerToggle(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPowerToggle(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kAirwellAuto);
			break;
		case 2:
			ac.setMode(kAirwellCool);
			break;
		case 3:
			ac.setMode(kAirwellDry);
			break;
		case 4:
			ac.setMode(kAirwellHeat);
			break;
		case 5:
			ac.setMode(kAirwellFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kAirwellFanAuto);
			break;
		case 2:
			ac.setFan(kAirwellFanLow);
			break;
		case 3:
			ac.setFan(kAirwellFanMedium);
			break;
		case 4:
			ac.setFan(kAirwellFanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_amcor_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRAmcorAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kAmcorAuto);
			break;
		case 2:
			ac.setMode(kAmcorCool);
			break;
		case 3:
			ac.setMode(kAmcorDry);
			break;
		case 4:
			ac.setMode(kAmcorHeat);
			break;
		case 5:
			ac.setMode(kAmcorFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kAmcorFanAuto);
			break;
		case 2:
			ac.setFan(kAmcorFanMin);
			break;
		case 3:
			ac.setFan(kAmcorFanMed);
			break;
		case 4:
			ac.setFan(kAmcorFanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_argo_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRArgoAC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kArgoAuto);
			break;
		case 2:
			ac.setMode(kArgoCool);
			break;
		case 3:
			ac.setMode(kArgoDry);
			break;
		case 4:
			ac.setMode(kArgoHeat);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kArgoFanAuto);
			break;
		case 2:
			ac.setFan(kArgoFan1);
			break;
		case 3:
			ac.setFan(kArgoFan2);
			break;
		case 4:
			ac.setFan(kArgoFan3);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_bosch144_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRBosch144AC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kBosch144Auto);
			break;
		case 2:
			ac.setMode(kBosch144Cool);
			break;
		case 3:
			ac.setMode(kBosch144Dry);
			break;
		case 4:
			ac.setMode(kBosch144Heat);
			break;
		case 5:
			ac.setMode(kBosch144Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kBosch144FanAuto);
			break;
		case 2:
			ac.setFan(kBosch144Fan40);
			break;
		case 3:
			ac.setFan(kBosch144Fan60);
			break;
		case 4:
			ac.setFan(kBosch144Fan80);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_corona_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRCoronaAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		
		case 2:
			ac.setMode(kCoronaAcModeCool);
			break;
		case 3:
			ac.setMode(kCoronaAcModeDry);
			break;
		case 4:
			ac.setMode(kCoronaAcModeHeat);
			break;
		case 5:
			ac.setMode(kCoronaAcModeFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kCoronaAcFanAuto);
			break;
		case 2:
			ac.setFan(kCoronaAcFanLow);
			break;
		case 3:
			ac.setFan(kCoronaAcFanMedium);
			break;
		case 4:
			ac.setFan(kCoronaAcFanHigh);
			break;
	}
	ac.setSwingVToggle((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_delonghi_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRDelonghiAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kDelonghiAcAuto);
			break;
		case 2:
			ac.setMode(kDelonghiAcCool);
			break;
		case 3:
			ac.setMode(kDelonghiAcDry);
			break;
		case 5:
			ac.setMode(kDelonghiAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kDelonghiAcFanAuto);
			break;
		case 2:
			ac.setFan(kDelonghiAcFanLow);
			break;
		case 3:
			ac.setFan(kDelonghiAcFanMedium);
			break;
		case 4:
			ac.setFan(kDelonghiAcFanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_ecoclim_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IREcoclimAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kEcoclimAuto);
			break;
		case 2:
			ac.setMode(kEcoclimCool);
			break;
		case 3:
			ac.setMode(kEcoclimDry);
			break;
		case 4:
			ac.setMode(kEcoclimHeat);
			break;
		case 5:
			ac.setMode(kEcoclimFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kEcoclimFanAuto);
			break;
		case 2:
			ac.setFan(kEcoclimFanMin);
			break;
		case 3:
			ac.setFan(kEcoclimFanMed);
			break;
		case 4:
			ac.setFan(kEcoclimFanMax);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_goodweather_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRGoodweatherAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kGoodweatherAuto);
			break;
		case 2:
			ac.setMode(kGoodweatherCool);
			break;
		case 3:
			ac.setMode(kGoodweatherDry);
			break;
		case 4:
			ac.setMode(kGoodweatherHeat);
			break;
		case 5:
			ac.setMode(kGoodweatherFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kGoodweatherFanAuto);
			break;
		case 2:
			ac.setFan(kGoodweatherFanLow);
			break;
		case 3:
			ac.setFan(kGoodweatherFanMed);
			break;
		case 4:
			ac.setFan(kGoodweatherFanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_haier_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHaierAC ac(13);
	if(temp != 1)
	{
		ac.setCommand(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setCommand(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kHaierAcAuto);
			break;
		case 2:
			ac.setMode(kHaierAcCool);
			break;
		case 3:
			ac.setMode(kHaierAcDry);
			break;
		case 4:
			ac.setMode(kHaierAcHeat);
			break;
		case 5:
			ac.setMode(kHaierAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHaierAcFanAuto);
			break;
		case 2:
			ac.setFan(kHaierAcFanLow);
			break;
		case 3:
			ac.setFan(kHaierAcFanMed);
			break;
		case 4:
			ac.setFan(kHaierAcFanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_haier176_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHaierAC176 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kHaierAcYrw02Auto);
			break;
		case 2:
			ac.setMode(kHaierAcYrw02Cool);
			break;
		case 3:
			ac.setMode(kHaierAcYrw02Dry);
			break;
		case 4:
			ac.setMode(kHaierAcYrw02Heat);
			break;
		case 5:
			ac.setMode(kHaierAcYrw02Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHaierAcYrw02FanAuto);
			break;
		case 2:
			ac.setFan(kHaierAcYrw02FanLow);
			break;
		case 3:
			ac.setFan(kHaierAcYrw02FanMed);
			break;
		case 4:
			ac.setFan(kHaierAcYrw02FanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_haieryrw02_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHaierACYRW02 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kHaierAcYrw02Auto);
			break;
		case 2:
			ac.setMode(kHaierAcYrw02Cool);
			break;
		case 3:
			ac.setMode(kHaierAcYrw02Dry);
			break;
		case 4:
			ac.setMode(kHaierAcYrw02Heat);
			break;
		case 5:
			ac.setMode(kHaierAcYrw02Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHaierAcYrw02FanAuto);
			break;
		case 2:
			ac.setFan(kHaierAcYrw02FanLow);
			break;
		case 3:
			ac.setFan(kHaierAcYrw02FanMed);
			break;
		case 4:
			ac.setFan(kHaierAcYrw02FanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_haier160_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRHaierAC160 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kHaierAcYrw02Auto);
			break;
		case 2:
			ac.setMode(kHaierAcYrw02Cool);
			break;
		case 3:
			ac.setMode(kHaierAcYrw02Dry);
			break;
		case 4:
			ac.setMode(kHaierAcYrw02Heat);
			break;
		case 5:
			ac.setMode(kHaierAcYrw02Fan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kHaierAcYrw02FanAuto);
			break;
		case 2:
			ac.setFan(kHaierAcYrw02FanLow);
			break;
		case 3:
			ac.setFan(kHaierAcYrw02FanMed);
			break;
		case 4:
			ac.setFan(kHaierAcYrw02FanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_kelon_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRKelonAc ac(13);
	if(temp != 1)
	{
		ac.setTogglePower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setTogglePower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kKelonModeCool);
			break;
		case 3:
			ac.setMode(kKelonModeDry);
			break;
		case 4:
			ac.setMode(kKelonModeHeat);
			break;
		case 5:
			ac.setMode(kKelonModeFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kKelonFanAuto);
			break;
		case 2:
			ac.setFan(kKelonFanMin);
			break;
		case 3:
			ac.setFan(kKelonFanMedium);
			break;
		case 4:
			ac.setFan(kKelonFanMax);
			break;
	}
	ac.setToggleSwingVertical((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_kelvinator_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRKelvinatorAC ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kKelvinatorAuto);
			break;
		case 2:
			ac.setMode(kKelvinatorCool);
			break;
		case 3:
			ac.setMode(kKelvinatorDry);
			break;
		case 4:
			ac.setMode(kKelvinatorHeat);
			break;
		case 5:
			ac.setMode(kKelvinatorFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kKelvinatorFanAuto);
			break;
		case 2:
			ac.setFan(kKelvinatorFanMin);
			break;
		case 4:
			ac.setFan(kKelvinatorFanMax);
			break;
	}
	ac.setSwingHorizontal((swing >> 1)&0x01);
	ac.send(kNoRepeat);
}

void setup_mirage_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRMirageAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kMirageAcCool);
			break;
		case 3:
			ac.setMode(kMirageAcDry);
			break;
		case 4:
			ac.setMode(kMirageAcHeat);
			break;
		case 5:
			ac.setMode(kMirageAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kMirageAcFanAuto);
			break;
		case 2:
			ac.setFan(kMirageAcFanLow);
			break;
		case 3:
			ac.setFan(kMirageAcFanMed);
			break;
		case 4:
			ac.setFan(kMirageAcFanHigh);
			break;
	}
	ac.setSwingH((swing >> 1)&0x01);
	ac.send(kNoRepeat);
}

void setup_neoclima_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRNeoclimaAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kNeoclimaAuto);
			break;
		case 2:
			ac.setMode(kNeoclimaCool);
			break;
		case 3:
			ac.setMode(kNeoclimaDry);
			break;
		case 4:
			ac.setMode(kNeoclimaHeat);
			break;
		case 5:
			ac.setMode(kNeoclimaFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kNeoclimaFanAuto);
			break;
		case 2:
			ac.setFan(kNeoclimaFanLow);
			break;
		case 3:
			ac.setFan(kNeoclimaFanMed);
			break;
		case 4:
			ac.setFan(kNeoclimaFanHigh);
			break;
	}
	ac.setSwingH((swing >> 1)&0x01);
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_rhoss_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRRhossAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kRhossModeAuto);
			break;
		case 2:
			ac.setMode(kRhossModeCool);
			break;
		case 3:
			ac.setMode(kRhossModeDry);
			break;
		case 4:
			ac.setMode(kRhossModeHeat);
			break;
		case 5:
			ac.setMode(kRhossModeFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kRhossFanAuto);
			break;
		case 2:
			ac.setFan(kRhossFanMin);
			break;
		case 3:
			ac.setFan(kRhossFanMed);
			break;
		case 4:
			ac.setFan(kRhossFanMax);
			break;
	}
	ac.setSwing((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_technibel_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRTechnibelAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kTechnibelAcCool);
			break;
		case 3:
			ac.setMode(kTechnibelAcDry);
			break;
		case 4:
			ac.setMode(kTechnibelAcHeat);
			break;
		case 5:
			ac.setMode(kTechnibelAcFan);
			break;
	}
	switch(fan)
	{
		case 2:
			ac.setFan(kTechnibelAcFanLow);
			break;
		case 3:
			ac.setFan(kTechnibelAcFanMedium);
			break;
		case 4:
			ac.setFan(kTechnibelAcFanHigh);
			break;
	}
	ac.setSwing((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_teco_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRTecoAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kTecoAuto);
			break;
		case 2:
			ac.setMode(kTecoCool);
			break;
		case 3:
			ac.setMode(kTecoDry);
			break;
		case 4:
			ac.setMode(kTecoHeat);
			break;
		case 5:
			ac.setMode(kTecoFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kTecoFanAuto);
			break;
		case 2:
			ac.setFan(kTecoFanLow);
			break;
		case 3:
			ac.setFan(kTecoFanMed);
			break;
		case 4:
			ac.setFan(kTecoFanHigh);
			break;
	}
	ac.setSwing((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_transcold_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRTranscoldAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kTranscoldAuto);
			break;
		case 2:
			ac.setMode(kTranscoldCool);
			break;
		case 3:
			ac.setMode(kTranscoldDry);
			break;
		case 4:
			ac.setMode(kTranscoldHeat);
			break;
		case 5:
			ac.setMode(kTranscoldFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kTranscoldFanAuto);
			break;
		case 2:
			ac.setFan(kTranscoldFanMin);
			break;
		case 3:
			ac.setFan(kTranscoldFanMed);
			break;
		case 4:
			ac.setFan(kTranscoldFanMax);
			break;
	}
	if((swing >> 2) & 0x01) ac.setSwing();
	ac.send(kNoRepeat);
}

void setup_trotecesp_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRTrotecESP ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kTrotecAuto);
			break;
		case 2:
			ac.setMode(kTrotecCool);
			break;
		case 3:
			ac.setMode(kTrotecDry);
			break;
		case 5:
			ac.setMode(kTrotecFan);
			break;
	}
	switch(fan)
	{
		case 2:
			ac.setFan(kTrotecFanLow);
			break;
		case 3:
			ac.setFan(kTrotecFanMed);
			break;
		case 4:
			ac.setFan(kTrotecFanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_trotec3550_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRTrotec3550 ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kTrotecAuto);
			break;
		case 2:
			ac.setMode(kTrotecCool);
			break;
		case 3:
			ac.setMode(kTrotecDry);
			break;
		case 5:
			ac.setMode(kTrotecFan);
			break;
	}
	switch(fan)
	{
		case 2:
			ac.setFan(kTrotecFanLow);
			break;
		case 3:
			ac.setFan(kTrotecFanMed);
			break;
		case 4:
			ac.setFan(kTrotecFanHigh);
			break;
	}
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_truma_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRTrumaAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kTrumaAuto);
			break;
		case 2:
			ac.setMode(kTrumaCool);
			break;
		case 5:
			ac.setMode(kTrumaFan);
			break;
	}
	switch(fan)
	{
		case 2:
			ac.setFan(kTrumaFanLow);
			break;
		case 3:
			ac.setFan(kTrumaFanMed);
			break;
		case 4:
			ac.setFan(kTrumaFanHigh);
			break;
	}
	ac.send(kNoRepeat);
}

void setup_vestel_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRVestelAc ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kVestelAcAuto);
			break;
		case 2:
			ac.setMode(kVestelAcCool);
			break;
		case 3:
			ac.setMode(kVestelAcDry);
			break;
		case 4:
			ac.setMode(kVestelAcHeat);
			break;
		case 5:
			ac.setMode(kVestelAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kVestelAcFanAuto);
			break;
		case 2:
			ac.setFan(kVestelAcFanLow);
			break;
		case 3:
			ac.setFan(kVestelAcFanMed);
			break;
		case 4:
			ac.setFan(kVestelAcFanHigh);
			break;
	}
	ac.setSwing((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_voltas_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRVoltas ac(13);
	if(temp != 1)
	{
		ac.setPower(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPower(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 2:
			ac.setMode(kVoltasCool);
			break;
		case 3:
			ac.setMode(kVoltasDry);
			break;
		case 4:
			ac.setMode(kVoltasHeat);
			break;
		case 5:
			ac.setMode(kVoltasFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kVoltasFanAuto);
			break;
		case 2:
			ac.setFan(kVoltasFanLow);
			break;
		case 3:
			ac.setFan(kVoltasFanMed);
			break;
		case 4:
			ac.setFan(kVoltasFanHigh);
			break;
	}
	ac.setSwingH((swing >> 1)&0x01);
	ac.setSwingV((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_whirlpool_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	IRWhirlpoolAc ac(13);
	if(temp != 1)
	{
		ac.setPowerToggle(1);
		ac.setTemp(temp);
	}
	else
	{
		ac.setPowerToggle(0);
		ac.setTemp(26);
	}
	switch(mode)
	{
		case 1:
			ac.setMode(kWhirlpoolAcAuto);
			break;
		case 2:
			ac.setMode(kWhirlpoolAcCool);
			break;
		case 3:
			ac.setMode(kWhirlpoolAcDry);
			break;
		case 4:
			ac.setMode(kWhirlpoolAcHeat);
			break;
		case 5:
			ac.setMode(kWhirlpoolAcFan);
			break;
	}
	switch(fan)
	{
		case 1:
			ac.setFan(kWhirlpoolAcFanAuto);
			break;
		case 2:
			ac.setFan(kWhirlpoolAcFanLow);
			break;
		case 3:
			ac.setFan(kWhirlpoolAcFanMedium);
			break;
		case 4:
			ac.setFan(kWhirlpoolAcFanHigh);
			break;
	}
	ac.setSwing((swing >> 2)&0x01);
	ac.send(kNoRepeat);
}

void setup_daikin_custom1_ac(uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	uint8_t i;
	uint8_t j;
	uint16_t k = 1;
	uint8_t code[25] = {0x88, 0x5B, 0xE8, 0x12, 0x20, 0x00, 0x72, 0x88, 0x5B, 0xE8, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, fan, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00};
	if(temp != 1)
	{
		code[14] = 0x84;
		code[17] = (temp - 9) << 1;
		code[17] = reserval_byte(code[17]);
	}
	else
	{
		code[14] = 0x04;
		code[17] = 0x44;
	}
	switch(mode)
	{
		case 1:
			code[12] = 0xCE;
			break;
		case 2:
			code[12] = 0xCE;
			break;
		case 3:
			code[12] = 0xC4;
			break;
		case 4:
			code[12] = 0xCE;
			break;
		case 5:
			code[12] = 0xC6;
			break;
		default:
			code[12] = 0xCE;
			break;
	}
	switch(fan)
	{
		case 1:
			code[18] = 0x40;
			break;
		case 2:
			code[18] = 0x48;
			break;
		case 3:
			code[18] = 0x4C;
			break;
		case 4:
			code[18] = 0x4A;
			break;
	}
	for(i=7; i<24; i++) code[24] += reserval_byte(code[i]);
	code[24] = reserval_byte(code[24]);
	data_ac[0] = 409;
	data_ac[k++] = 1609;
	data_ac[k++] = 679;
	for(i=0; i<7; i++)
	{
		for(j=8; j>0; j--)
		{
			if((code[i] >> (j-1)) & 0x01)
			{
				data_ac[k++] = 122;
				data_ac[k++] = 564;
			}
			else
			{
				data_ac[k++] = 122;
				data_ac[k++] = 221;
			}	
		}
	}
	data_ac[k++] = 122;
	data_ac[k++] = 9371;
	data_ac[k++] = 1611;
	data_ac[k++] = 679;
	for(i=7; i<25; i++)
	{
		for(j=8; j>0; j--)
		{
			if((code[i] >> (j-1)) & 0x01)
			{
				data_ac[k++] = 122;
				data_ac[k++] = 564;
			}
			else
			{
				data_ac[k++] = 122;
				data_ac[k++] = 221;
			}	
		}
	}
	data_ac[k++] = 122;
	data_ac[k++] = 0;
}

void setup_send_ac(uint8_t idl, uint8_t idh, uint8_t index, uint8_t temp, uint8_t mode, uint8_t fan, uint8_t swing)
{
	if((idl+256*idh == _AIRTON_AC) && (index == 1)) setup_airton_ac(temp, mode, fan, swing);
	if((idl+256*idh == _AIRWELL_AC) && (index == 1)) setup_airwell_ac(temp, mode, fan, swing);
	if((idl+256*idh == _ALASKA_AC) && (index == 1)) setup_teco_ac(temp, mode, fan, swing);
	if((idl+256*idh == _AMCOR_AC) && (index == 1)) setup_amcor_ac(temp, mode, fan, swing);
	if((idl+256*idh == _AQUA_AC) && (index == 1)) setup_sanyo_ac(temp, mode, fan, swing);
	if((idl+256*idh == _AQUA_AC) && (index == 2)) setup_sanyo88_ac(temp, mode, fan, swing);
	if((idl+256*idh == _AQUA_AC) && (index == 3)) setup_haieryrw02_ac(temp, mode, fan, swing);
	if((idl+256*idh == _ARGO_AC) && (index == 1)) setup_argo_ac(temp, mode, fan, swing);
	if((idl+256*idh == _ASANZO_AC) && (index == 1)) setup_electra_ac(temp, mode, fan, swing);
	if((idl+256*idh == _BOSCH_AC) && (index == 1)) setup_bosch144_ac(temp, mode, fan, swing);
	if((idl+256*idh == _BEKO_AC) && (index == 1)) setup_coolix_ac(temp, mode, fan, swing);
	if((idl+256*idh == _CARRIER_AC) && (index == 1)) setup_carrier64_ac(temp, mode, fan, swing);
	if((idl+256*idh == _COMFEE_AC) && (index == 1)) setup_midea_ac(temp, mode, fan, swing);
	if((idl+256*idh == _COOLIX_AC) && (index == 1)) setup_coolix_ac(temp, mode, fan, swing);
	if((idl+256*idh == _CORONA_AC) && (index == 1)) setup_corona_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 1)) setup_daikinesp_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 2)) setup_daikin2_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 3)) setup_daikin216_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 4)) setup_daikin160_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 5)) setup_daikin176_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 6)) setup_daikin128_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 7)) setup_daikin152_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 8)) setup_daikin64_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DAIKIN_AC) && (index == 9)) setup_daikin_custom1_ac(temp, mode, fan, swing);
	if((idl+256*idh == _DELONGHI_AC) && (index == 1)) setup_delonghi_ac(temp, mode, fan, swing);
	if((idl+256*idh == _ECOCLIM_AC) && (index == 1)) setup_ecoclim_ac(temp, mode, fan, swing);
	if((idl+256*idh == _ELECTRA_AC) && (index == 1)) setup_electra_ac(temp, mode, fan, swing);
	if((idl+256*idh == _FUJITSU_AC) && (index == 1)) setup_fujitsu_ac(temp, mode, fan, swing);
	if((idl+256*idh == _GOODWEATHER_AC) && (index == 1)) setup_goodweather_ac(temp, mode, fan, swing);
	if((idl+256*idh == _GREE_AC) && (index == 1)) setup_gree_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HAIER_AC) && (index == 1)) setup_haier_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HAIER_AC) && (index == 2)) setup_haier176_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HAIER_AC) && (index == 3)) setup_haier160_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HAIER_AC) && (index == 4)) setup_haieryrw02_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HISENSE_AC) && (index == 1)) setup_kelon_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HITACHI_AC) && (index == 1)) setup_hitachi_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HITACHI_AC) && (index == 2)) setup_hitachi1_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HITACHI_AC) && (index == 3)) setup_hitachi424_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HITACHI_AC) && (index == 4)) setup_hitachi344_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HITACHI_AC) && (index == 5)) setup_hitachi264_ac(temp, mode, fan, swing);
	if((idl+256*idh == _HITACHI_AC) && (index == 6)) setup_hitachi296_ac(temp, mode, fan, swing);
	if((idl+256*idh == _KELON_AC) && (index == 1)) setup_kelon_ac(temp, mode, fan, swing);
	if((idl+256*idh == _KELVINATOR_AC) && (index == 1)) setup_kelvinator_ac(temp, mode, fan, swing);
	if((idl+256*idh == _LG_AC) && (index == 1)) setup_lg_ac(temp, mode, fan, swing);
	if((idl+256*idh == _LG_AC) && (index == 2)) setup_lg2_ac(temp, mode, fan, swing);
	if((idl+256*idh == _MIDEA_AC) && (index == 1)) setup_midea_ac(temp, mode, fan, swing);
	if((idl+256*idh == _MIDEA_AC) && (index == 2)) setup_coolix_ac(temp, mode, fan, swing);
	if((idl+256*idh == _MIRAGE_AC) && (index == 1)) setup_mirage_ac(temp, mode, fan, swing);
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 1)) setup_mitsubishi_ac(temp, mode, fan, swing);
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 2)) setup_mitsubishi136_ac(temp, mode, fan, swing);
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 3)) setup_mitsubishi112_ac(temp, mode, fan, swing);
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 4)) setup_mitsubishi152_ac(temp, mode, fan, swing);
	if((idl+256*idh == _MITSUBISHI_AC) && (index == 5)) setup_mitsubishi88_ac(temp, mode, fan, swing);
	if((idl+256*idh == _NEOCLIMA_AC) && (index == 1)) setup_neoclima_ac(temp, mode, fan, swing);
	if((idl+256*idh == _PANASONIC_AC) && (index == 1)) setup_panasonic_ac(temp, mode, fan, swing);
	if((idl+256*idh == _PANASONIC_AC) && (index == 2)) setup_panasonic32_ac(temp, mode, fan, swing);
	if((idl+256*idh == _RHOSS_AC) && (index == 1)) setup_rhoss_ac(temp, mode, fan, swing);
	if((idl+256*idh == _SAMSUNG_AC) && (index == 1)) setup_samsung_ac(temp, mode, fan, swing);
	if((idl+256*idh == _SHARP_AC) && (index == 1)) setup_sharp_ac(temp, mode, fan, swing);
	if((idl+256*idh == _TCL_AC) && (index == 1)) setup_tcl_ac(temp, mode, fan, swing);
	if((idl+256*idh == _TECHNIBEL_AC) && (index == 1)) setup_technibel_ac(temp, mode, fan, swing);
	if((idl+256*idh == _TECO_AC) && (index == 1)) setup_teco_ac(temp, mode, fan, swing);
	if((idl+256*idh == _TOSHIBA_AC) && (index == 1)) setup_toshiba_ac(temp, mode, fan, swing);
	if((idl+256*idh == _TRANSCOLD_AC) && (index == 1)) setup_transcold_ac(temp, mode, fan, swing);
	if((idl+256*idh == _TROTEC_AC) && (index == 1)) setup_trotecesp_ac(temp, mode, fan, swing);
	if((idl+256*idh == _TROTEC_AC) && (index == 2)) setup_trotec3550_ac(temp, mode, fan, swing);	
	if((idl+256*idh == _TRUMA_AC) && (index == 1)) setup_truma_ac(temp, mode, fan, swing);
	if((idl+256*idh == _VESTEL_AC) && (index == 1)) setup_vestel_ac(temp, mode, fan, swing);
	if((idl+256*idh == _VOLTAS_AC) && (index == 1)) setup_voltas_ac(temp, mode, fan, swing);
	if((idl+256*idh == _WHIRLPOOL_AC) && (index == 1)) setup_whirlpool_ac(temp, mode, fan, swing);
}

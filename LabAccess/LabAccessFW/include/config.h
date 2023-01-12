#ifndef CONFIG_H
#define CONFIG_H

#include <ArduinoJson.h>
#include <StreamUtils.h>

#include "hex_utils.h"
#define DEBUG_FLAG "CFG"
#include "la_debug.h"

#define LOCAL_OTA
#define REMOTE_OTA
#define MANUAL_SIGNING


bool readConfigFile(JsonDocument &json){
	File configFile;
	configFile = LittleFS.open(config_filename, "r");
	if (!configFile)
	{
		LA_DPRINTF("Error opening config file for reading\n");
		configFile.close();
		return false;
	}
	LA_DPRINTF("Opened config file for reading\n");
	ReadBufferingStream readBufStream(configFile, 64);
	DeserializationError desErr = deserializeJson(json, readBufStream);
	configFile.close();
	if (desErr)
	{
		LA_DPRINTF("Deserialization error: %s\n", desErr.f_str());
		return false;
	}
	LA_DPRINTF("Config file deserialized\n");
	LA_DJSON(json);
	return true;
}

bool writeConfigFile(JsonDocument &json){
	File configFile;
	configFile = LittleFS.open(config_filename, "w");
	if (!configFile)
	{
		LA_DPRINTF("Error opening config file for writing\n");
		configFile.close();
		return false;
	}
	LA_DPRINTF("Opened config file for writing\n");
	WriteBufferingStream writeBufStream(configFile, 64);
	size_t bw = serializeJson(json, writeBufStream);
	configFile.flush();
	configFile.close();
	LA_DJSON(json);
	LA_DPRINTF("%lu bytes written\n", bw);
	return true;
}

struct pin_config_t
{
	uint8_t csn, rst, irq, led, out, esw, isw, buz, pwr;
};

enum LabAccessMode
{
	LA_MODE_NONE = 0,
	LA_MODE_DOOR = 1,
	LA_MODE_LASER = 2,
	LA_MODE_TIG = 3,
	LA_MODE_3D = 4,
	LA_MODE_CNC = 5,
	LA_MODE_OTHER = 6
};

#define PIN_CONFIG_NONE {.csn = 0xFF, .rst = 0xFF, .irq = 0xFF, .led = 0xFF, .out = 0xFF, .esw = 0xFF, .isw = 0xFF, .buz = 0xFF, .pwr = 0xFF}

const struct pin_config_t PROGMEM pin_config[] = {
	PIN_CONFIG_NONE, // NONE
	{.csn =    0, .rst = 0xFF, .irq =    5, .led = 0xFF, .out =    2, .esw =    4, .isw = 0xFF, .buz =   15, .pwr =   16}, // DOOR
	{.csn = 0xFF, .rst = 0xFF, .irq = 0xFF, .led = 0xFF, .out = 0xFF, .esw = 0xFF, .isw = 0xFF, .buz = 0xFF, .pwr = 0xFF}, // LASER
	{.csn = 0xFF, .rst = 0xFF, .irq = 0xFF, .led = 0xFF, .out = 0xFF, .esw = 0xFF, .isw = 0xFF, .buz = 0xFF, .pwr = 0xFF}, // TIG
	{.csn = 0xFF, .rst = 0xFF, .irq = 0xFF, .led = 0xFF, .out = 0xFF, .esw = 0xFF, .isw = 0xFF, .buz = 0xFF, .pwr = 0xFF}, // 3D
	{.csn = 0xFF, .rst = 0xFF, .irq = 0xFF, .led = 0xFF, .out = 0xFF, .esw = 0xFF, .isw = 0xFF, .buz = 0xFF, .pwr = 0xFF}, // CNC
	{.csn =   15, .rst = 0xFF, .irq = 0xFF, .led =    2, .out = 0xFF, .esw = 0xFF, .isw = 0xFF, .buz = 0xFF, .pwr = 0xFF}, // OTHER
};


static uint8_t dev_mode = LA_MODE_NONE;
static uint8_t nfc_key[16] = {0};
static String accessScriptURL = "*****************************************************************************************";
static String logScriptURL = "*****************************************************************************************";
static String dev_name = "";
static String ap_key = "";
static String ota_key = "";
static struct pin_config_t pin = PIN_CONFIG_NONE;

void init_config(JsonDocument &json)
{
	LA_DJSON(json);
	dev_mode = json["mode"];
	memcpy_P(&pin, &pin_config[dev_mode], sizeof(struct pin_config_t));
	LA_DPRINTF("dev %2u, pin {csn %2u rst %2u irq %2u led %2u out %2u esw %2u buz %2u pwr %2u}\n", 
		dev_mode, pin.csn, pin.rst, pin.irq, pin.led, pin.out, pin.esw, pin.buz, pin.pwr);
	String tmp = json["nfc_key"];
	char_to_hex(tmp.c_str(), 16, nfc_key);
	dev_name = json["name"].as<const char*>();
	accessScriptURL = String("/macros/s/") + json["api_id"].as<const char*>() + "/exec?boardName=" + dev_name;
	logScriptURL = String("/macros/s/") + json["log_id"].as<const char*>() + "/exec?log";
	ap_key = json["ap_key"].as<const char*>();
	ota_key = json["ota_key"].as<const char*>();
}

#undef DEBUG_FLAG
#endif

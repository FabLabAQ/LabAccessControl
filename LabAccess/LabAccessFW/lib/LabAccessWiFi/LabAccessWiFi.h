/* 
 * This file is part of the SmartBirdFeeder https://github.com/FabLabAQ/SmartBirdFeeder
 * Copyright (c) 2021 FabLabAQ <info@fablaquila.org>.
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LAB_ACCESS_WIFI_H
#define LAB_ACCESS_WIFI_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#define DEBUG_FLAG "[WIFI]"
#include "debug.h"
#include "config.h"

enum LabAccessMode {
	LA_MODE_NONE = 0,
	LA_MODE_DOOR = 1,
	LA_MODE_LASER = 2,
	LA_MODE_TIG = 3,
	LA_MODE_3D = 4,
	LA_MODE_CNC = 5,
	LA_MODE_OTHER = 6
};

static const uint8_t default_key_length = 16;
static const char* config_filename = "/config.json";

static WiFiManager wm;


void setupWiFi(){
	WiFiManagerParameter device_name("name", "Device Name", "", 32);
	char default_wifi_key[default_key_length];
	#ifdef LOCAL_OTA
	char default_ota_key[default_key_length];
	#endif
	for (uint8_t i = 0; i < default_key_length; i++) {
		default_wifi_key[i] = secureRandom(32, 126);
		#ifdef LOCAL_OTA
		default_ota_key[i] = secureRandom(32, 126);
		#endif
	}
	WiFiManagerParameter ap_key("ap_key", "AP mode password", default_wifi_key, default_key_length);
	#ifdef LOCAL_OTA
	WiFiManagerParameter ota_key("ota_key", "OTA update password", default_ota_key, default_key_length);
	#endif
	WiFiManagerParameter nfc_key("nfc_key", "NFC card key", "", 32);
	WiFiManagerParameter api_id("api_id", "API script ID", "", 64);
	WiFiManagerParameter log_id("log_id", "Log script ID", "", 64);
	WiFiManagerParameter mode_sel(
		const char* custom_radio_str = 
		"<br/><label for='mode'>Operating mode selection</label>"
		"<br><input type='radio' name='mode' value='1'> Door opener"
		"<br><input type='radio' name='mode' value='2'> Laser engraver"
		"<br><input type='radio' name='mode' value='3'> TIG welder"
		"<br><input type='radio' name='mode' value='4'> 3D printer"
		"<br><input type='radio' name='mode' value='5'> CNC mill"
		"<br><input type='radio' name='mode' value='6'> Other"
	);
	
	wm.addParameter(&device_name);
	#ifdef LOCAL_OTA
	wm.addParameter(&ota_key);
	#endif
	wm.addParameter(&ap_key);
	wm.addParameter(&nfc_key);
	wm.addParameter(&api_id);
	wm.addParameter(&log_id);
	wm.addParameter(&mode_sel);
	wm.setSaveConfigCallback(configCallback);


	if(LittleFS.begin()){
		DPRINTF("Mount successful\n");
		if(LittleFS.exists(config_filename)){
			DPRINTF("Config file exists\n");
			File configFile = SPIFFS.open("/config.json", "r");
			if (configFile) {
				DPRINTF("Opened config file\n");
				size_t size = configFile.size();
        		std::unique_ptr<char[]> buf(new char[size]);
        		configFile.readBytes(buf.get(), size);
        		auto deserializeError = deserializeJson(jsonConfig, buf.get());
				configFile.close();
        		if ( ! deserializeError ) {
					DPRINTF("Config file deserialized successfully\n");
					wm.autoConnect(jsonConfig["name"], jsonConfig["ap_key"]);
				}
				else {
					DPRINTF("Deserialization error: %S\n", deserializeError.f_str());
				}
			}
			else {
				DPRINTF("Config file does not exist, entering configuration\n");
				wm.autoConnect("LabAccess");
			}
		}
	}
	else {
		DPRINTF("Mount failed, formatting filesystem\n");
		if (LittleFS.format()){
			DPRINTF("Formatted, entering new configuration\n");
			wm.autoConnect("LabAccess");
		}
		else {
			DPRINTF("Format failed, restarting\n");
			ESP.restart();
		}
	}

}

static void configCallback(){
	bool configDone = false;
	
	jsonConfig["name"] = getParam("name");
	configDone = configDone && (jsonConfig["name"] != "");
	jsonConfig["nfc_key"] = getParam("nfc_key");
	configDone = configDone && (jsonConfig["nfc_key"] != "");
	jsonConfig["ap_key"] = getParam("ap_key");
	configDone = configDone && (jsonConfig["ap_key"] != "");
	jsonConfig["api_id"] = getParam("api_id");
	configDone = configDone && (jsonConfig["api_id"] != "");
	jsonConfig["log_id"] = getParam("log_id");
	configDone = configDone && (jsonConfig["log_id"] != "");
	jsonConfig["mode"] = getParam("mode").toInt();
	configDone = configDone && (jsonConfig["mode"] != LA_MODE_NONE);
	#ifdef LOCAL_OTA
	jsonConfig["ota_key"] = getParam("ota_key");
	configDone = configDone && (jsonConfig["ota_key"] != "");
	#endif

	if (configDone) {
		File configFile = LittleFS.open(config_filename, "w");
		if (configFile) {
			DPRINTF("Writing config file");
			serializeJson(jsonConfig, configFile);
			configFile.close();
		}
		else {
			DPRINTF("Failed to open config for writing\n");
			ESP.restart();
		}
	}
	else {
		DPRINTF("Incorrect configuration, restarting\n");
		ESP.restart();
	}
}

static String getParam(String name){
  //read parameter from server, for customhmtl input
  String value;
  if(wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
} 


#endif // LAB_ACCESS_WIFI_H
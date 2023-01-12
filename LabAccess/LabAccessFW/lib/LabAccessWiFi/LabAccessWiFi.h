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
#include "config.h"
#define DEBUG_FLAG "WIFI"
#include "la_debug.h"

static const uint8_t default_key_length = 16;

static bool conf_done = false;
const uint32_t conf_led_time = 500;

static WiFiManager wm;

static String getParam(String name)
{
	// read parameter from server, for customhmtl input
	String value = "";
	if (wm.server->hasArg(name))
	{
		value = wm.server->arg(name);
	}
	return value;
}

static void configCallback()
{

	DynamicJsonDocument jsonConfig(1024);
	File tmpFile = LittleFS.open(config_filename, "r");
	deserializeJson(jsonConfig, tmpFile);
	tmpFile.close();

	int configDone = 1;
#ifdef LOCAL_OTA
	const String keys[] = {"name", "nfc_key", "ap_key", "api_id", "log_id", "mode", "ota_key"};
#else
	const String keys[] = {"name", "nfc_key", "ap_key", "api_id", "log_id", "mode"};
#endif

	for (unsigned int i = 0; i < (sizeof(keys) / sizeof(keys[0])); i++)
	{
		String key = keys[i];
		String param = getParam(key);
		LA_DPRINTF("param key %s value %s\n", key.c_str(), param.c_str());
		LA_DPRINTF("jsonConfig[%s] = %s\n", key.c_str(), jsonConfig[key].as<String>().c_str());
		if ((jsonConfig.containsKey(key)) && (jsonConfig[key] != ""))
		{

			if (param != "")
			{
				if (key == "mode")
				{
					jsonConfig[key] = param.toInt();
				}
				else
				{
					jsonConfig[key] = param;
				}
			}
		}
		else
		{
			// TODO: remove duplicate code
			if (param != "")
			{
				if (key == "mode")
				{
					jsonConfig[key] = param.toInt();
				}
				else
				{
					jsonConfig[key] = param;
				}
			}
			else
			{
				configDone = 0;
				break;
			}
		}
	}

	LA_DJSON(jsonConfig);

	if (configDone)
	{
		File configFile = LittleFS.open(config_filename, "w");
		if (configFile)
		{
			LA_DPRINTF("Writing config file");
			serializeJson(jsonConfig, configFile);
			configFile.close();
			conf_done = true;
		}
		else
		{
			LA_DPRINTF("Failed to open config for writing\n");
			ESP.restart();
		}
	}
	else
	{
		LA_DPRINTF("Incorrect configuration\n");
		conf_done = false;
	}
}

void wait_config()
{
	uint32_t m = millis();
	pinMode(LED_BUILTIN, OUTPUT);
	while (!conf_done)
	{
		yield();
		wm.process();
		if (millis() > m + conf_led_time)
		{
			m = millis();
			digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
		}
	}
	pinMode(LED_BUILTIN, INPUT);
}

char default_wifi_key[default_key_length];
WiFiManagerParameter dev_name_param("name", "Device Name", "", 32);
WiFiManagerParameter ap_key_param("ap_key", "AP mode password", default_wifi_key, default_key_length);
#ifdef LOCAL_OTA
char default_ota_key[default_key_length];
WiFiManagerParameter ota_key_param("ota_key", "OTA update password", default_ota_key, default_key_length);
#endif
WiFiManagerParameter nfc_key_param("nfc_key", "NFC card key", "", 32);
WiFiManagerParameter api_id_param("api_id", "API script ID", "", 64);
WiFiManagerParameter log_id_param("log_id", "Log script ID", "", 64);
WiFiManagerParameter mode_param(
	"<br/><label for='mode'>Operating mode selection</label>"
	"<br><input type='radio' name='mode' value='1'> Door opener"
	"<br><input type='radio' name='mode' value='2'> Laser engraver"
	"<br><input type='radio' name='mode' value='3'> TIG welder"
	"<br><input type='radio' name='mode' value='4'> 3D printer"
	"<br><input type='radio' name='mode' value='5'> CNC mill"
	"<br><input type='radio' name='mode' value='6'> Other");

void init_config_portal()
{
#ifdef LAB_ACCESS_DEBUG
	wm.setDebugOutput(true);
#else
	wm.setDebugOutput(false);
#endif

	for (uint8_t i = 0; i < default_key_length; i++)
	{
		default_wifi_key[i] = secureRandom(32, 126);
#ifdef LOCAL_OTA
		default_ota_key[i] = secureRandom(32, 126);
#endif
	}

	wm.addParameter(&dev_name_param);
#ifdef LOCAL_OTA
	wm.addParameter(&ota_key_param);
#endif
	wm.addParameter(&ap_key_param);
	wm.addParameter(&nfc_key_param);
	wm.addParameter(&api_id_param);
	wm.addParameter(&log_id_param);
	wm.addParameter(&mode_param);
	// wm.setSaveConfigCallback(configCallback);
	wm.setSaveParamsCallback(configCallback);
	wm.setConnectTimeout(60);
	// wm.setConfigPortalTimeout(600);
	wm.setConfigPortalBlocking(false);
}

void setupWiFi()
{

	init_config_portal();

	if (LittleFS.begin())
	{
		LA_DPRINTF("Mount successful\n");
		if (LittleFS.exists(config_filename))
		{
			LA_DPRINTF("Config file exists\n");
			File configFile = LittleFS.open(config_filename, "r");
			if (configFile)
			{
				LA_DPRINTF("Opened config file\n");
				// size_t size = configFile.size();
				// std::unique_ptr<char[]> buf(new char[size]);
				DynamicJsonDocument jsonConfig(1024);
				// configFile.readBytes(buf.get(), size);
				DeserializationError deserializeError = deserializeJson(jsonConfig, configFile);
				configFile.close();
				if ((!deserializeError) && (jsonConfig["mode"] != (uint8_t)LA_MODE_NONE))
				{
					LA_DPRINTF("Config file deserialized successfully\n");
					init_config(jsonConfig);
					wm.autoConnect(dev_name.c_str(), ap_key.c_str());
					conf_done = true;
				}
				else
				{
					LA_DPRINTF("Deserialization error: %s\n", deserializeError.c_str());
					wm.startConfigPortal("LabAccess");
				}
			}
			else
			{
				LA_DPRINTF("File error\n");
				wm.startConfigPortal("LabAccess");
			}
		}
		else
		{
			LA_DPRINTF("Config file does not exist, entering configuration\n");
			wm.startConfigPortal("LabAccess");
		}
	}
	else
	{
		LA_DPRINTF("Mount failed, formatting filesystem\n");
		if (LittleFS.format())
		{
			LA_DPRINTF("Formatted, entering new configuration\n");
			wm.startConfigPortal("LabAccess");
		}
		else
		{
			LA_DPRINTF("Format failed, restarting\n");
			ESP.restart();
		}
	}
	wait_config();
}

#undef DEBUG_FLAG
#endif // LAB_ACCESS_WIFI_H
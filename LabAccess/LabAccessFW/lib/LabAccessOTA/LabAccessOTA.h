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

#ifndef LAB_ACCESS_OTA_H
#define LAB_ACCESS_OTA_H

#include <Arduino.h>
#define DEBUG_FLAG "[OTA]"
#include "debug.h"
#include "config.h"

#include <ArduinoOTA.h>

static const int current_major = 0;
static const int current_minor = 1;



#ifdef MANUAL_SIGNING
const char pubkey[] PROGMEM = R"EOF(
-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAm5YKLFShvR1ZCRqnUv40
5/8KA+bTdYwZCauscumzb1kXqyY35OKjCeZoEpJXx1lS8sWJc2QBpgqz++cGXecT
Nl++NIvB3s1vURpXuPSoBq0QlDHNOohR2ASGcPerhE6mMvq4Sa7KEXCFr0ItTM7H
QMr/MxDaoy1Q3F7dljYvpWBLSCUNRPvutcwAZt02WLj8zVba3NhTxBrj9FHiekbC
bK0NMOvYiqRuDzgC5+BFucmO6nFzg78ydR1YaTrrlc13BA7kw4w1QhnpKwA99i3I
2ZwRrdp+vBLqsNpfHghKTAS+JIa0aPQfrHfk1SUeDG7CO993ktjetV5UyDefRmCD
8wIDAQAB
-----END PUBLIC KEY-----
)EOF";
BearSSL::PublicKey *signPubKey = nullptr;
BearSSL::HashSHA256 *hash;
BearSSL::SigningVerifier *sign;
#endif

#ifdef LOCAL_OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#endif

#ifdef REMOTE_OTA
#include "ESP8266httpUpdate.h"
#include <ESP8266HTTPClient.h>
#include "ArduinoJson.h"
static HTTPClient ota_http_client;
static WiFiClientSecure ota_wifi_client;
static const char *github_root_ca PROGMEM =
	"-----BEGIN CERTIFICATE-----\
	MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\
	MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\
	d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\
	ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\
	MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\
	LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\
	RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\
	+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\
	PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\
	xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\
	Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\
	hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\
	EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\
	MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\
	FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\
	nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\
	eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\
	hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\
	Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\
	vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\
	+OkuE6N36B9K\
	-----END CERTIFICATE-----";
static const X509List github_root_cert_store(github_root_ca);
static const char *update_url = "https://github.com/FabLabAQ/LabAccessControl/releases/latest/download/firmware.bin";
static const char *tag_url = "https://api.github.com/repos/FabLabAQ/LabAccessControl/releases/latest";
#ifdef LAB_ACCESS_DEBUG
static const unsigned long ota_check_interval = 1000 * 60; // 1 minute
#else
static const unsigned long ota_check_interval = 1000 * 3600 * 24; // 24 hours
#endif
static unsigned long last_ota_check = 0;
#endif

void setupOTA()
{
#ifdef LOCAL_OTA
	ArduinoOTA.setHostname(jsonConfig["name"]);
	ArduinoOTA.setPassword(jsonConfig["ota_pwd"]);
	ArduinoOTA.onStart([]()
					   {
						   String type;
						   if (ArduinoOTA.getCommand() == U_FLASH)
							   type = "sketch";
						   else
							   type = "filesystem";
						   DPRINTF("Start updating %s\n", type.c_str());
					   });
	ArduinoOTA.onEnd([]()
					 { DPRINTF("End\n"); });
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
						  { DPRINTF("Progress: %u%%\r", (progress / (total / 100))); });
	ArduinoOTA.onError([](ota_error_t error)
					   {
						   DPRINTF("Error[%u]: ", error);
						   if (error == OTA_AUTH_ERROR)
							   DPRINTF("Auth Failed\n");
						   else if (error == OTA_BEGIN_ERROR)
							   DPRINTF("Begin Failed\n");
						   else if (error == OTA_CONNECT_ERROR)
							   DPRINTF("Connect Failed\n");
						   else if (error == OTA_RECEIVE_ERROR)
							   DPRINTF("Receive Failed\n");
						   else if (error == OTA_END_ERROR)
							   DPRINTF("End Failed\n");
					   });
	ArduinoOTA.begin();
#endif
#ifdef REMOTE_OTA
	ota_wifi_client.setTrustAnchors(&github_root_cert_store);
	ota_http_client.begin(ota_wifi_client, tag_url);
#endif
#ifdef MANUAL_SIGNING
	signPubKey = new BearSSL::PublicKey(pubkey);
	hash = new BearSSL::HashSHA256();
	sign = new BearSSL::SigningVerifier(signPubKey);
#endif
}

void handleOTA()
{

#ifdef MANUAL_SIGNING
	Update.installSignature(hash, sign);
#endif

#ifdef LOCAL_OTA
	ArduinoOTA.handle();
#endif

#ifdef REMOTE_OTA
	if (millis() > (last_ota_check + ota_check_interval))
	{
		DPRINTF("Checking for updates\n");
		bool updateAvailable = false;
		if (ota_http_client.GET() > 0)
		{
			String payload = ota_http_client.getString();
			DPRINTF("Response: %s\n", payload.c_str());
			DynamicJsonDocument githubJson(payload.length());
			deserializeJson(githubJson, payload);
			String latest_version_s = githubJson["tag_name"];
			int dot_pos = latest_version_s.lastIndexOf('.');
			int latest_major = latest_version_s.substring(0, dot_pos).toInt();
			int latest_minor = latest_version_s.substring(dot_pos).toInt();
			updateAvailable = (latest_major >= current_major) && (latest_minor > current_minor);
		}
		if (updateAvailable)
		{
			DPRINTF("Update available\n");
			ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
			t_httpUpdate_return ret = ESPhttpUpdate.update(ota_wifi_client, update_url);
			switch (ret)
			{
			case HTTP_UPDATE_FAILED:
				DPRINTF("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
				break;
			case HTTP_UPDATE_OK:
				DPRINTF("HTTP_UPDATE_OK\n");
				break;
			}
		}
		last_ota_check = millis();
	}
#endif
}

#endif // LAB_ACCESS_OTA_H
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

#define DEBUG_FLAG "OTA"
#include "la_debug.h"
#include <Arduino.h>
#include "config.h"
#include "constants.h"

#include <ArduinoOTA.h>

// static const int current_major = 0;
// static const int current_minor = 1;

#ifdef MANUAL_SIGNING
#include "StackThunk.h"
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
BearSSL::PublicKey signPubKey(pubkey);
BearSSL::HashSHA256 hash;
BearSSL::SigningVerifier sign(&signPubKey);
#endif

#ifdef LOCAL_OTA
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#endif

#ifdef REMOTE_OTA
#include "ESP8266httpUpdate.h"
#define DEBUG_ESP_HTTP_CLIENT
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
static const unsigned long ota_check_interval = 1000 * 3;
const uint8_t ota_check_start_hour = 0, ota_check_stop_hour = 24;
#else
static const unsigned long ota_check_interval = 1000 * 60 * 30; // 30 minutes
const uint8_t ota_check_start_hour = 2, ota_check_stop_hour = 3;
#endif
static unsigned long last_ota_check = 0;
#endif

void setupOTA()
{
#ifdef LOCAL_OTA
	ArduinoOTA.setHostname(dev_name.c_str());
	ArduinoOTA.setPassword(ota_key.c_str());
	ArduinoOTA.onStart([]()
					   {
					   		redirect.stopAll();
						   String type;
						   if (ArduinoOTA.getCommand() == U_FLASH)
							   type = "sketch";
						   else
							   type = "filesystem";
						   LA_DPRINTF("(local) Start updating %s\n", type.c_str()); });
	ArduinoOTA.onEnd([]()
					 { LA_DPRINTF("(local) End\n"); });
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
						  { LA_DPRINTF("(local) Progress: %u%%\n", (progress / (total / 100))); });
	ArduinoOTA.onError([](ota_error_t error)
					   {
						   LA_DPRINTF("(local) Error[%u]: ", error);
						   if (error == OTA_AUTH_ERROR)
							   LA_DPRINTF("Auth Failed\n");
						   else if (error == OTA_BEGIN_ERROR)
							   LA_DPRINTF("Begin Failed\n");
						   else if (error == OTA_CONNECT_ERROR)
							   LA_DPRINTF("Connect Failed\n");
						   else if (error == OTA_RECEIVE_ERROR)
							   LA_DPRINTF("Receive Failed\n");
						   else if (error == OTA_END_ERROR)
							   LA_DPRINTF("End Failed\n"); });
	ArduinoOTA.begin();
	LA_DPRINTF("Local OTA started\n");
#endif
#ifdef REMOTE_OTA
	// ota_wifi_client.setTrustAnchors(&github_root_cert_store);
	ota_wifi_client.setInsecure();
	ota_wifi_client.stop();
	// ota_wifi_client
	// ota_http_client.begin(ota_wifi_client, tag_url);
	ESPhttpUpdate.followRedirects(true);
	ESPhttpUpdate.onStart([]()
						  { LA_DPRINTF("(remote) Start updating\n"); });
	ESPhttpUpdate.onProgress([](unsigned int progress, unsigned int total)
							 { 
		LA_DPRINTF("(remote) Progress: %u%%\n", (progress / (total / 100)));
		static uint32_t t = 0; 
		if(millis() > t + http_update_led_blink){
			digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
			t = millis();
		} });
	ESPhttpUpdate.onEnd([]()
						{ LA_DPRINTF("(remote) End\n"); });
	ArduinoOTA.onError([](ota_error_t error)
					   {
		LA_DPRINTF("(remote) Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR)
			LA_DPRINTF("Auth Failed\n");
		else if (error == OTA_BEGIN_ERROR)
			LA_DPRINTF("Begin Failed\n");
		else if (error == OTA_CONNECT_ERROR)
			LA_DPRINTF("Connect Failed\n");
		else if (error == OTA_RECEIVE_ERROR)
			LA_DPRINTF("Receive Failed\n");
		else if (error == OTA_END_ERROR)
			LA_DPRINTF("End Failed\n"); });
	LA_DPRINTF("Remote OTA started\n");
#endif
#ifdef MANUAL_SIGNING
	// signPubKey = new BearSSL::PublicKey(pubkey);
	// hash = new BearSSL::HashSHA256();
	// sign = new BearSSL::SigningVerifier(signPubKey);
	Update.installSignature(&hash, &sign);
	stack_thunk_add_ref();
#endif
}

union version_t
{
	struct
	{
		uint8_t ptc, rev, min, maj; // little endian
	};
	uint32_t u32;
};

// same as string to ip ???
uint32_t str_to_version(const char *str)
{
	char buf[16];
	strncpy(buf, str, sizeof(buf));
	union version_t ver = {.u32 = 0};
	char *tok;
	const char *sep = ".";
	if ((tok = strtok(buf, sep)) != NULL)
		ver.maj = atoi(tok);
	if ((tok = strtok(NULL, sep)) != NULL)
		ver.min = atoi(tok);
	if ((tok = strtok(NULL, sep)) != NULL)
		ver.rev = atoi(tok);
	if ((tok = strtok(NULL, sep)) != NULL)
		ver.ptc = atoi(tok);
	return ver.u32;
}

void handleOTA()
{

#ifdef MANUAL_SIGNING

#endif

#ifdef LOCAL_OTA
	ArduinoOTA.handle();
#endif

#ifdef REMOTE_OTA
	static bool updateAvailable = false;
	time_t raw_time = time(NULL);
	struct tm cur_time;
	cur_time = *localtime(&raw_time);
	uint8_t hour = cur_time.tm_hour;
	if (((millis() > (last_ota_check + ota_check_interval)) && (hour >= ota_check_start_hour) && (hour <= ota_check_stop_hour)))
	{
		redirect.stopAll();
		if (!updateAvailable)
		{
			ota_http_client.begin(ota_wifi_client, tag_url);

			int code = ota_http_client.GET();
			LA_DPRINTF("Checking for updates: return code %d\n", code);
			if (code > 0)
			{
				StaticJsonDocument<50> filter;
				filter["tag_name"] = true;
				StaticJsonDocument<100> resp;
				deserializeJson(resp, ota_http_client.getStream(), DeserializationOption::Filter(filter));
				LA_DJSON(resp);
				union version_t l = {.u32 = str_to_version(resp["tag_name"])};
				union version_t c = {.u32 = str_to_version(LAB_ACCESS_VERSION)};
				LA_DPRINTF("current %u.%u.%u.%u latest %u.%u.%u.%u\n", c.maj, c.min, c.rev, c.ptc, l.maj, l.min, l.rev, l.ptc);
				updateAvailable = (l.u32 > c.u32);
			}
		}
		if (updateAvailable)
		{
			LA_DPRINTF("Update available\n");
			ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
			t_httpUpdate_return ret = ESPhttpUpdate.update(ota_wifi_client, update_url);

			switch (ret)
			{
			case HTTP_UPDATE_NO_UPDATES:
				LA_DPRINTF("HTTP_UPDATE_NO_UPDATES\n");
				break;
			case HTTP_UPDATE_FAILED:
				LA_DPRINTF("HTTP_UPDATE_FAILED Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
				break;
			case HTTP_UPDATE_OK:
				LA_DPRINTF("HTTP_UPDATE_OK\n");
				break;
			}
		}
		ota_http_client.end();

		ota_wifi_client.stop();
		last_ota_check = millis();
	}
#endif
}

#endif // LAB_ACCESS_OTA_H

// const unsigned int maxBufferSize = 300;
// char answerBuffer[maxBufferSize + 1];

// boolean buildAnswerBuffer(WiFiClientSecure &client, const char * marker, unsigned long duration)
// {
//   int localBufferSize = strlen(marker); // we won't need an \0 at the end
//   char localBuffer[localBufferSize];
//   int index = 0, pos = 0;
//   boolean markerFound = false;
//   unsigned long currentTime;

//   memset(localBuffer, '\0', localBufferSize); // clear buffer

//   currentTime = millis();
//   while ((millis() - currentTime <= duration) && client.connected()) {
//     if (client.available() > 0) {
//       if (index == localBufferSize) index = 0;
//       localBuffer[index] = (uint8_t) client.read();

//       answerBuffer[pos++] = localBuffer[index];
//       answerBuffer[pos] = '\0';
//       if (pos >= maxBufferSize) pos = maxBufferSize - 1; // will loose the end
//       markerFound = true;
//       for (int i = 0; i < localBufferSize; i++) {
//         if (localBuffer[(index + 1 + i) % localBufferSize] != marker[i]) {
//           markerFound = false;
//           break;
//         }
//       }
//       index++;
//     }
//     if (markerFound) break;
//   }
//   return markerFound;
// }

// boolean fetchPart(const char * magicMarker, const char * endMarker )
// {
//   // Use WiFiClientSecure class to create TLS connection
//   WiFiClientSecure client;

//   boolean success = false;

//   Serial.print("connecting to ");
//   Serial.println(host);

//   if (!client.connect(host, httpsPort)) {
//     Serial.println("connection failed");
//     return false;
//   }

//   if (client.verify(sha1, host)) {
//     Serial.println("Sha1 fingerprint OK");
//   } else {
//     Serial.println("certificate error");
//     return false;
//   }

//   client.print("GET ");
//   client.print(urlCommand);
//   client.print(" HTTP/1.1\r\nHost: ");
//   client.print(host);
//   client.print("\r\nUser - Agent: ESP8266\r\nConnection: close\r\n\r\n");

//   if (buildAnswerBuffer(client, magicMarker, 10000ul)) {
//     // we found the start marker
//     if (buildAnswerBuffer(client, endMarker, 10000ul)) {
//       Serial.print("\n\nFound ");
//       Serial.println(magicMarker);
//       Serial.println(answerBuffer);
//       success = true;
//     }
//   }
//   client.stop();
//   return success;
// }

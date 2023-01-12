/*
 * (c) 2023 Luca Anastasio
 * anastasio.lu@gmail.com
 * www.fablaquila.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef KEY_PROVISIONING_H
#define KEY_PROVISIONING_H

#define TEST_CURVE25519_FIELD_OPS 1
#include <Crypto.h>
#include <Curve25519.h>
#include <HKDF.h>
#include <Hash.h>
#include <SHA256.h>
#include <AES.h>
#include <BlockCipher.h>
#include <CTR.h>

#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>
#include <LittleFS.h>
// #include <base64.hpp>

#include "hex_utils.h"
#define DEBUG_FLAG "KEY"
#include "la_debug.h"
#include "constants.h"
#include "config.h"
#include "GScript.h"

#define KEY_LEN 32
// 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF

/*
Provisioning steps:
	1. Generate random client key pair (client_priv, client_pub), save public to file
	2. Send client public key and get server public key (server_pub), save to file
	3. Derive private key (deriv_priv = server_pub * client_priv), then discard client_priv
	4. Derive encryption key from the previous (enc_key = hkdf(deriv_priv)), then discard deriv_priv
	////? or generate public key instead -> then send public derived key to server, will be used to send encrypted local keys???
	5. Encrypt/wrap local keys with the previous, save to file, then discard enc_key
	////? to bind to multiple server keys, repeat and save each server_pub and client_pub

*/

bool ExchangeKeys(uint8_t *server, uint8_t *client)
{
	char key[2 * KEY_LEN + 1], cmd[2 * KEY_LEN + 32];
	hex_to_char(client, KEY_LEN, key);
	sprintf(cmd, "exchangeKeys&id=%08x&key=%s", ESP.getChipId(), key);
	int ret = gscript.exec(cmd);
	LA_DPRINTF("exchange keys return code: %d\n", ret);
	if (ret <= 0)
	{
		return false;
	}
	StaticJsonDocument<128> resp;
	DeserializationError desErr = gscript.readJson(resp);
	if (desErr)
	{
		LA_DPRINTF("exchange keys deserialization error: %s\n", desErr.f_str());
		return false;
	}
	LA_DJSON(resp);
	const char *resp_key = resp["key"];
	if (strlen(resp_key) != (2 * KEY_LEN))
	{
		LA_DPRINTF("invalid length\n");
		return false;
	}
	char_to_hex(resp_key, server);
	return true;
}

bool SaveKeysToFile(const uint8_t *server, const uint8_t *client, const uint8_t *key)
{
	DynamicJsonDocument jsonConfig(1024);
	if (!readConfigFile(jsonConfig))
	{
		return false;
	}
	if (!jsonConfig["nkey"])
	{
		jsonConfig.createNestedObject("nkey");
		LA_DPRINTF("Created key object\n");
	}
	JsonObject keyObj = jsonConfig["nkey"];
	keyObj["srv"] = JsonString((const char *)server, KEY_LEN);
	keyObj["pub"] = JsonString((const char *)client, KEY_LEN);
	keyObj["enc"] = JsonString((const char *)key, KEY_LEN);
	return writeConfigFile(jsonConfig);
}

bool KeyProvisioning(uint8_t *gen_key)
{
	LA_DPRINTF("Starting Key Provisioning\n");
	uint8_t server_pub[KEY_LEN], derived[KEY_LEN], client_pub[KEY_LEN], client_priv[KEY_LEN];
	Curve25519::dh1(client_pub, client_priv);
	LA_DPRINTHEX("generated client private key", client_priv);
	LA_DPRINTHEX("client public key", client_pub);
	if (!ExchangeKeys(server_pub, client_pub))
	{
		LA_DPRINTF("Error exchanging keys\n");
		return false;
	}
	LA_DPRINTHEX("received server public key", server_pub);
	memcpy(derived, server_pub, KEY_LEN);
	if (!Curve25519::dh2(derived, client_priv))
	{
		LA_DPRINTF("Error during dh2: weak key\n");
		return false;
	}
	LA_DPRINTHEX("calculated derived key", derived);
	hkdf<SHA256>(client_priv, KEY_LEN, derived, KEY_LEN, 0, 0, 0, 0);
	LA_DPRINTHEX("encryption/wrapping key", client_priv);
	clean(derived, KEY_LEN);
	CTR<AES256> ctr;
	ctr.setKey(client_priv, KEY_LEN);
	ctr.setCounterSize(16);
	static uint8_t iv[16];
	clean(iv, 16); // anyway
	ctr.setIV(iv, 16);
	ctr.encrypt(derived, gen_key, KEY_LEN);
	ctr.clear();
	LA_DPRINTHEX("encrypted general key", derived);
	// clean(gen_key, KEY_LEN);
	if (!SaveKeysToFile(server_pub, client_pub, derived))
	{
		LA_DPRINTF("Error saving keys to file\n");
		return false;
	}
	LA_DPRINTF("Provisioning complete\n");
	return true;
}

/*
Recovery steps:
	1. Generate a random ephemeral key pair (eph_priv, eph_pub)
	2. Calculate first exchange key from group addition (xkey = client_pub + eph_pub)
	4. Calculate third exchange key from group multiplication (zkey = server_pub * eph_priv), then discard the ephemeral pair
	3. Send first exchange key to server and get second/intermediate exchange key (server side: ykey = xkey * server_priv)
	5. Recover derived key from the last two by subtraction ((recovered)deriv_priv = ykey - zkey), then discard the exchange keys
	6. Recover encryption key (enc_key = hkdf(deriv_priv)), then discard the recovered derived key //? or generate public key instead
	7. Decrypt/unwrap local keys, then discard the wrapping key

*/

bool ReadKeysFromFile(uint8_t *server, uint8_t *client, uint8_t *key)
{
	DynamicJsonDocument jsonConfig(1024);
	if (!readConfigFile(jsonConfig))
	{
		return false;
	}
	if (!jsonConfig["nkey"])
	{
		LA_DPRINTF("No keys in config file\n");
		return false;
	}
	JsonObject keyObj = jsonConfig["nkey"];
	JsonString tmp = jsonConfig["srv"];
	memcpy(server, tmp.c_str(), KEY_LEN);
	tmp = jsonConfig["pub"];
	memcpy(client, tmp.c_str(), KEY_LEN);
	tmp = jsonConfig["enc"];
	memcpy(key, tmp.c_str(), KEY_LEN);
	return true;
}

bool StartKeyRecovery()
{
}

bool CompleteKeyRecovery()
{
}

bool KeyRecovery(uint8_t *gen_key)
{
	LA_DPRINTF("starting key recovery\n");
	uint8_t client[KEY_LEN], server[KEY_LEN], encrypted[KEY_LEN];
	if (!ReadKeysFromFile(server, client, encrypted))
	{
		LA_DPRINTF("Cannot read keys from file\n");
		return false;
	}
	LA_DPRINTHEX("server public key", server);
	LA_DPRINTHEX("client public key",client);
	LA_DPRINTHEX("encrypted general key",encrypted);
	uint8_t eph_pub[KEY_LEN], eph_priv[KEY_LEN];
	Curve25519::dh1(eph_pub, eph_priv);
	LA_DPRINTHEX("ephemeral public key",eph_pub);
	LA_DPRINTHEX("ephemeral private key",eph_priv);
	uint8_t x_key[KEY_LEN], y_key[KEY_LEN];
	Curve25519::add(x_key, client, eph_pub);
	clean(eph_pub, KEY_LEN);
	LA_DPRINTHEX("first exchange key (x = client - eph_pub)",x_key);
	Curve25519::dh2(server, eph_priv);
	clean(eph_priv, KEY_LEN);
	LA_DPRINTHEX("last exchange key (z = server * eph_priv)",server); // now contains z_key
	if (!ExchangeKeys(y_key, x_key))
	{
		LA_DPRINTF("Error exchanging keys\n");
		return false;
	}
	LA_DPRINTHEX("intermediate exchange key (y = server_priv * x)",y_key);
	Curve25519::sub(server, y_key, server);
	clean(y_key, KEY_LEN);
	LA_DPRINTHEX("recovered derived key (y - z)", server); // now contains the recovered key
	hkdf<SHA256>(client, KEY_LEN, server, KEY_LEN, 0, 0, 0, 0);
	LA_DPRINTHEX("encryption/wrapping key", client);
	clean(server, KEY_LEN);
	CTR<AES256> ctr;
	ctr.setKey(client, KEY_LEN);
	ctr.setCounterSize(16);
	static uint8_t iv[16];
	clean(iv, 16); // anyway
	ctr.setIV(iv, 16);
	ctr.decrypt(gen_key, encrypted, KEY_LEN);
	ctr.clear();
	LA_DPRINTHEX("decrypted general key", derived);
	LA_DPRINTF("Key recovery complete\n");
	return true;
}

#undef DEBUG_FLAG
#endif

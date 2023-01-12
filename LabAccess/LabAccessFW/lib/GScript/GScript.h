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

#ifndef GSCRIPT_H
#define GSCRIPT_H

#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <StreamUtils.h>

#define DEBUG_FLAG "GSCRIPT"
#include "la_debug.h"


class GScript : HTTPClient
{
private:
	static const char *_gscript_host = "script.google.com";
	static const char *_uri_prefix = "/macros/s/";
	static const char *_uri_suffix = "/exec?cmd=";
	static const char *_google_root_ca =
		"-----BEGIN CERTIFICATE-----\
		MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\
		A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\
		b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\
		MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\
		YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\
		aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\
		jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\
		xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\
		1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\
		snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\
		U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\
		9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\
		BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\
		AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\
		yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\
		38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\
		AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\
		DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\
		HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\
		-----END CERTIFICATE-----";
	static const X509List _root_cert_store(_google_root_ca);
	char[64] _id;
	WiFiClientSecure _client;

public:

	GScript() : HTTPClient() {}

	GScript(const char *id) : HTTPClient()
	{
		this->setID(id);
		this->_followRedirects = HTTPC_FORCE_FOLLOW_REDIRECTS;
	}

	~GScript() {}

	bool begin()
	{
		this->_client.setTrustAnchors(_root_cert_store);
		return this->begin(this->_client, String(_gscript_host)+_uri_prefix+this->_id, 443, "", true);
	}

	void setID(const char *id)
	{
		strncpy(this->_id, id, sizeof(this->_id));
	}

	int exec(const char *cmds)
	{
		if (!this->setURL(String(_uri_suffix) + cmds))
		{
			LA_DPRINTF("Error setting url\n");
			return -1;
		}
		if(!this->connected()){
		LA_DPRINTF("Connecting\n");
		if(!this->connect()){
			LA_DPRINTF("Connection failed\n");
			return -1;
		}
		LA_DPRINTF("Connected successfully\n");
		}
		return this->GET();
	}

	DeserializationError readJson(JsonDocument &json){
		ReadBufferingStream buf(this->getStream(), 64);
		return deserializeJson(json, buf);
	}

};

GScript gscript;

#endif
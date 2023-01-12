#include <Arduino.h>
#include <unity.h>

#define TEST_CURVE25519_FIELD_OPS 1
#include <Crypto.h>
#include <Curve25519.h>
#include <HKDF.h>
#include <Hash.h>
#include <SHA256.h>
#include <AES.h>
#include <BlockCipher.h>
#include <CTR.h>
#include <utility/LimbUtil.h>

#include "hex_utils.h"

#define LA_DPRINTHEX(str, hex)              \
	{                                       \
		char tmp[2 * sizeof(hex) + 1];      \
		hex_to_char(hex, sizeof(hex), tmp); \
		Serial.printf_P(str ": %s\n", tmp); \
	}

#define LA_DPRINTF(fmt, ...) \
	Serial.printf_P(fmt, ##__VA_ARGS__)

void setUp(void)
{
	// set stuff up here
}

void tearDown(void)
{
	// clean stuff up here
}

inline uint32_t TrueRandomGet32()
{
	return RANDOM_REG32;
}

void TrueRandomGet256(uint8_t *ptr)
{
	for (uint8_t i = 0; i < 32; i += 4)
	{
		uint32_t rnd = TrueRandomGet32();
		memcpy(ptr + i, &rnd, 4);
	}
}

#define KEY_LEN 32

#define KEY_T uint8_t __attribute__((__aligned__(4)))

inline void unpack(limb_t *limb, const uint8_t *data)
{
	BigNumberUtil::unpackLE(limb, NUM_LIMBS_256BIT, data, KEY_LEN);
}

inline void unpack0(limb_t *limb, const uint8_t *data)
{
	BigNumberUtil::unpackLE(limb, NUM_LIMBS_256BIT, data, KEY_LEN);
	limb[NUM_LIMBS_256BIT - 1] &= ((((limb_t)1) << (LIMB_BITS - 1)) - 1);
}

inline void pack(uint8_t *data, const limb_t *limb)
{
	BigNumberUtil::packLE(data, 32, limb, NUM_LIMBS_256BIT);
}

void test_keys(void)
{
	uint32_t t_start = millis();
	LA_DPRINTF("Starting Key Provisioning\n");
	KEY_T server_pub[KEY_LEN], server_priv[KEY_LEN], derived[KEY_LEN], client_pub[KEY_LEN], client_priv[KEY_LEN], gen_key[KEY_LEN], test_key[KEY_LEN];

	TrueRandomGet256(gen_key);
	LA_DPRINTHEX("cleartext general key", gen_key);
	memcpy(test_key, gen_key, KEY_LEN);

	Curve25519::dh1(client_pub, client_priv);
	LA_DPRINTHEX("generated client private key", client_priv);
	LA_DPRINTHEX("client public key", client_pub);

	// server side
	Curve25519::dh1(server_pub, server_priv);
	LA_DPRINTHEX("server private key", server_priv);
	//

	LA_DPRINTHEX("server public key", server_pub);
	memcpy(derived, server_pub, KEY_LEN);
	if (!Curve25519::dh2(derived, client_priv))
	{
		LA_DPRINTF("Error during dh2: weak key\n");
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
	clean(gen_key, KEY_LEN);

	LA_DPRINTF("Provisioning complete\n");

	LA_DPRINTF("starting key recovery\n");

	KEY_T eph_pub[KEY_LEN], eph_priv[KEY_LEN], server[KEY_LEN], client[KEY_LEN];
	memcpy(server, server_pub, KEY_LEN);
	memcpy(client, client_pub, KEY_LEN);
	Curve25519::dh1(eph_pub, eph_priv);
	LA_DPRINTHEX("ephemeral public key", eph_pub);
	LA_DPRINTHEX("ephemeral private key", eph_priv);
	KEY_T x_key[KEY_LEN];


	limb_t  x_key_l[NUM_LIMBS_256BIT], client_pub_l[NUM_LIMBS_256BIT], eph_pub_l[NUM_LIMBS_256BIT];
	unpack(client_pub_l, client_pub);
	unpack(eph_pub_l, eph_pub);

	Curve25519::add(x_key_l, client_pub_l, eph_pub_l);
	clean(eph_pub, KEY_LEN);
	pack(x_key, x_key_l);
	LA_DPRINTHEX("first exchange key (x = client_pub + eph_pub)", x_key);
	Curve25519::dh2(server, eph_priv);
	clean(eph_priv, KEY_LEN);
	LA_DPRINTHEX("last exchange key (z = server_pub * eph_priv)", server); // now contains z_key
	// server side
	Curve25519::dh2(x_key, server_priv);
	//
	unpack(x_key_l, x_key);
	LA_DPRINTHEX("intermediate exchange key (y = server_priv * x)", x_key);
	limb_t rec_key_l[NUM_LIMBS_256BIT];
	unpack(rec_key_l, server);

	Curve25519::sub(rec_key_l, x_key_l, rec_key_l);
	clean(x_key, KEY_LEN);
	pack(server, rec_key_l);
	LA_DPRINTHEX("recovered derived key (y - z)", server); // now contains the recovered key
	hkdf<SHA256>(client, KEY_LEN, server, KEY_LEN, 0, 0, 0, 0);
	LA_DPRINTHEX("encryption/wrapping key", client);
	clean(server, KEY_LEN);
	CTR<AES256> ctr2;
	ctr2.setKey(client, KEY_LEN);
	ctr2.setCounterSize(16);

	clean(iv, 16); // anyway
	ctr2.setIV(iv, 16);
	ctr2.decrypt(gen_key, derived, KEY_LEN);
	ctr2.clear();
	LA_DPRINTHEX("decrypted general key", gen_key);
	LA_DPRINTF("Key recovery complete\n");
	LA_DPRINTF("time: %lums\n", millis() - t_start);

	TEST_ASSERT_EQUAL_UINT8_ARRAY(test_key, gen_key, KEY_LEN);
}

void setup()
{
	// NOTE!!! Wait for >2 secs
	// if board doesn't support software reset via Serial.DTR/RTS
	delay(3000);
	// Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);

	UNITY_BEGIN(); // IMPORTANT LINE!

	TEST_MESSAGE("testing key provisioning and recovery");
	RUN_TEST(test_keys);
	UNITY_END(); // stop unit testing
}

void loop()
{
}
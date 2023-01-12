# LabAccessControl

## FabLab access and machine control system proof-of-concept
* We use this system to manage access to our laboratory and machines usage (3D printers, laser cutter, CNC mill, etc.).
* Maker friendly and customizable, based on ESP8266 and programmed through the Arduino IDE.
* The ESP retrieves an authorized RFID cards list from a Google sheet and stores that list on flash memory, only when that list changes the internal cards list is updated with minimum flash memory wearing.
* It also logs events (what happened, when, and eventually who) on an internal FIFO and sends these logs to another Google sheet when internet connection is available.
* Due to the features above, this system can work also when internet connection goes down.

# WARNING:
Don't use MIFARE Classic cards if you don't want unauthorized access, they are clonable. Use at least Ultralight C for secure encryption.

# Development
## TODO (urgent)
Common
- [ ] diversify card keys
- [ ] switch to msgpack instead of json

Firmware
- [ ] replace DES library
- [ ] implement DES key generation
- [ ] switch to incremental mode for db updates
- [ ] implement signed messages
- [ ] implement provisioning and recovery of keys (Tang-like)


Script
- [ ] unify sheets
- [ ] incremental updates
- [ ] provisioning and recovery

App
- [ ] signed messages


## Wishlist
- [ ] switch to MQTT
- [ ] full server implmentation
- [ ] code refactoring
- [ ] external crypto chip
- [ ] smartphone nfc auth

Notes:

External crypto chip: should be used to generate card keys through hkdf/hmac, holding the source key securely and allowing it to be used only on successful authentication by the host MCU. The MCU should authenticate by recovering its private key using the procedure described above (Tang-like) or should use encrypted communications with that key.

Smartphone nfc auth: without changing the nfc reader chip (RC522), the only usable mode is host card emulation, which doesn't have a fixed UID, so the authentication protocol must be implemented on top of NDEF messages. For example: first the reader determines it's a smartphone, eventually by reading the ndef message from it, then begins the authentication process in which the reader, writing an NDEF message to the smartphone's emulated card, asks to hash some random data, and once hashed/signed using the phone's private key, that data should be read through another NDEF message and verified using the public key on the reader.

HTTP requests from Android: try retrofit


Test key: `KhGJGmHMHDS0FubbcqiVnAqxoRv1SlqD0Mg/EtXrdb0=`


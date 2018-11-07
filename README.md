# LabAccessControl

## FabLab access and machine control system proof-of-concept
* We use this system to manage access to our laboratory and machines usage (3D printers, laser cutter, CNC mill, etc.).
* Maker friendly and customizable, based on ESP8266 and programmed through the Arduino IDE.
* The ESP retrieves an authorized RFID cards list from a Google sheet and stores that list on flash memory, only when that list changes the internal cards list is updated with minimum flash memory wearing.
* It also logs events (what appened, when, and eventually who) on an internal FIFO and sends these logs to another Google sheet when internet connection is available.
* Due to the features above, this system can work also when internet connection goes down.

# WARNING:
Don't use MIFARE Classic cards if you don't want unauthorized access, they are clonable. Use at least Ultralight C for secure encryption.

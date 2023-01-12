# This is a sample Python script.
import os

# Press Maiusc+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

from tinyec import registry
import secrets
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives import hashes, keywrap, padding
from cryptography.hazmat.primitives.asymmetric.ec import *
from cryptography.hazmat.primitives.asymmetric.dh import *
#from cryptography.hazmat.primitives


def compress(pubKey):
	return hex(pubKey.x) + hex(pubKey.y % 2)[2:]


def print_hi(name):
	# Use a breakpoint in the code line below to debug your script.
	print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.


# Press the green button in the gutter to run the script.
if __name__ == '__main__':
	curve = registry.get_curve('brainpoolP256r1')
	serverPrivKey = secrets.randbelow(curve.field.n)
	serverPubKey = curve.g * serverPrivKey
	
	# provisioning
	
	data = b"**Hello World!**"
	clientPrivKey = secrets.randbelow(curve.field.n)
	clientPubKey = curve.g * clientPrivKey
	clientDerivedKey = serverPubKey * clientPrivKey
	print(compress(clientDerivedKey))
	
	derKeyBytes = clientDerivedKey.p.to_bytes(32, 'little')
	
	salt = os.urandom(16)
	hkdf = HKDF(algorithm=hashes.SHA256(), length=32, salt=salt, info=b"hkdf-info")
	key = hkdf.derive(derKeyBytes)
	iv = os.urandom(16)
	cipher = Cipher(algorithms.AES256(key), modes.CBC(iv))
	enc = cipher.encryptor()
	encData = enc.update(data) + enc.finalize()

	del data, enc, cipher, key, hkdf, clientPrivKey, clientDerivedKey, derKeyBytes
	# end provisioning
	
	# recovery
	ephPrivKey = secrets.randbelow(curve.field.n)
	ephPubKey = curve.g * ephPrivKey
	xKey = clientPubKey + ephPubKey
	
	yKey = xKey * serverPrivKey  # server side
	
	zKey = serverPubKey * ephPrivKey
	clientRecoveredKey = yKey - zKey
	print(compress(clientRecoveredKey))
	
	hkdf = HKDF(algorithm=hashes.SHA256(), length=32, salt=salt, info=b"hkdf-info")
	key = hkdf.derive(clientRecoveredKey.p.to_bytes(32, 'little'))
	cipher = Cipher(algorithms.AES256(key), modes.CBC(iv))
	dec = cipher.decryptor()
	decData = dec.update(encData) + dec.finalize()

	print(decData)
	
	
	
	

# See PyCharm help at https://www.jetbrains.com/help/pycharm/

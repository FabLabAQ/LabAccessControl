/*
 * (c) 2019 Luca Anastasio
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

package org.fablaquila.cardmaker;

import android.nfc.tech.MifareUltralight;
import android.security.keystore.KeyProperties;
import android.util.Log;

import java.io.IOException;
import java.security.InvalidAlgorithmParameterException;
import java.security.InvalidKeyException;
import java.security.Key;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.KeySpec;
import java.util.Arrays;
import java.util.Random;

import org.apache.commons.lang3.ArrayUtils;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.Mac;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.DESedeKeySpec;
import javax.crypto.spec.IvParameterSpec;
import javax.crypto.spec.SecretKeySpec;

public class MifareUltralightAuth {

    public enum AuthRetCode {AUTHENTICATED, IO_EXCEPTION, AUTH1_BAD_RESPONSE, AUTH2_BAD_RESPONSE, AUTH3_BAD_RESPONSE};

    public static AuthRetCode authenticate(MifareUltralight card, final byte[] key) {
        Log.d("AUTH", "Key: " + HexUtils.getHex(key));
        // initialize vector with 0
        for (int i = 0; i < iv.length; i++) {
            iv[i] = 0;
        }
        // mifare ultralight authenticate command
        final byte[] auth_cmd = new byte[] { 0x1A, 0x00 };
        Log.d("AUTH", "Cmd1: " + HexUtils.getHex(auth_cmd));
        // send command and receive encoded random number
        byte[] encRndB;
        try {
            encRndB = card.transceive(auth_cmd);
        } catch (IOException e) {
            return AuthRetCode.IO_EXCEPTION;
        }
        Log.d("AUTH", "Rcv1: " + HexUtils.getHex(encRndB));
        // check is the response contains the authentication command: 0xAF | ek(rndB)
        if((encRndB.length != 9) || (encRndB[0] != (byte)0xAF)) {
            return AuthRetCode.AUTH1_BAD_RESPONSE;
        }
        // delete the authentication command from the array
        encRndB = Arrays.copyOfRange(encRndB, 1, 9);
        // decrypt the random number and rotate ek(rndB) -> rndB'
        byte[] rndB = desDecrypt(key, encRndB);
        // now IV = rndB
        Log.d("AUTH", "rndB: " + HexUtils.getHex(rndB));
        byte[] rndBrot = rotateLeft(rndB);
        // generate a new random number rndA
        byte[] rndA = new byte[8];
        generateRandom(rndA);
        Log.d("AUTH", "rndA: " + HexUtils.getHex(rndA));
        // and build the second authentication command: 0xAF | ek(rndA | rndB')
        byte[] rndAB = ArrayUtils.addAll(rndA, rndBrot);
        Log.d("AUTH", "rndAB: " + HexUtils.getHex(rndAB));
        byte[] encRndAB = desEncrypt(key, rndAB);
        // now IV = last 8 bytes of encRndAB
        byte[] auth2_cmd = ArrayUtils.addAll(new byte[] {(byte)0xAF}, encRndAB);
        Log.d("AUTH", "Cmd2: " + HexUtils.getHex(auth2_cmd));
        // send and receive ek(rndA')
        byte[] encRndArot;
        try {
            encRndArot = card.transceive(auth2_cmd);
        } catch (IOException e) {
            return AuthRetCode.IO_EXCEPTION;
        }
        Log.d("AUTH", "Rcv2: " + HexUtils.getHex(encRndArot));
        // check response
        if((encRndArot.length!=9)||(encRndArot[0]!=0x00)) {
            return AuthRetCode.AUTH2_BAD_RESPONSE;
        }
        // discard the authentication finished response byte
        encRndArot=Arrays.copyOfRange(encRndArot, 1, 9);
        // decrypt rndA'
        byte[] rndArot = desDecrypt(key, encRndArot);
        Log.d("AUTH", "rndArot: " + HexUtils.getHex(rndArot));
        // check if the random number matches
        if(!Arrays.equals(rotateLeft(rndA), rndArot)) {
            return AuthRetCode.AUTH3_BAD_RESPONSE;
        }
        return AuthRetCode.AUTHENTICATED;
    }

    private static final SecureRandom rnd = new SecureRandom();
    private static void generateRandom(byte[] rndA) {
        rnd.nextBytes(rndA);
    }

    private static byte[] desEncrypt(byte[] key, byte[] data) throws RuntimeException {

        return performDes(Cipher.ENCRYPT_MODE, key, data);
    }
    private static byte[] desDecrypt(byte[] key, byte[] data) throws RuntimeException {
        return performDes(Cipher.DECRYPT_MODE, key, data);
    }

    private static byte[] iv = new byte[8];

    private static byte[] performDes(int opMode, byte[] key, byte[] data) throws RuntimeException {
        try {


//            KeyStore keyStore = KeyStore.getInstance(SettingsTab.ANDROID_KEYSTORE);
//            keyStore.load(null);
//            SecretKey desKey =  (SecretKey) keyStore.getKey(SettingsTab.desKeyAlias, null);

            Cipher des = Cipher.getInstance("DESede/CBC/NoPadding");
            SecretKeyFactory desKeyFactory = SecretKeyFactory.getInstance("DESede");
            Key desKey = desKeyFactory.generateSecret(new DESedeKeySpec(ArrayUtils.addAll(key, Arrays.copyOf(key, 8))));
            des.init(opMode, desKey, new IvParameterSpec(iv));
            byte[] ret = des.doFinal(data);

            if(opMode==Cipher.ENCRYPT_MODE) {
                iv=Arrays.copyOfRange(ret, ret.length-8, ret.length);
            } else {
                iv=Arrays.copyOfRange(data, data.length-8, data.length);
            }
            return ret;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    private static byte[] rotateLeft(byte[] in) {
        return ArrayUtils.add(Arrays.copyOfRange(in, 1, 8), in[0]);
    }

    public static boolean WriteMifareUltralightKey(MifareUltralight card, final byte[] Key) {
        //Log.d("----AUTH----", "WriteKey");
        try {
            for (int i = 0; i < 4; i++) {
                byte[] page = ArrayUtils.subarray(Key, i * 4, (i + 1) * 4);
                //Log.d("----AUTH----", "WritePage: " + HexUtils.getHex(new byte[] {(byte)(0x2C+i)}) + " | " + HexUtils.getHex(page));
                card.writePage(0x2C + i, page);
            }
            return true;
        }
        catch (IOException e) {
            //Log.d("----AUTH----", "WriteFailed");
            return false;
        }
    }

    public static boolean WriteMifareUltralightKeyAndRestrict(MifareUltralight card, final byte[] Key) {
        try {
            for (int i = 0; i < 4; i++) {
                byte[] page = ArrayUtils.subarray(Key, i * 4, (i + 1) * 4);
                card.writePage(0x2C + i, page);
            }
            // restrict write access
            //card.writePage(0x2A, new byte[] {0x03, 0x00, 0x00, 0x00});
            //card.writePage(0x2B, new byte[] {0x00, 0x00, 0x00, 0x00});
            return true;
        }
        catch (IOException e) {
            return false;
        }
    }

    public static boolean WriteMifareUltralightKeyAndRestore(MifareUltralight card, final byte[] Key) {
        try {
            for (int i = 0; i < 4; i++) {
                byte[] page = ArrayUtils.subarray(Key, i * 4, (i + 1) * 4);
                card.writePage(0x2C + i, page);
            }
            card.writePage(0x2A, new byte[] {0x30, 0x00, 0x00, 0x00});
            return true;
        }
        catch (IOException e) {
            return false;
        }
    }

    public static byte[] generateKey(byte[] id, SecretKey genKey) {
        try {
            Mac mac = Mac.getInstance(KeyProperties.KEY_ALGORITHM_HMAC_SHA256);
            mac.init(genKey);
            return mac.doFinal(id);
        }
        catch (NoSuchAlgorithmException | InvalidKeyException e) {
            return null;
        }
    }


}

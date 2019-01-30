package org.fablaquila.cardmaker;

import java.util.Arrays;

public class MifareUltralightKey {

    protected byte[] key;
    protected byte[] original;

    MifareUltralightKey(final byte[] bytes) {
        original = Arrays.copyOfRange(bytes, 0, 16);
    }

    public byte[] getBytes() {
        key = new byte[16];
        for (int i = 0; i < 8; i++) {
            key[i] = original[7-i];
        }
        for (int i = 0; i < 8; i++) {
            key[8+i] = original[15-i];
        }
        return key;
    }

    public byte[] getOriginal() {
        return original.clone();
    }



}

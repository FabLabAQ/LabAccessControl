package org.fablaquila.cardmaker;

public final class HexUtils {

    public static String getHex(byte[] bytes){
        return getHex(bytes, "");
    }

    public static String getHex(byte[] bytes, String delimiter) {
        StringBuilder sb = new StringBuilder();
        for (int i = 0; i < bytes.length; ++i) {
            int b = bytes[i] & 0xff;
            if (b < 0x10)
                sb.append('0');
            sb.append(Integer.toHexString(b).toUpperCase());
            if ((i < bytes.length -1)) {
                sb.append(delimiter);
            }
        }
        return sb.toString();
    }
}

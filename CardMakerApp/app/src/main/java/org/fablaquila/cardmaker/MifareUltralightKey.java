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

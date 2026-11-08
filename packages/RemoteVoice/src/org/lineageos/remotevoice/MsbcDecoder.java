/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

package org.lineageos.remotevoice;

/**
 * Decoder for mSBC, the wideband speech profile of SBC: 16 kHz, mono, 8 subbands, 15 blocks,
 * loudness allocation and a bitpool of 26. Each 57 byte frame decodes to 120 samples.
 *
 * Follows the SBC decoding process of the A2DP specification (frame header, scale factors, bit
 * allocation, reconstruction and synthesis filter bank).
 */
public final class MsbcDecoder {
    public static final int SAMPLE_RATE = 16000;
    public static final int FRAME_SIZE = 57;
    public static final int SAMPLES_PER_FRAME = 120;

    private static final int SYNCWORD = 0xAD;
    private static final int SUBBANDS = 8;
    private static final int BLOCKS = 15;
    private static final int BITPOOL = 26;
    /** Loudness offsets for 8 subbands at 16 kHz. */
    private static final int[] OFFSET8_16K = {-2, 0, 0, 0, 0, 0, 0, 1};

    /** Prototype filter window for 8 subbands (Q31), as in the SBC encoder of AOSP. */
    private static final int[] PROTO_8_80 = {
        0x00000000, 0x00052173, 0x000B3F71, 0x00122C7D, 0x001AFF89, 0x00255A62, 0x003060F4,
        0x003A72E7, 0x0041EC6A, 0x0044EF48, 0x00415B75, 0x0034F8B6, 0x001D8FD2, 0xFFFA2413,
        0xFFC9F10E, 0xFF8D6793, 0x00B97348, 0x01071B96, 0x0156B3CA, 0x01A1B38B, 0x01E0224C,
        0x0209291F, 0x02138653, 0x01F5F424, 0x01A7ECEF, 0x01223EBA, 0x005FD0FF, 0xFF5EEB73,
        0xFE20435D, 0xFCA86E7E, 0xFAFF95FC, 0xF9312891, 0x08B4307A, 0x0A9F3E9A, 0x0C7D59B6,
        0x0E3BB16F, 0x0FC721F9, 0x110ECEF0, 0x120435FA, 0x129C226F, 0x12CF6C75, 0x129C226F,
        0x120435FA, 0x110ECEF0, 0x0FC721F9, 0x0E3BB16F, 0x0C7D59B6, 0x0A9F3E9A, 0xF74BCF86,
        0xF9312891, 0xFAFF95FC, 0xFCA86E7E, 0xFE20435D, 0xFF5EEB73, 0x005FD0FF, 0x01223EBA,
        0x01A7ECEF, 0x01F5F424, 0x02138653, 0x0209291F, 0x01E0224C, 0x01A1B38B, 0x0156B3CA,
        0x01071B96, 0xFF468CB8, 0xFF8D6793, 0xFFC9F10E, 0xFFFA2413, 0x001D8FD2, 0x0034F8B6,
        0x00415B75, 0x0044EF48, 0x0041EC6A, 0x003A72E7, 0x003060F4, 0x00255A62, 0x001AFF89,
        0x00122C7D, 0x000B3F71, 0x00052173,
    };

    /** Gain of the synthesis window relative to the analysis prototype. */
    private static final double WINDOW_SCALE = -SUBBANDS;

    private static final double[] WINDOW = new double[80];
    private static final double[][] MATRIX = new double[16][SUBBANDS];

    static {
        for (int i = 0; i < 80; i++) {
            WINDOW[i] = WINDOW_SCALE * PROTO_8_80[i] / 2147483648.0;
        }
        for (int k = 0; k < 16; k++) {
            for (int i = 0; i < SUBBANDS; i++) {
                MATRIX[k][i] = Math.cos((i + 0.5) * (k + 4) * Math.PI / SUBBANDS);
            }
        }
    }

    private final double[] mV = new double[160];
    private final int[] mScaleFactors = new int[SUBBANDS];
    private final int[] mBits = new int[SUBBANDS];
    private final double[] mSubbandSamples = new double[SUBBANDS];
    private final double[] mU = new double[80];

    private byte[] mData;
    private int mBitPos;

    /** Resets the filter bank, e.g. after a gap in the stream. */
    public void reset() {
        java.util.Arrays.fill(mV, 0);
    }

    /**
     * Decodes the frame at {@code offset} into {@code out[outOffset..outOffset+120)}.
     *
     * @return false if the data is not an mSBC frame
     */
    public boolean decode(byte[] data, int offset, short[] out, int outOffset) {
        if (data.length - offset < FRAME_SIZE || (data[offset] & 0xFF) != SYNCWORD) {
            return false;
        }
        mData = data;
        // Sync word, two reserved bytes and the CRC
        mBitPos = (offset + 4) * 8;
        for (int sb = 0; sb < SUBBANDS; sb++) {
            mScaleFactors[sb] = readBits(4);
        }
        allocateBits();

        for (int blk = 0; blk < BLOCKS; blk++) {
            for (int sb = 0; sb < SUBBANDS; sb++) {
                int bits = mBits[sb];
                if (bits == 0) {
                    mSubbandSamples[sb] = 0;
                    continue;
                }
                int levels = (1 << bits) - 1;
                double scale = 1 << (mScaleFactors[sb] + 1);
                mSubbandSamples[sb] = scale * ((readBits(bits) * 2.0 + 1.0) / levels - 1.0);
            }
            synthesize(out, outOffset + blk * SUBBANDS);
        }
        return true;
    }

    private void allocateBits() {
        int[] bitneed = new int[SUBBANDS];
        int maxBitneed = 0;
        for (int sb = 0; sb < SUBBANDS; sb++) {
            if (mScaleFactors[sb] == 0) {
                bitneed[sb] = -5;
            } else {
                int loudness = mScaleFactors[sb] - OFFSET8_16K[sb];
                bitneed[sb] = loudness > 0 ? loudness / 2 : loudness;
            }
            maxBitneed = Math.max(maxBitneed, bitneed[sb]);
        }

        int bitcount = 0;
        int slicecount = 0;
        int bitslice = maxBitneed + 1;
        do {
            bitslice--;
            bitcount += slicecount;
            slicecount = 0;
            for (int sb = 0; sb < SUBBANDS; sb++) {
                if (bitneed[sb] > bitslice + 1 && bitneed[sb] < bitslice + 16) {
                    slicecount++;
                } else if (bitneed[sb] == bitslice + 1) {
                    slicecount += 2;
                }
            }
        } while (bitcount + slicecount < BITPOOL);
        if (bitcount + slicecount == BITPOOL) {
            bitcount += slicecount;
            bitslice--;
        }

        for (int sb = 0; sb < SUBBANDS; sb++) {
            mBits[sb] = bitneed[sb] < bitslice + 2 ? 0 : Math.min(bitneed[sb] - bitslice, 16);
        }
        for (int sb = 0; bitcount < BITPOOL && sb < SUBBANDS; sb++) {
            if (mBits[sb] >= 2 && mBits[sb] < 16) {
                mBits[sb]++;
                bitcount++;
            } else if (bitneed[sb] == bitslice + 1 && BITPOOL > bitcount + 1) {
                mBits[sb] = 2;
                bitcount += 2;
            }
        }
        for (int sb = 0; bitcount < BITPOOL && sb < SUBBANDS; sb++) {
            if (mBits[sb] < 16) {
                mBits[sb]++;
                bitcount++;
            }
        }
    }

    private void synthesize(short[] out, int outOffset) {
        System.arraycopy(mV, 0, mV, 16, 160 - 16);
        for (int k = 0; k < 16; k++) {
            double sum = 0;
            for (int i = 0; i < SUBBANDS; i++) {
                sum += MATRIX[k][i] * mSubbandSamples[i];
            }
            mV[k] = sum;
        }
        for (int i = 0; i < 5; i++) {
            for (int j = 0; j < SUBBANDS; j++) {
                mU[i * 16 + j] = mV[i * 32 + j];
                mU[i * 16 + 8 + j] = mV[i * 32 + 24 + j];
            }
        }
        for (int j = 0; j < SUBBANDS; j++) {
            double x = 0;
            for (int i = 0; i < 10; i++) {
                x += mU[j + 8 * i] * WINDOW[j + 8 * i];
            }
            long s = Math.round(x);
            out[outOffset + j] = (short) Math.max(Short.MIN_VALUE, Math.min(Short.MAX_VALUE, s));
        }
    }

    private int readBits(int count) {
        int value = 0;
        for (int i = 0; i < count; i++) {
            int b = (mData[mBitPos >> 3] >> (7 - (mBitPos & 7))) & 1;
            value = (value << 1) | b;
            mBitPos++;
        }
        return value;
    }
}

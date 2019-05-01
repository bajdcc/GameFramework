//
// Project: clibparser
// Created by bajdcc
//

#include "/include/xtoa_lut"

// Refer: https://github.com/miloyip/itoa-benchmark
// Method: branchlut

// Branching for different cases (forward)
// Use lookup table of two digits

void u32toa(unsigned int value, char* buffer) {
    if (value < 10000U) {
        unsigned int d1 = (value / 100U) << 1U;
        unsigned int d2 = (value % 100U) << 1U;

        if (value >= 1000U)
            *buffer++ = g_xtoa_lut[d1];
        if (value >= 100U)
            *buffer++ = g_xtoa_lut[d1 + 1U];
        if (value >= 10U)
            *buffer++ = g_xtoa_lut[d2];
        *buffer++ = g_xtoa_lut[d2 + 1U];
    } else if (value < 100000000U) {
        // value = bbbbcccc
        unsigned int b = value / 10000U;
        unsigned int c = value % 10000U;

        unsigned int d1 = (b / 100U) << 1U;
        unsigned int d2 = (b % 100U) << 1U;

        unsigned int d3 = (c / 100U) << 1U;
        unsigned int d4 = (c % 100U) << 1U;

        if (value >= 10000000U)
            *buffer++ = g_xtoa_lut[d1];
        if (value >= 1000000U)
            *buffer++ = g_xtoa_lut[d1 + 1U];
        if (value >= 100000U)
            *buffer++ = g_xtoa_lut[d2];
        *buffer++ = g_xtoa_lut[d2 + 1U];

        *buffer++ = g_xtoa_lut[d3];
        *buffer++ = g_xtoa_lut[d3 + 1U];
        *buffer++ = g_xtoa_lut[d4];
        *buffer++ = g_xtoa_lut[d4 + 1U];
    } else {
        // value = aabbbbcccc in decimal

        unsigned int a = value / 100000000U; // 1 to 42
        value %= 100000000U;

        if (a >= 10U) {
            unsigned int i = a << 1U;
            *buffer++ = g_xtoa_lut[i];
            *buffer++ = g_xtoa_lut[i + 1U];
        } else
            *buffer++ = '0' + (char) a;

        unsigned int b = value / 10000U; // 0 to 9999
        unsigned int c = value % 10000U; // 0 to 9999

        unsigned int d1 = (b / 100U) << 1U;
        unsigned int d2 = (b % 100U) << 1U;

        unsigned int d3 = (c / 100U) << 1U;
        unsigned int d4 = (c % 100U) << 1U;

        *buffer++ = g_xtoa_lut[d1];
        *buffer++ = g_xtoa_lut[d1 + 1U];
        *buffer++ = g_xtoa_lut[d2];
        *buffer++ = g_xtoa_lut[d2 + 1U];
        *buffer++ = g_xtoa_lut[d3];
        *buffer++ = g_xtoa_lut[d3 + 1U];
        *buffer++ = g_xtoa_lut[d4];
        *buffer++ = g_xtoa_lut[d4 + 1U];
    }
    *buffer++ = '\0';
}

void i32toa(int value, char* buffer) {
    unsigned int u = (unsigned int) value;
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1U;
    }

    u32toa(u, buffer);
}

void u64toa(unsigned long value, char* buffer) {
    if (value < 100000000L) {
        unsigned int v = (unsigned int) value;
        if (v < 10000U) {
            unsigned int d1 = (v / 100U) << 1U;
            unsigned int d2 = (v % 100U) << 1U;

            if (v >= 1000U)
                *buffer++ = g_xtoa_lut[d1];
            if (v >= 100U)
                *buffer++ = g_xtoa_lut[d1 + 1U];
            if (v >= 10U)
                *buffer++ = g_xtoa_lut[d2];
            *buffer++ = g_xtoa_lut[d2 + 1U];
        } else {
            // value = bbbbcccc
            unsigned int b = v / 10000U;
            unsigned int c = v % 10000U;

            unsigned int d1 = (b / 100U) << 1U;
            unsigned int d2 = (b % 100U) << 1U;

            unsigned int d3 = (c / 100U) << 1U;
            unsigned int d4 = (c % 100U) << 1U;

            if (value >= 10000000U)
                *buffer++ = g_xtoa_lut[d1];
            if (value >= 1000000U)
                *buffer++ = g_xtoa_lut[d1 + 1U];
            if (value >= 100000U)
                *buffer++ = g_xtoa_lut[d2];
            *buffer++ = g_xtoa_lut[d2 + 1U];

            *buffer++ = g_xtoa_lut[d3];
            *buffer++ = g_xtoa_lut[d3 + 1U];
            *buffer++ = g_xtoa_lut[d4];
            *buffer++ = g_xtoa_lut[d4 + 1U];
        }
    } else if (value < 10000000000000000L) {
        unsigned int v0 = (unsigned int) (value / 100000000L);
        unsigned int v1 = (unsigned int) (value % 100000000L);

        unsigned int b0 = v0 / 10000U;
        unsigned int c0 = v0 % 10000U;

        unsigned int d1 = (b0 / 100U) << 1U;
        unsigned int d2 = (b0 % 100U) << 1U;

        unsigned int d3 = (c0 / 100U) << 1U;
        unsigned int d4 = (c0 % 100U) << 1U;

        unsigned int b1 = v1 / 10000U;
        unsigned int c1 = v1 % 10000U;

        unsigned int d5 = (b1 / 100U) << 1U;
        unsigned int d6 = (b1 % 100U) << 1U;

        unsigned int d7 = (c1 / 100U) << 1U;
        unsigned int d8 = (c1 % 100U) << 1U;

        if (value >= 1000000000000000L)
            *buffer++ = g_xtoa_lut[d1];
        if (value >= 100000000000000L)
            *buffer++ = g_xtoa_lut[d1 + 1U];
        if (value >= 10000000000000L)
            *buffer++ = g_xtoa_lut[d2];
        if (value >= 1000000000000L)
            *buffer++ = g_xtoa_lut[d2 + 1U];
        if (value >= 100000000000L)
            *buffer++ = g_xtoa_lut[d3];
        if (value >= 10000000000L)
            *buffer++ = g_xtoa_lut[d3 + 1U];
        if (value >= 1000000000L)
            *buffer++ = g_xtoa_lut[d4];
        if (value >= 100000000L)
            *buffer++ = g_xtoa_lut[d4 + 1U];

        *buffer++ = g_xtoa_lut[d5];
        *buffer++ = g_xtoa_lut[d5 + 1U];
        *buffer++ = g_xtoa_lut[d6];
        *buffer++ = g_xtoa_lut[d6 + 1U];
        *buffer++ = g_xtoa_lut[d7];
        *buffer++ = g_xtoa_lut[d7 + 1U];
        *buffer++ = g_xtoa_lut[d8];
        *buffer++ = g_xtoa_lut[d8 + 1U];
    } else {
        unsigned int a = (unsigned int) (value / 10000000000000000L); // 1 to 1844
        value %= 10000000000000000L;

        if (a < 10U)
            *buffer++ = '0' + (char) a;
        else if (a < 100U) {
            unsigned int i = a << 1U;
            *buffer++ = g_xtoa_lut[i];
            *buffer++ = g_xtoa_lut[i + 1U];
        } else if (a < 1000U) {
            *buffer++ = '0' + (char) (a / 100);

            unsigned int i = (a % 100U) << 1U;
            *buffer++ = g_xtoa_lut[i];
            *buffer++ = g_xtoa_lut[i + 1U];
        } else {
            unsigned int i = (a / 100U) << 1U;
            unsigned int j = (a % 100U) << 1U;
            *buffer++ = g_xtoa_lut[i];
            *buffer++ = g_xtoa_lut[i + 1U];
            *buffer++ = g_xtoa_lut[j];
            *buffer++ = g_xtoa_lut[j + 1U];
        }

        unsigned int v0 = (unsigned int) (value / 100000000U);
        unsigned int v1 = (unsigned int) (value % 100000000U);

        unsigned int b0 = v0 / 10000U;
        unsigned int c0 = v0 % 10000U;

        unsigned int d1 = (b0 / 100U) << 1U;
        unsigned int d2 = (b0 % 100U) << 1U;

        unsigned int d3 = (c0 / 100U) << 1U;
        unsigned int d4 = (c0 % 100U) << 1U;

        unsigned int b1 = v1 / 10000U;
        unsigned int c1 = v1 % 10000U;

        unsigned int d5 = (b1 / 100U) << 1U;
        unsigned int d6 = (b1 % 100U) << 1U;

        unsigned int d7 = (c1 / 100U) << 1U;
        unsigned int d8 = (c1 % 100U) << 1U;

        *buffer++ = g_xtoa_lut[d1];
        *buffer++ = g_xtoa_lut[d1 + 1U];
        *buffer++ = g_xtoa_lut[d2];
        *buffer++ = g_xtoa_lut[d2 + 1U];
        *buffer++ = g_xtoa_lut[d3];
        *buffer++ = g_xtoa_lut[d3 + 1U];
        *buffer++ = g_xtoa_lut[d4];
        *buffer++ = g_xtoa_lut[d4 + 1U];
        *buffer++ = g_xtoa_lut[d5];
        *buffer++ = g_xtoa_lut[d5 + 1U];
        *buffer++ = g_xtoa_lut[d6];
        *buffer++ = g_xtoa_lut[d6 + 1U];
        *buffer++ = g_xtoa_lut[d7];
        *buffer++ = g_xtoa_lut[d7 + 1U];
        *buffer++ = g_xtoa_lut[d8];
        *buffer++ = g_xtoa_lut[d8 + 1U];
    }

    *buffer = '\0';
}

void i64toa(long value, char* buffer) {
    unsigned long u = (unsigned long) value;
    if (value < 0) {
        *buffer++ = '-';
        u = ~u + 1UL;
    }

    u64toa(u, buffer);
}

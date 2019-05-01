//
// Project: clibparser
// Created by bajdcc
//xtoa_kDiySignificandSize

#include "/include/memory"
#include "/include/xtoa_lut"
#include "/include/xtoa_cached_powers"

// Refer: https://github.com/miloyip/dtoa-benchmark
// Method: milo

struct xtoa_DiyFp {
    unsigned long f;
    int e;
};
union xtoa_DiyFp_Union {
    double d;
    unsigned long u64;
};

int xtoa_kDiySignificandSize = 64;
int xtoa_kDpSignificandSize = 52;
int xtoa_kDpExponentBias = 1075;
int xtoa_kDpMinExponent = -1075;
unsigned long xtoa_kDpExponentMask = 0x7FF0000000000000;
unsigned long xtoa_kDpSignificandMask = 0x000FFFFFFFFFFFFF;
unsigned long xtoa_kDpHiddenBit = 0x0010000000000000;

xtoa_DiyFp xtoa_DiyFp_Make(double d) {
    xtoa_DiyFp_Union u;
    u.d = d;
    int biased_e = (int) ((u.u64 & xtoa_kDpExponentMask) >> xtoa_kDpSignificandSize);
    unsigned long significand = (u.u64 & xtoa_kDpSignificandMask);

    xtoa_DiyFp fp;
    if (biased_e != 0) {
        fp.f = significand + xtoa_kDpHiddenBit;
        fp.e = biased_e - xtoa_kDpExponentBias;
    } else {
        fp.f = significand;
        fp.e = xtoa_kDpMinExponent + 1;
    }
    return fp;
}

xtoa_DiyFp xtoa_DiyFp_Sub(xtoa_DiyFp a, xtoa_DiyFp b) {
    xtoa_DiyFp out;
    out.f = a.f - b.f;
    out.e = a.e;
    return out;
}

xtoa_DiyFp xtoa_DiyFp_Mul(xtoa_DiyFp a, xtoa_DiyFp b) {
    unsigned long M32 = (unsigned long) 0xFFFFFFFF;
    unsigned long _a = a.f >> 32UL;
    unsigned long _b = a.f & M32;
    unsigned long _c = b.f >> 32UL;
    unsigned long _d = b.f & M32;
    unsigned long ac = _a * _c;
    unsigned long bc = _b * _c;
    unsigned long ad = _a * _d;
    unsigned long bd = _b * _d;
    unsigned long tmp = (bd >> 32UL) + (ad & M32) + (bc & M32);
    tmp += 1UL << 31UL;  // mult_round
    xtoa_DiyFp out;
    out.f = (ac + (ad >> 32UL) + (bc >> 32UL)) + (tmp >> 32UL);
    out.e = a.e + b.e + 64;
    return out;
}

xtoa_DiyFp xtoa_DiyFp_Normalize(xtoa_DiyFp fp) {
    while (!(fp.f & xtoa_kDpHiddenBit)) {
        fp.f <<= 1;
        fp.e--;
    }
    fp.f <<= (xtoa_kDiySignificandSize - xtoa_kDpSignificandSize - 1);
    fp.e = fp.e - (xtoa_kDiySignificandSize - xtoa_kDpSignificandSize - 1);
    return fp;
}

xtoa_DiyFp xtoa_DiyFp_NormalizeBoundary(xtoa_DiyFp fp) {
    while (!(fp.f & (xtoa_kDpHiddenBit << 1))) {
        fp.f <<= 1;
        fp.e--;
    }
    fp.f <<= (xtoa_kDiySignificandSize - xtoa_kDpSignificandSize - 2);
    fp.e = fp.e - (xtoa_kDiySignificandSize - xtoa_kDpSignificandSize - 2);
    return fp;
}

void xtoa_DiyFp_NormalizedBoundaries(xtoa_DiyFp fp, xtoa_DiyFp* minus, xtoa_DiyFp* plus) {
    xtoa_DiyFp tmp;
    tmp.f = (fp.f << 1UL) + 1UL;
    tmp.e = fp.e - 1;
    xtoa_DiyFp pl = xtoa_DiyFp_NormalizeBoundary(tmp);
    xtoa_DiyFp mi;
    if (fp.f == xtoa_kDpHiddenBit) {
        mi.f = (fp.f << 2UL) - 1UL;
        mi.e = fp.e - 2;
    } else {
        mi.f = (fp.f << 1UL) - 1UL;
        mi.e = fp.e - 1;
    }
    mi.f <<= mi.e - pl.e;
    mi.e = pl.e;
    plus->f = pl.f;
    plus->e = pl.e;
    minus->f = mi.f;
    minus->e = mi.e;
}

xtoa_DiyFp xtoa_GetCachedPower(int e, int* K) {
    double dk = (-61 - e) * 0.30102999566398114 + 347; // dk must be positive, so can do ceiling in positive
    int k = (int)(dk);
    if (k != dk)
        k++;

    unsigned index = (unsigned)((k >> 3) + 1);
    *K = -(-348 + (int)(index << 3)); // decimal exponent no need lookup table

    xtoa_DiyFp fp;
    fp.f = xtoa_kCachedPowers_F[index];
    fp.e = xtoa_kCachedPowers_E[index];
    return fp;
}

void xtoa_GrisuRound(char* buffer, int len, unsigned long delta, unsigned long rest, unsigned long ten_kappa, unsigned long wp_w) {
    while (rest < wp_w && delta - rest >= ten_kappa &&
           (rest + ten_kappa < wp_w ||  // closer
            wp_w - rest > rest + ten_kappa - wp_w)) {
        buffer[len - 1]--;
        rest += ten_kappa;
    }
}

unsigned xtoa_CountDecimalDigit32(unsigned n) {
    // Simple pure C++ implementation was faster than __builtin_clz version in this situation.
    if (n < 10U) return 1U;
    if (n < 100U) return 2U;
    if (n < 1000U) return 3U;
    if (n < 10000U) return 4U;
    if (n < 100000U) return 5U;
    if (n < 1000000U) return 6U;
    if (n < 10000000U) return 7U;
    if (n < 100000000U) return 8U;
    if (n < 1000000000U) return 9U;
    return 10U;
}

unsigned xtoa_kPow10[0] = { 1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U };
void xtoa_DigitGen(xtoa_DiyFp W, xtoa_DiyFp Mp, unsigned long delta, char* buffer, int* len, int* K) {
    xtoa_DiyFp one;
    one.f = 1UL << (unsigned long)(-Mp.e);
    one.e = Mp.e;
    xtoa_DiyFp wp_w = xtoa_DiyFp_Sub(Mp, W);
    unsigned int p1 = (unsigned int)(Mp.f >> -one.e);
    unsigned long p2 = Mp.f & (one.f - 1);
    int kappa = (int)(xtoa_CountDecimalDigit32(p1));
    *len = 0;

    while (kappa > 0) {
        unsigned int d;
        switch (kappa) {
            case 10: d = p1 / 1000000000U; p1 %= 1000000000U; break;
            case  9: d = p1 /  100000000U; p1 %=  100000000U; break;
            case  8: d = p1 /   10000000U; p1 %=   10000000U; break;
            case  7: d = p1 /    1000000U; p1 %=    1000000U; break;
            case  6: d = p1 /     100000U; p1 %=     100000U; break;
            case  5: d = p1 /      10000U; p1 %=      10000U; break;
            case  4: d = p1 /       1000U; p1 %=       1000U; break;
            case  3: d = p1 /        100U; p1 %=        100U; break;
            case  2: d = p1 /         10U; p1 %=         10U; break;
            case  1: d = p1;               p1 =           0U; break;
            default: d = 0U;                                  break;
        }

        if (d || *len)
            buffer[(*len)++] = '0' + (char)(d);
        kappa--;
        unsigned long tmp = ((unsigned long)(p1) << -one.e) + p2;
        if (tmp <= delta) {
            *K += kappa;
            xtoa_GrisuRound(buffer, *len, delta, tmp, (unsigned long)(xtoa_kPow10[kappa]) << -one.e, wp_w.f);
            return;
        }
    }

    for (;;) {
        p2 *= 10;
        delta *= 10;
        char d = (char)(p2 >> -one.e);
        if (d || *len)
            buffer[(*len)++] = '0' + d;
        p2 &= one.f - 1;
        kappa--;
        if (p2 < delta) {
            *K += kappa;
            xtoa_GrisuRound(buffer, *len, delta, p2, one.f, wp_w.f * xtoa_kPow10[-kappa]);
            return;
        }
    }
}

void xtoa_Grisu2(double value, char* buffer, int* length, int* K) {
    xtoa_DiyFp v = xtoa_DiyFp_Make(value);
    xtoa_DiyFp w_m, w_p;
    xtoa_DiyFp_NormalizedBoundaries(v, &w_m, &w_p);

    xtoa_DiyFp c_mk = xtoa_GetCachedPower(w_p.e, K);
    xtoa_DiyFp tmp = xtoa_DiyFp_Normalize(v);
    xtoa_DiyFp W = xtoa_DiyFp_Mul(tmp, c_mk);
    xtoa_DiyFp Wp = xtoa_DiyFp_Mul(w_p, c_mk);
    xtoa_DiyFp Wm = xtoa_DiyFp_Mul(w_m, c_mk);
    Wm.f++;
    Wp.f--;

    xtoa_DigitGen(W, Wp, Wp.f - Wm.f, buffer, length, K);
}

void xtoa_WriteExponent(int K, char* buffer) {
    if (K < 0) {
        *buffer++ = '-';
        K = -K;
    }

    if (K >= 100) {
        *buffer++ = '0' + (char)(K / 100);
        K %= 100;
        *buffer++ = g_xtoa_lut[K * 2];
        *buffer++ = g_xtoa_lut[K * 2 + 1];
    } else if (K >= 10) {
        *buffer++ = g_xtoa_lut[K * 2];
        *buffer++ = g_xtoa_lut[K * 2 + 1];
    } else
        *buffer++ = '0' + (char)(K);

    *buffer = '\0';
}

void xtoa_Prettify(char* buffer, int length, int k) {
    int kk = length + k; // 10^(kk-1) <= v < 10^kk

    if (length <= kk && kk <= 21) {
        int i;
        // 1234e7 -> 12340000000
        for (i = length; i < kk; i++)
            buffer[i] = '0';
        buffer[kk] = '.';
        buffer[kk + 1] = '0';
        buffer[kk + 2] = '\0';
    } else if (0 < kk && kk <= 21) {
        // 1234e-2 -> 12.34
        memmove(&buffer[kk + 1], &buffer[kk], length - kk);
        buffer[kk] = '.';
        buffer[length + 1] = '\0';
    } else if (-6 < kk && kk <= 0) {
        // 1234e-6 -> 0.001234
        int offset = 2 - kk;
        memmove(&buffer[offset], &buffer[0], length);
        buffer[0] = '0';
        buffer[1] = '.';
        int i;
        for (i = 2; i < offset; i++)
            buffer[i] = '0';
        buffer[length + offset] = '\0';
    } else if (length == 1) {
        // 1e30
        buffer[1] = 'e';
        xtoa_WriteExponent(kk - 1, &buffer[2]);
    } else {
        // 1234e30 -> 1.234e33
        memmove(&buffer[2], &buffer[1], length - 1);
        buffer[1] = '.';
        buffer[length + 1] = 'e';
        xtoa_WriteExponent(kk - 1, &buffer[0 + length + 2]);
    }
}

void xtoa_dtoa(double value, char* buffer) {
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '.';
        buffer[2] = '0';
        buffer[3] = '\0';
    } else {
        if (value < 0) {
            *buffer++ = '-';
            value = -value;
        }
        int length, K;
        xtoa_Grisu2(value, buffer, &length, &K);
        xtoa_Prettify(buffer, length, K);
    }
}

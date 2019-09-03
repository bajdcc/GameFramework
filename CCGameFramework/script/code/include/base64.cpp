//
// Project: clibparser
// Created by bajdcc
//

char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_encode(char* indata, int inlen, char* outdata, int* outlen) {

    unsigned char current;
    int i, j;
    for (i = 0, j = 0; i < inlen; i += 3)
    {
        current = (indata[i] >> 2);
        current &= (unsigned char)0x3F;
        outdata[j++] = base64char[(int)current];

        current = ((unsigned char)(indata[i] << 4)) & ((unsigned char)0x30);
        if (i + 1 >= inlen)
        {
            outdata[j++] = base64char[(int)current];
            outdata[j++] = '=';
            outdata[j++] = '=';
            break;
        }
        current |= ((unsigned char)(indata[i + 1] >> 4)) & ((unsigned char)0x0F);
        outdata[j++] = base64char[(int)current];

        current = ((unsigned char)(indata[i + 1] << 2)) & ((unsigned char)0x3C);
        if (i + 2 >= inlen)
        {
            outdata[j++] = base64char[(int)current];
            outdata[j++] = '=';
            break;
        }
        current |= ((unsigned char)(indata[i + 2] >> 6)) & ((unsigned char)0x03);
        outdata[j++] = base64char[(int)current];

        current = ((unsigned char)indata[i + 2]) & ((unsigned char)0x3F);
        outdata[j++] = base64char[(int)current];
    }

    if (outlen != (int*)0) {
        *outlen = j;
    }

    return 0;
}


int base64_decode(char* indata, int inlen, char* outdata, int* outlen) {

    unsigned char k;
    unsigned char temp[4];
    int i, j;
    for (i = 0, j = 0; i < inlen; i += 4)
    {
        *((unsigned int*)& temp) = 0Xfffffff;
        for (k = 0; k < 64; k++)
        {
            if (base64char[k] == indata[i])
                temp[0] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (base64char[k] == indata[i + 1])
                temp[1] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (base64char[k] == indata[i + 2])
                temp[2] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (base64char[k] == indata[i + 3])
                temp[3] = k;
        }

        outdata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC)) |
            ((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03));
        if (indata[i + 2] == '=')
            break;

        outdata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0)) |
            ((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F));
        if (indata[i + 3] == '=')
            break;

        outdata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) |
            ((unsigned char)(temp[3] & 0x3F));
    }

    if (outlen != (int*)0) {
        *outlen = j;
    }

    return 0;
}
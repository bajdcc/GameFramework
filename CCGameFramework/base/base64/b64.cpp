#include "stdafx.h"

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const std::vector<byte>& data)
{
    unsigned char current;
    auto& bindata = data;
    auto binlength = data.size();
    std::string str;

    for (size_t i = 0; i < binlength; i += 3)
    {
        current = (bindata[i] >> 2);
        current &= (unsigned char)0x3F;
        str.push_back(base64char[(int)current]);

        current = ((unsigned char)(bindata[i] << 4)) & ((unsigned char)0x30);
        if (i + 1 >= binlength)
        {
            str.push_back(base64char[(int)current]);
            str.push_back('=');
            str.push_back('=');
            break;
        }
        current |= ((unsigned char)(bindata[i + 1] >> 4)) & ((unsigned char)0x0F);
        str.push_back(base64char[(int)current]);

        current = ((unsigned char)(bindata[i + 1] << 2)) & ((unsigned char)0x3C);
        if (i + 2 >= binlength)
        {
            str.push_back(base64char[(int)current]);
            str.push_back('=');
            break;
        }
        current |= ((unsigned char)(bindata[i + 2] >> 6)) & ((unsigned char)0x03);
        str.push_back(base64char[(int)current]);

        current = ((unsigned char)bindata[i + 2]) & ((unsigned char)0x3F);
        str.push_back(base64char[(int)current]);
    }
    
    return str;
}

std::vector<byte> base64_decode(const std::string& data)
{
    unsigned char k;
    unsigned char temp[4];
    auto len = data.length();
    std::vector<byte> bindata;
    for (size_t i = 0, j = 0; i < len; i += 4)
    {
        memset(temp, 0xFF, sizeof(temp));
        for (k = 0; k < 64; k++)
        {
            if (base64char[k] == data[i])
                temp[0] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (base64char[k] == data[i + 1])
                temp[1] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (base64char[k] == data[i + 2])
                temp[2] = k;
        }
        for (k = 0; k < 64; k++)
        {
            if (base64char[k] == data[i + 3])
                temp[3] = k;
        }

        bindata.push_back(((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC)) |
            ((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03)));
        if (data[i + 2] == '=')
            break;

        bindata.push_back(((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0)) |
            ((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F)));
        if (data[i + 3] == '=')
            break;

        bindata.push_back(((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) |
            ((unsigned char)(temp[3] & 0x3F)));
    }
    return bindata;
}

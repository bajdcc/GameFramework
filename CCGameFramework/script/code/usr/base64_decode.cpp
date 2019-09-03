#include "/include/io"
char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
int decode() {
    int c;
    input_lock();
    char indata[4]; int i = 0;
    unsigned char k;
    unsigned char temp[4];
    while ((c = input_valid()) != -1) {
        indata[i++] = input_char();
        if (i == 4) {
            i = 0;
            *((unsigned int*)& temp) = 0Xfffffff;
            for (k = 0; k < 64; k++)
            {
                if (base64char[k] == indata[0])
                    temp[0] = k;
            }
            for (k = 0; k < 64; k++)
            {
                if (base64char[k] == indata[1])
                    temp[1] = k;
            }
            for (k = 0; k < 64; k++)
            {
                if (base64char[k] == indata[2])
                    temp[2] = k;
            }
            for (k = 0; k < 64; k++)
            {
                if (base64char[k] == indata[3])
                    temp[3] = k;
            }
            put_char(((unsigned char)(((unsigned char)(temp[0] << 2)) & 0xFC)) |
                ((unsigned char)((unsigned char)(temp[1] >> 4) & 0x03)));
            if (indata[2] == '=')
                break;

            put_char(((unsigned char)(((unsigned char)(temp[1] << 4)) & 0xF0)) |
                ((unsigned char)((unsigned char)(temp[2] >> 2) & 0x0F)));
            if (indata[3] == '=')
                break;

            put_char(((unsigned char)(((unsigned char)(temp[2] << 6)) & 0xF0)) |
                ((unsigned char)(temp[3] & 0x3F)));
        }
    }
    input_unlock();
    return 0;
}
int main(int argc, char** argv) {
    decode();
    return 0;
}
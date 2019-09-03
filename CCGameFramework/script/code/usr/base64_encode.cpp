#include "/include/io"
char* base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
int encode() {
    int c;
    input_lock();
    char indata[3]; int i = 0;
    unsigned char current;
    while ((c = input_valid()) != -1) {
        indata[i++] = input_char();
        if (i == 3) {
            i = 0;
            current = (indata[0] >> 2);
            current &= (unsigned char)0x3F;
            put_char(base64char[(int)current]);

            current = ((unsigned char)(indata[0] << 4)) & ((unsigned char)0x30);
            current |= ((unsigned char)(indata[1] >> 4)) & ((unsigned char)0x0F);
            put_char(base64char[(int)current]);

            current = ((unsigned char)(indata[1] << 2)) & ((unsigned char)0x3C);
            current |= ((unsigned char)(indata[2] >> 6)) & ((unsigned char)0x03);
            put_char(base64char[(int)current]);

            current = ((unsigned char)indata[2]) & ((unsigned char)0x3F);
            put_char(base64char[(int)current]);
        }
    }
    if (i == 1) {
        current = (indata[0] >> 2);
        current &= (unsigned char)0x3F;
        put_char(base64char[(int)current]);

        current = ((unsigned char)(indata[0] << 4)) & ((unsigned char)0x30);
        put_char(base64char[(int)current]);
        put_char('=');
        put_char('=');
    }
    else if (i == 2) {
        current = (indata[0] >> 2);
        current &= (unsigned char)0x3F;
        put_char(base64char[(int)current]);

        current = ((unsigned char)(indata[0] << 4)) & ((unsigned char)0x30);
        current |= ((unsigned char)(indata[1] >> 4)) & ((unsigned char)0x0F);
        put_char(base64char[(int)current]);

        current = ((unsigned char)(indata[1] << 2)) & ((unsigned char)0x3C);
        put_char(base64char[(int)current]);
        put_char('=');
    }
    input_unlock();
    return 0;
}
int main(int argc, char **argv) {
    encode();
    return 0;
}
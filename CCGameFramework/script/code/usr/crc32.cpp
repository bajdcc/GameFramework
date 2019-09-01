#include "/include/io"
unsigned int table[0] = {
   0X00000000, 0X77073096, 0Xee0e612c, 0X990951ba, 0X076dc419, 0X706af48f, 0Xe963a535, 0X9e6495a3,
   0X0edb8832, 0X79dcb8a4, 0Xe0d5e91e, 0X97d2d988, 0X09b64c2b, 0X7eb17cbd, 0Xe7b82d07, 0X90bf1d91,
   0X1db71064, 0X6ab020f2, 0Xf3b97148, 0X84be41de, 0X1adad47d, 0X6ddde4eb, 0Xf4d4b551, 0X83d385c7,
   0X136c9856, 0X646ba8c0, 0Xfd62f97a, 0X8a65c9ec, 0X14015c4f, 0X63066cd9, 0Xfa0f3d63, 0X8d080df5,
   0X3b6e20c8, 0X4c69105e, 0Xd56041e4, 0Xa2677172, 0X3c03e4d1, 0X4b04d447, 0Xd20d85fd, 0Xa50ab56b,
   0X35b5a8fa, 0X42b2986c, 0Xdbbbc9d6, 0Xacbcf940, 0X32d86ce3, 0X45df5c75, 0Xdcd60dcf, 0Xabd13d59,
   0X26d930ac, 0X51de003a, 0Xc8d75180, 0Xbfd06116, 0X21b4f4b5, 0X56b3c423, 0Xcfba9599, 0Xb8bda50f,
   0X2802b89e, 0X5f058808, 0Xc60cd9b2, 0Xb10be924, 0X2f6f7c87, 0X58684c11, 0Xc1611dab, 0Xb6662d3d,
   0X76dc4190, 0X01db7106, 0X98d220bc, 0Xefd5102a, 0X71b18589, 0X06b6b51f, 0X9fbfe4a5, 0Xe8b8d433,
   0X7807c9a2, 0X0f00f934, 0X9609a88e, 0Xe10e9818, 0X7f6a0dbb, 0X086d3d2d, 0X91646c97, 0Xe6635c01,
   0X6b6b51f4, 0X1c6c6162, 0X856530d8, 0Xf262004e, 0X6c0695ed, 0X1b01a57b, 0X8208f4c1, 0Xf50fc457,
   0X65b0d9c6, 0X12b7e950, 0X8bbeb8ea, 0Xfcb9887c, 0X62dd1ddf, 0X15da2d49, 0X8cd37cf3, 0Xfbd44c65,
   0X4db26158, 0X3ab551ce, 0Xa3bc0074, 0Xd4bb30e2, 0X4adfa541, 0X3dd895d7, 0Xa4d1c46d, 0Xd3d6f4fb,
   0X4369e96a, 0X346ed9fc, 0Xad678846, 0Xda60b8d0, 0X44042d73, 0X33031de5, 0Xaa0a4c5f, 0Xdd0d7cc9,
   0X5005713c, 0X270241aa, 0Xbe0b1010, 0Xc90c2086, 0X5768b525, 0X206f85b3, 0Xb966d409, 0Xce61e49f,
   0X5edef90e, 0X29d9c998, 0Xb0d09822, 0Xc7d7a8b4, 0X59b33d17, 0X2eb40d81, 0Xb7bd5c3b, 0Xc0ba6cad,
   0Xedb88320, 0X9abfb3b6, 0X03b6e20c, 0X74b1d29a, 0Xead54739, 0X9dd277af, 0X04db2615, 0X73dc1683,
   0Xe3630b12, 0X94643b84, 0X0d6d6a3e, 0X7a6a5aa8, 0Xe40ecf0b, 0X9309ff9d, 0X0a00ae27, 0X7d079eb1,
   0Xf00f9344, 0X8708a3d2, 0X1e01f268, 0X6906c2fe, 0Xf762575d, 0X806567cb, 0X196c3671, 0X6e6b06e7,
   0Xfed41b76, 0X89d32be0, 0X10da7a5a, 0X67dd4acc, 0Xf9b9df6f, 0X8ebeeff9, 0X17b7be43, 0X60b08ed5,
   0Xd6d6a3e8, 0Xa1d1937e, 0X38d8c2c4, 0X4fdff252, 0Xd1bb67f1, 0Xa6bc5767, 0X3fb506dd, 0X48b2364b,
   0Xd80d2bda, 0Xaf0a1b4c, 0X36034af6, 0X41047a60, 0Xdf60efc3, 0Xa867df55, 0X316e8eef, 0X4669be79,
   0Xcb61b38c, 0Xbc66831a, 0X256fd2a0, 0X5268e236, 0Xcc0c7795, 0Xbb0b4703, 0X220216b9, 0X5505262f,
   0Xc5ba3bbe, 0Xb2bd0b28, 0X2bb45a92, 0X5cb36a04, 0Xc2d7ffa7, 0Xb5d0cf31, 0X2cd99e8b, 0X5bdeae1d,
   0X9b64c2b0, 0Xec63f226, 0X756aa39c, 0X026d930a, 0X9c0906a9, 0Xeb0e363f, 0X72076785, 0X05005713,
   0X95bf4a82, 0Xe2b87a14, 0X7bb12bae, 0X0cb61b38, 0X92d28e9b, 0Xe5d5be0d, 0X7cdcefb7, 0X0bdbdf21,
   0X86d3d2d4, 0Xf1d4e242, 0X68ddb3f8, 0X1fda836e, 0X81be16cd, 0Xf6b9265b, 0X6fb077e1, 0X18b74777,
   0X88085ae6, 0Xff0f6a70, 0X66063bca, 0X11010b5c, 0X8f659eff, 0Xf862ae69, 0X616bffd3, 0X166ccf45,
   0Xa00ae278, 0Xd70dd2ee, 0X4e048354, 0X3903b3c2, 0Xa7672661, 0Xd06016f7, 0X4969474d, 0X3e6e77db,
   0Xaed16a4a, 0Xd9d65adc, 0X40df0b66, 0X37d83bf0, 0Xa9bcae53, 0Xdebb9ec5, 0X47b2cf7f, 0X30b5ffe9,
   0Xbdbdf21c, 0Xcabac28a, 0X53b39330, 0X24b4a3a6, 0Xbad03605, 0Xcdd70693, 0X54de5729, 0X23d967bf,
   0Xb3667a2e, 0Xc4614ab8, 0X5d681b02, 0X2a6f2b94, 0Xb40bbe37, 0Xc30c8ea1, 0X5a05df1b, 0X2d02ef8d,
};
void crc32();
int main(int argc, char **argv) {
    crc32();
    return 0;
}

void crc32() {
    int c;
    input_lock();
    unsigned int crc = 0Xffffffff;
    while ((c = input_valid()) != -1) {
        crc = table[(crc ^ ((unsigned char)input_char())) & 0Xff] ^ (crc >> 8);
    }
    input_unlock();
    crc = crc ^ 0Xffffffff;
    put_hex(crc);
}